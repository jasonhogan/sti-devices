
#ifndef SMARTEKDEVICE_H
#define SMARTEKDEVICE_H

#include <STI_Device_Adapter.h>

#include "smcs_cpp/CameraSDK.h"

#include <vector>
#include <memory>

class SmartekNodeValue;



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

	void init();
	void initializedNodeValues();

	smcs::IDevice camera;

	std::vector<std::shared_ptr<SmartekNodeValue>> nodeValues;
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
