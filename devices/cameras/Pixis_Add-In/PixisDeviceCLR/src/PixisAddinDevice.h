#ifndef PIXISADDINDEVICE_H
#define PIXISADDINDEVICE_H

#include <mutex>
#include <vector>

#include <STI_Device_Adapter.h>
#include "LightFieldHandle.h"

class PixisAddinDevice : public STI_Device_Adapter
{
public:

	PixisAddinDevice(ORBManager* orb_manager, const ConfigFile& configFile);

	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

//	void stopEventPlayback();

	void aquireImage(int index);
	void stopWaiting(int index);

	LightFieldHandle lightfield;

	void resetImageIndex();

	void setSavedImageFilename(int index, const std::string& filename);
	void setSavedSPEFilename(int index, const std::string& filename);

private:

	//Partner information for external trigger
	struct TriggerDevice
	{
		TriggerDevice() 
			: name(""), ip(""), module(0), channel(0), low(0), high(1), duration_ns(1), resetHoldoff_ns(0) {}
		string name;
		string ip;
		short module;
		short channel;
		double low;			//event value
		double high;		//event value
		double duration_ns;
		double resetHoldoff_ns;	//how early the trigger goes low initially
	} trigger;

	bool waiting;
	int currentImageIndex;
	std::string baseDirectory;

	std::string generateDataDestinationDirectory();
	std::string getFilename(int index, const std::map<int, std::string>& filenames);

	std::map<int, std::string> imageFilenames;
	std::map<int, std::string> speFilenames;
	mutable std::mutex filename_mutex;

	mutable std::mutex aquire_mutex;
	std::condition_variable aquire_condition;

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

		void setupEvent();
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

		mutable std::mutex collect_mutex;
		std::condition_variable collect_condition;

	};
};


#endif
