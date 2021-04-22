
#ifndef BLACKFLYDEVICE_H
#define BLACKFLYDEVICE_H

#include <STI_Device_Adapter.h>

//#include "smcs_cpp/CameraSDK.h"
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "Image.h"
#include "BlackflyNodeValue.h"

#include <vector>
#include <memory>

//class BlackflyNodeValue;
class Image;
class ImageWriter;

struct BlackflyDeviceEventValue
{
	unsigned short channel;
	double exposureTime;
	std::string fileTag;
	std::string paneTag;
	std::string description;
	bool newGroup;	//ch2
};

struct GroupImageProperties
{
};

class BlackflyDevice : public STI_Device_Adapter
{
public:


//	enum AbsorptionImageType { Signal, Reference, Background, None };
//	enum Operation { Add, Subtract, Mean, Absorption };
	
	BlackflyDevice(ORBManager* orb_manager, const ConfigFile& configFile, const Spinnaker::CameraPtr& camera);


	void defineAttributes();
	bool updateAttribute(std::string key, std::string value);
	void refreshAttributes();

	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

private:

	template<typename T>
	MixedData makeLabeledDataPair(const std::string& label, const T& value) {
		MixedData vec;
		vec.addValue(label);
		vec.addValue(value);
		return vec;
	}

	bool parseEventValue(const std::vector<RawEvent>& rawEvents, BlackflyDeviceEventValue& value, std::string& message);

	void generateExternalTriggerEvents(double eventTime, const RawEvent& sourceEvt);

	void init();
	void initializedNodeValues();

	void unpackLine(UINT8* rawLine, std::vector<IMAGEWORD>& unpackedLine);

	std::string baseDirectory;
	std::string generateDataDestinationDirectory();
	std::string generateFileTimestamp();

	Spinnaker::CameraPtr camera;
	//smcs::IDevice camera;

	std::vector<std::shared_ptr<BlackflyNodeValue>> nodeValues;

	std::shared_ptr<BlackflyNodeValue> exposureTimeNodeValue;
	std::shared_ptr<BlackflyNodeValue> gainAutoNodeValue;
	std::shared_ptr<BlackflyNodeValue> gainConversionNodeValue;
	std::shared_ptr<BlackflyNodeValue> gainNodeValue;
	std::shared_ptr<BlackflyNodeValue> triggerSourceNode;

	//Node value factory
	template<typename T>
	std::shared_ptr<BlackflyNodeValue> makeNodeValue(const std::string& key,
		const T& defaultValue, const std::string& allowedValues = "");
	
	template<typename T>
	bool setNodeValue(const std::string& key, const T& value);

	//Node value factory
	template<typename T>
	std::shared_ptr<BlackflyNodeValue> makeStreamNodeValue(const std::string& key,
		const T& defaultValue, const std::string& allowedValues = "");

	template<typename T>
	bool setStreamNodeValue(const std::string& key, const T& value);

	template<typename T>
	std::shared_ptr<BlackflyNodeValue> makeNodeValue(const Spinnaker::GenApi::INodeMap& nodeMap, const std::string& key,
		const T& defaultValue, const std::string& allowedValues = "");

	template<typename T>
	bool setNodeValue(const Spinnaker::GenApi::INodeMap& nodeMap, const std::string& key, const T& value);

	//template<typename T>
	//static std::shared_ptr<BlackflyNodeValue> makeNodeValue(const Node_ptr& node, const std::string& key,
	//	const T& defaultValue) 
	//{
	//	return makeNodeValue(node, key, defaultValue, "");
	//}




	bool setDownsample(int ds);
	int downsample;


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

	bool isHardwareTriggered();

	//*****************************

	friend class BlackflyEvent;
	friend class BlackflyInitializeEvent;
	friend class BlackflyFinalizeEvent;

	class ImageWriterEvent : public SynchronousEventAdapter
	{
	public:

		ImageWriterEvent(double time, BlackflyDevice* cameraDevice_, const std::string& filename);

		void collectMeasurementData();
		void waitBeforeCollectData();

		void addImage(const std::shared_ptr<Image>& image);

	private:

		BlackflyDevice* cameraDevice;
		shared_ptr<ImageWriter> imageWriter;
		//ImageWriter imageWriter;
		std::vector<std::shared_ptr<Image>> images;

		std::string basefilename;
//		std::vector<std::string> imageTags;

	};

};



template<typename T>
std::shared_ptr<BlackflyNodeValue> BlackflyDevice::makeNodeValue(const std::string& key,
	const T& defaultValue, const std::string& allowedValues)
{
	return makeNodeValue(camera->GetNodeMap(), key, defaultValue, allowedValues);
}


template<typename T>
bool BlackflyDevice::setNodeValue(const std::string& key, const T& value)
{
	return setNodeValue(camera->GetNodeMap(), key, value);
}

template<typename T>
std::shared_ptr<BlackflyNodeValue> BlackflyDevice::makeStreamNodeValue(const std::string& key,
	const T& defaultValue, const std::string& allowedValues)
{
	return makeNodeValue(camera->GetTLStreamNodeMap(), key, defaultValue, allowedValues);
}


template<typename T>
bool BlackflyDevice::setStreamNodeValue(const std::string& key, const T& value)
{
	return setNodeValue(camera->GetTLStreamNodeMap(), key, value);
}


template<typename T>
std::shared_ptr<BlackflyNodeValue> BlackflyDevice::makeNodeValue(const Spinnaker::GenApi::INodeMap& nodeMap, const std::string& key,
	const T& defaultValue, const std::string& allowedValues)
{
	Spinnaker::GenApi::CNodePtr node;

	try {
		Spinnaker::GenICam::gcstring gcKey = key.c_str();
		node = nodeMap.GetNode(gcKey);
	}
	catch (Spinnaker::Exception&)
	{
	}

	return BlackflyNodeValue::makeNodeValue(node, key, defaultValue, allowedValues);
}

template<typename T>
bool BlackflyDevice::setNodeValue(const Spinnaker::GenApi::INodeMap& nodeMap, const std::string& key, const T& value)
{
	std::shared_ptr<BlackflyNodeValue> nodePtr = makeNodeValue(nodeMap, key, value);	//factory calls setValue(value) in node constructor

	return nodePtr != 0 && nodePtr->valueIs(STI::Utils::valueToString(value));
}



#endif
