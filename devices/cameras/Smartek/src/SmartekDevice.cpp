
#include "SmartekDevice.h"
#include "Image.h"
#include "ImageWriter.h"

#include <thread>

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

bool SmartekDevice::parseEventValue(const std::vector<RawEvent>& rawEvents, SmartekDeviceEventValue& value, std::string& message)
{
	if (rawEvents.size() != 1) {
		message = "Only one image event at a time is allowed.";
		return false;
	}

	if (rawEvents.at(0).getValueType() != MixedValue::Vector) {
		message = "Invalid data type.  The camera expects a vector.";
		return false;
	}

	value.channel = rawEvents.at(0).channel();

	auto tuple = rawEvents.at(0).value().getVector();
	bool success = true;

	//Check that each type in tuple is correct.

	if (tuple.size() >= 2) {
		if (tuple.at(1).getType() != MixedValue::String) {
			message = "Camera image description must be a string";
			success = false;
		}
	}
	if (tuple.size() >= 1) {
		if (tuple.at(0).getType() != MixedValue::String) {
			message = "Camera filename must be a string";
			success = false;
		}
	}
	else {
		message = "Camera commands must be a tuple in the form (string description, string filename).";  // or (int centerPixelX, int centerPixelY, int halfWidth).
		success = false;
	}

	if (!success) return false;

	switch (tuple.size())
	{
	case 2:
		value.description = tuple.at(0).getString();
		value.baseFilename = tuple.at(1).getString();
		break;
	case 1:
		value.description = tuple.at(0).getString();
		value.baseFilename = "default";
		break;
	default:
		message = "Never should get here, but Andor camera commands must be a tuple in the form (double exposureTime, string description, string filename)";
		success = false;
		break;
	}

	return success;
}

void SmartekDevice::parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut)
{
//	status = device->CommandNodeExecute("AcquisitionStart");
	//ch 0: Individual image
	//ch 1: Image group.  By default, groups all images taken in a given shot into one multi-page tif. Has argument "new_group" that starts a new group (false by default)
	//ch 2: Image group with operation. Examples: Average, Absorption (1-2)/3.  Does not save intermediate images.

	double eventTime = 0;
	double lastEventTime = 0;
	double holdoff = 10000000;		//10 ms
//	RawEventMap::const_iterator events;

	SmartekDeviceEventValue value;
	std::string err_message;

	shared_ptr<Image> image;
	shared_ptr<ImageWriter> imageWriter;


	for (auto events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (!parseEventValue(events->second, value, err_message)) {
			throw EventParsingException(events->second.at(0), err_message);
		}

		eventTime = events->first - holdoff;

		if (value.channel == 0) {
			
			//smcs::IImageInfo imageInfo = nullptr;
			//UINT32 sizeX, sizeY;
			//camera->GetImageInfo(&imageInfo);
			//imageInfo->GetSize(sizeX, sizeY);

			image = std::make_shared<Image>(1, 1);

			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime, this, image);	//schedule event to plau immediately after last event

			eventsOut.push_back(smartekEvent.release());


			imageWriter = std::make_shared<ImageWriter>();		//can be shared by multiple events, if images are to be combined into one file (multipane tif)
			//imageWriter->addImage(image);

			std::string filenametemp = "C:\\Users\\Jason\\Code\\dev\\sti-devices\\devices\\cameras\\Smartek\\project\\Release\\test.tif";

			auto writer = std::make_unique<ImageWriterEvent>(eventTime, this, imageWriter, filenametemp);
			writer->addImage(image);
			writer->addMeasurement( events->second.at(0) );		//register the measurement with the source RawEvent

			eventsOut.push_back(writer.release());
		}

		lastEventTime = eventTime;
	}
}

void SmartekDevice::definePartnerDevices()
{
//	addPartnerDevice("ext_trigger", "sr-timing1.stanford.edu", 2, "Digital Out");
}

std::string SmartekDevice::getDeviceHelp()
{
	return "";
}


SmartekDevice::SmartekEvent::SmartekEvent(double time, SmartekDevice* cameraDevice_, 
	const shared_ptr<Image>& image)
	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image)
{
	image->imageData.reserve(image->getImageSize());	//reserve memory for image data
}

void SmartekDevice::SmartekEvent::playEvent()
{
	//TEMP
	bool status;
	// set trigger selector to frame start
	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);
	status = cameraDevice->camera->SetStringNodeValue("TriggerSelector", "FrameStart");

	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 1);
	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStart");
	status = cameraDevice->camera->CommandNodeExecute("TriggerSoftware");
//	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);
}

void SmartekDevice::SmartekEvent::stop()
{
	//Abort
	bool status;
//	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 1);
	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStop");
//	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);

}

void SmartekDevice::SmartekEvent::waitBeforeCollectData()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));	//TEMP
}


void SmartekDevice::SmartekEvent::collectMeasurementData()
{

	//Pulls the image from the camera and stores it in an Image

	smcs::IImageInfo imageInfo = nullptr;
	cameraDevice->camera->GetImageInfo(&imageInfo);

//	return;

	UINT32 sizeX, sizeY;
	imageInfo->GetSize(sizeX, sizeY);

	image->setImageHeight(sizeY);
	image->setImageWidth(sizeX);

//	UINT8* data = imageInfo->GetRawData();
//	image->imageData;
	
//	test.data();
//	test.insert(test.end(), &data[0], &data[dataArraySize]);
	
//	std::vector<char> test;
//	test.clear();

	//copy image from camera buffer, line by line
	UINT8* lineData;
	for (unsigned int j = 0; j < sizeY; j++) {	// j is image line number
		//Get next line and insert it into vector
		lineData = imageInfo->GetRawData(j);
		image->imageData.insert(image->imageData.end(), &lineData[0], &lineData[imageInfo->GetLineSize()]);
	}

	cameraDevice->camera->PopImage(imageInfo);	//removes image from camera buffer

//	for (unsigned int j = 0; j < sizeY; j++)
//		::memcpy(m_svBitmap.GetRawData(j), imageInfo->GetRawData(j), imageInfo->GetLineSize());
}


void SmartekDevice::ImageWriterEvent::addImage(const std::shared_ptr<Image>& image)
{
	imageWriter->addImage(image);
}


void SmartekDevice::ImageWriterEvent::collectMeasurementData()
{
	if (eventMeasurements.size() == 1)
	{
		eventMeasurements.at(0)->setData(filename);
	}

	imageWriter->write(filename);

}

void SmartekDevice::ImageWriterEvent::waitBeforeCollectData()
{
	//Make sure all images are extracted from camera.
	//This should happen automatically if this event is after all image events in the set. (?)
}


