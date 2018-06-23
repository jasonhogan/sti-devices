
#ifndef SMARTEKDEVICE_H
#define SMARTEKDEVICE_H

#include <STI_Device_Adapter.h>

#include "smcs_cpp/CameraSDK.h"

#include <vector>
#include <memory>

class SmartekNodeValue;
class Image;
class ImageWriter;

struct SmartekDeviceEventValue
{
	unsigned short channel;
	std::string baseFilename;
	std::string description;
};

class SmartekDevice : public STI_Device_Adapter
{
public:

	SmartekDevice(ORBManager* orb_manager, std::string configFilename, const smcs::IDevice& camera);


	void defineAttributes();
	bool updateAttribute(std::string key, std::string value);
	void refreshAttributes();

	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

private:

	bool parseEventValue(const std::vector<RawEvent>& rawEvents, SmartekDeviceEventValue& value, std::string& message);

	void init();
	void initializedNodeValues();

	smcs::IDevice camera;

	std::vector<std::shared_ptr<SmartekNodeValue>> nodeValues;

	//*****************************

	class ImageWriterEvent : public SynchronousEventAdapter
	{
	public:

		ImageWriterEvent(double time, SmartekDevice* cameraDevice_, const shared_ptr<ImageWriter>& imageWriter, const std::string& filename)
			: SynchronousEventAdapter(time, cameraDevice_), imageWriter(imageWriter), filename(filename) {}

		void collectMeasurementData();
		void waitBeforeCollectData();

		void addImage(const std::shared_ptr<Image>& image);

	private:

		shared_ptr<ImageWriter> imageWriter;
		std::string filename;

	};

	class SmartekEvent : public SynchronousEventAdapter
	{
	public:

		SmartekEvent(double time, SmartekDevice* cameraDevice_, const shared_ptr<Image>& image);

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

		SmartekDevice* cameraDevice;

		shared_ptr<Image> image;

	private:

	};


};

class SmartekNodeValue	//abstract
{
public:
	SmartekNodeValue(const smcs::IDevice& device, const std::string& key, 
		const std::string& defaultValue, const std::string& allowedValues)
		: device(device), key(key), allowedValues(allowedValues), defaultValue(defaultValue) {}
	SmartekNodeValue(const smcs::IDevice& device, const std::string& key, const std::string& defaultValue)
		: SmartekNodeValue(device, key, defaultValue, "") {}

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

class SmartekStringNodeValue : public SmartekNodeValue
{
public:
	SmartekStringNodeValue(const smcs::IDevice& device, const std::string& key,
		const std::string& defaultValue, const std::string& allowedValues) 
		: SmartekNodeValue(device, key, defaultValue, allowedValues) {}

	bool setValue(const std::string& value) { return device->SetStringNodeValue(key, value); }
	bool getValue(std::string& value) { return device->GetStringNodeValue(key, value); }
};

class SmartekFloatNodeValue : public SmartekNodeValue
{
public:

	SmartekFloatNodeValue(const smcs::IDevice& device, const std::string& key,
		double defaultValue)
		: SmartekNodeValue(device, key, STI::Utils::valueToString(defaultValue)) {}

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
