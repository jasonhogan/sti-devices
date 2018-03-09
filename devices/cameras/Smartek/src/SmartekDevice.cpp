
#include "SmartekDevice.h"

SmartekDevice::SmartekDevice(ORBManager* orb_manager, std::string configFilename, const smcs::IDevice& camera)
	: STI_Device_Adapter(orb_manager, configFilename), camera(camera)
{
	init();
}

void SmartekDevice::init()
{
//	camera->SetIntegerNodeValue("TLParamsLocked", 0);	//Transport Layer and Device critical features are locked and cannot be changed
}

void SmartekDevice::initializedNodeValues()
{
	std::shared_ptr<SmartekNodeValue> value;


	value = std::make_shared<SmartekStringNodeValue>(camera, 
		"AcquisitionMode", "SingleFrame", "Continuous, SingleFrame, MultiFrame");
	nodeValues.push_back(value);

	value = std::make_shared<SmartekFloatNodeValue>(camera, "ExposureTime", 20000);
	nodeValues.push_back(value);

	value = std::make_shared<SmartekFloatNodeValue>(camera, "Gain", 0);	//Range 0 to 24 in dB
	nodeValues.push_back(value);	

	value = std::make_shared<SmartekStringNodeValue>(camera, "TriggerMode", "On", "On, Off");
	nodeValues.push_back(value);
	
	value = std::make_shared<SmartekStringNodeValue>(camera, "TriggerSource", 
		"Line1", "Line1, Software");
	nodeValues.push_back(value);
	
	value = std::make_shared<SmartekStringNodeValue>(camera, "TriggerActivation",
		"RisingEdge", "RisingEdge, FallingEdge");
	nodeValues.push_back(value);

	//Not supported; only accepts TriggerSelector = AcquisitionStart
	//value = std::make_shared<SmartekStringNodeValue>(camera, "TriggerSelector",
	//	"AcquisitionStart", "AcquisitionStart, AcquisitionEnd, AcquisitionActive,	FrameStart,	FrameEnd, FrameActive, LineStart, ExposureStart, ExposureEnd, ExposureActive");
	//nodeValues.push_back(value);


}

void SmartekDevice::defineAttributes()
{
	initializedNodeValues();

	for (auto node : nodeValues) {
		addAttribute(node->key, node->defaultValue, node->allowedValues);
	}
}

bool SmartekDevice::updateAttribute(std::string key, std::string value)
{
	bool success = false;
	
	//unlock params
	if (!camera->SetIntegerNodeValue("TLParamsLocked", 0)) {
		return false;
	}

	for (auto node : nodeValues) {
		if (key.compare(node->key) == 0) {
			success = node->setValue(value);
			break;
		}
	}

	//re-lock params
	camera->SetIntegerNodeValue("TLParamsLocked", 1);

	return success;
}

void SmartekDevice::refreshAttributes()
{
	std::string value;

	for (auto node : nodeValues) {
		if (node->getValue(value)) {
			setAttribute(node->key, value);
		}
	}
}

void SmartekDevice::defineChannels()
{
	addInputChannel(0, DataString, ValueVector);
}

void SmartekDevice::parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut)
{
//	status = device->CommandNodeExecute("AcquisitionStart");
}

void SmartekDevice::definePartnerDevices()
{
//	addPartnerDevice("ext_trigger", "sr-timing1.stanford.edu", 2, "Digital Out");
}

std::string SmartekDevice::getDeviceHelp()
{
	return "";
}

