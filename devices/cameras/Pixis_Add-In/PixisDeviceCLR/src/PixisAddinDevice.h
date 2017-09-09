#ifndef PIXISADDINDEVICE_H
#define PIXISADDINDEVICE_H

#include <mutex>

#include <STI_Device_Adapter.h>
#include "LightFieldHandle.h"

class PixisAddinDevice : public STI_Device_Adapter
{
public:


	PixisAddinDevice(ORBManager* orb_manager)
		: STI_Device_Adapter(orb_manager, "Pixis", "localhost", 0)
	{
		waiting = false;
	}

	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

	void wait();
	void stopWaiting();

//	Callback aquireHandler;

	LightFieldHandle lightfield;

private:

	bool waiting;

	mutable std::mutex aquire_mutex;
	std::condition_variable aquire_condition;

	class PixisEvent : public SynchronousEvent
	{
	public:

		PixisEvent(double time, PixisAddinDevice* cameraDevice_, int index)
			: SynchronousEvent(time, cameraDevice_), cameraDevice(cameraDevice_), imageIndex(index)
		{
		}

		//		void reset();	//override
		void stop();	//override

		void setupEvent() { }
		void loadEvent();
		void playEvent();
		void collectMeasurementData();

		void waitBeforeCollectData();

		PixisAddinDevice* cameraDevice;

	private:
		int imageIndex;

	};

};


#endif
