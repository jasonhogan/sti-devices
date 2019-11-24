
#ifndef BLACKFLYDEVICE_H
#define BLACKFLYDEVICE_H

#include <STI_Device_Adapter.h>

//#include "smcs_cpp/CameraSDK.h"

#include "Image.h"

#include <vector>
#include <memory>

class BlackflyNodeValue;
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

	enum BlackflyEventMode { Normal, Mean, Photodetector };
//	enum AbsorptionImageType { Signal, Reference, Background, None };
//	enum Operation { Add, Subtract, Mean, Absorption };
	
	BlackflyDevice(ORBManager* orb_manager, const ConfigFile& configFile, const smcs::IDevice& camera);


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

	smcs::IDevice camera;

	std::vector<std::shared_ptr<BlackflyNodeValue>> nodeValues;

	std::shared_ptr<BlackflyNodeValue> exposureTimeNodeValue;
	std::shared_ptr<BlackflyNodeValue> gainNodeValue;


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

	class BlackflyInitializeEvent : public SynchronousEventAdapter
	{
	public:
		BlackflyInitializeEvent(double time, BlackflyDevice* cameraDevice, int totalImages)
			: SynchronousEventAdapter(time, cameraDevice), cameraDevice(cameraDevice), totalImages(totalImages) {}
		
		void loadEvent();
		void playEvent();

		int totalImages;

	private:
		BlackflyDevice* cameraDevice;
	};

	class BlackflyEvent : public SynchronousEventAdapter
	{
	public:

		BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image);
		BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, BlackflyEventMode mode);
		BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, const shared_ptr<Image>& imageBuffer, BlackflyEventMode mode);

//			: SynchronousEvent(time, cameraDevice_), cameraDevice(cameraDevice_),
//			image(image), imageWriter(imageWriter)
//		{
//			image->imageData.reserve(image->getImageSize());	//reserve memory for image data
//		}

//		void reset();	//override
		void stop();	//override

		void setupEvent() { }
//		void loadEvent();
		void playEvent();
		void collectMeasurementData();

		void waitBeforeCollectData();

		BlackflyDevice* cameraDevice;

		shared_ptr<Image> image;
		shared_ptr<Image> imageBuffer;
		int imageCount;		//for keeping track of the mean

//		AbsorptionImageType absType;

		BlackflyEventMode mode;

	private:
		Int64 getTotal(const vector<IMAGEWORD>& data);
	};

};

class BlackflyNodeValue	//abstract
{
public:
	BlackflyNodeValue(const smcs::IDevice& device, const std::string& key, 
		const std::string& defaultValue, const std::string& allowedValues)
		: device(device), key(key), allowedValues(allowedValues), defaultValue(defaultValue) {}
	BlackflyNodeValue(const smcs::IDevice& device, const std::string& key, const std::string& defaultValue)
		: BlackflyNodeValue(device, key, defaultValue, "") {}

	//	virtual bool setValue(const std::string& key) = 0;
	//	virtual bool getValue(const std::string& key, std::string& value) = 0;

	bool operator=(const std::string& nodeKey) { return key.compare(nodeKey) == 0; }

	virtual bool setValue(const std::string& value) = 0;
	virtual bool getValue(std::string& value) = 0;

	std::string key;
	std::string defaultValue;	//Used as the inital value for the Attribute.
	std::string allowedValues;	//For initializing Attributes with a drop down list of options. Comma delimited.

protected:
	smcs::IDevice device;
};

class BlackflyStringNodeValue : public BlackflyNodeValue
{
public:
	BlackflyStringNodeValue(const smcs::IDevice& device, const std::string& key,
		const std::string& defaultValue, const std::string& allowedValues) 
		: BlackflyNodeValue(device, key, defaultValue, allowedValues) {}

	bool setValue(const std::string& value) { return device->SetStringNodeValue(key, value); }
	bool getValue(std::string& value) { return device->GetStringNodeValue(key, value); }
};

class BlackflyFloatNodeValue : public BlackflyNodeValue
{
public:

	BlackflyFloatNodeValue(const smcs::IDevice& device, const std::string& key,
		double defaultValue)
		: BlackflyNodeValue(device, key, STI::Utils::valueToString(defaultValue)) {}

	bool setValue(const std::string& value)
	{
		double val;
		return STI::Utils::stringToValue(value, val) && device->SetFloatNodeValue(key, val);
	}
	bool getValue(std::string& value)
	{
		double val;
		if (device->GetFloatNodeValue(key, val)) {
			value = STI::Utils::valueToString(val);
			return true;
		}
		return false;
	}
};

#endif
