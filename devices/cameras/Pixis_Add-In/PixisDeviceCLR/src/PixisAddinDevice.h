#ifndef PIXISADDINDEVICE_H
#define PIXISADDINDEVICE_H

#include <mutex>
#include <vector>

#include <STI_Device_Adapter.h>
#include "LightFieldHandle.h"

class PixisAddinDevice : public STI_Device_Adapter
{
public:


	PixisAddinDevice(ORBManager* orb_manager)
		: STI_Device_Adapter(orb_manager, "Pixis", "localhost", 0)
	{
//		waiting = false;

		currentImageIndex = 0;
		
//		resetImageIndex();
	}


	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

//	void stopEventPlayback();

	void aquireImage(int index);


//	void waitForImage(int index);
	void stopWaiting(int index);

//	Callback aquireHandler;

	LightFieldHandle lightfield;

	void resetImageIndex();

	void setSavedImageFilename(int index, const std::string& filename);
	void setSavedSPEFilename(int index, const std::string& filename);


private:

	bool waiting;
	int currentImageIndex;
//	std::vector<bool> imagesDone;

	mutable std::mutex aquire_mutex;
	std::condition_variable aquire_condition;

//	class LightFieldCallback
//	{
//		virtual void setSavedImageFilename(const std::string& filename) = 0;
//		virtual void setSavedSPEFilename(const std::string& filename) = 0;
////		virtual void stopWaiting() = 0;
//	};


	std::string getFilename(int index, const std::map<int, std::string>& filenames);
	std::map<int, std::string> imageFilenames;
	std::map<int, std::string> speFilenames;
	mutable std::mutex filename_mutex;


	friend class PixisEvent;

	class PixisEvent : public SynchronousEvent
	{
	public:

		PixisEvent(double time, PixisAddinDevice* cameraDevice_, int index)
			: SynchronousEvent(time, cameraDevice_), cameraDevice(cameraDevice_), imageIndex(index)
		{
			reset();
		}

		void reset();	//override
		void stop();	//override

		void setupEvent() { }
		void loadEvent();
		void playEvent();
		void collectMeasurementData();

		void waitBeforeCollectData();


//		void setSavedImageFilename(const std::string& filename) { imageFilename = filename; }
//		void setSavedSPEFilename(const std::string& filename) { speFilename = filename; }
//		void stopWaiting();


	private:

		PixisAddinDevice* cameraDevice;
		int imageIndex;
		bool dataReady;
//		std::string imageFilename;
//		std::string speFilename;

		mutable std::mutex collect_mutex;
		std::condition_variable collect_condition;

	};

};


#endif
