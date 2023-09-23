#ifndef PICOSCOPE5000DEVICE_H
#define PICOSCOPE5000DEVICE_H

#include "STI_Device_Adapter.h"

#include "PicoScope5000.h"

#include <string>
#include <mutex>
#include <condition_variable>


struct PicoScope5000DeviceEventValue
{
	std::vector<unsigned> channels;
	std::string filename;

	unsigned preTriggerSamples;
	unsigned postTriggerSamples;
};



class PicoScope5000Device : public STI_Device_Adapter
{
public:

	PicoScope5000Device(ORBManager* orb_manager, const ConfigFile& configFile);
	~PicoScope5000Device();

	void defineAttributes();
	bool updateAttribute(std::string key, std::string value);
	void refreshAttributes();

	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

private:

	bool parseEventValue(const std::vector<RawEvent>& rawEvents, PicoScope5000DeviceEventValue& value, std::string& message);

	PicoScope5000 picoScope;

	std::string baseDirectory;
	std::string generateDataDestinationDirectory();
	std::string generateFileTimestamp();

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
	bool externalTriggerEventsOn;		//true if this device automatically generates its own trigger events on a partner device

	static double convertTime_ns(const std::string& value, const std::string& units);
	static double convertValue_mV(const std::string& value, const std::string& units);

	template<typename T>
	MixedData makeLabeledDataPair(const std::string& label, const T& value) {
		MixedData vec;
		vec.addValue(label);
		vec.addValue(value);
		return vec;
	}


	friend class PicoScope5000DeviceEvent;

	class PicoScope5000DeviceEvent : public SynchronousEventAdapter
	{
	public:

		PicoScope5000DeviceEvent(double time, PicoScope5000Device* scopeDevice, 
			const std::string& filename, unsigned preTriggerSamples, unsigned postTriggerSamples);

		~PicoScope5000DeviceEvent();

		void setupEvent();
		void loadEvent();
		void playEvent();
		void collectMeasurementData();
		void waitBeforeCollectData();


	private:

		PicoScope5000Device* scopeDevice;

		bool runStarted;
		bool abort;

		std::string basefilename;
		unsigned preTriggerSamples;
		unsigned postTriggerSamples;

		mutable std::mutex runMutex;
		mutable std::condition_variable runCondition;

	};

};


#endif
