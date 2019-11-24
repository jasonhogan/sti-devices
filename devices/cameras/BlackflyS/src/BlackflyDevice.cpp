
#include "BlackflyDevice.h"
#include "Image.h"
#include "ImageWriter.h"

#include <thread>
#include <sstream>
#include <iomanip>

#include <filesystem>
namespace fs = std::experimental::filesystem;

BlackflyDevice::BlackflyDevice(ORBManager* orb_manager, const ConfigFile& configFile, const smcs::IDevice& camera)
	: STI_Device_Adapter(orb_manager, configFile), camera(camera)
{
	configFile.getParameter("Trigger partner name", trigger.name);
	configFile.getParameter("Trigger partner IP", trigger.ip);
	configFile.getParameter("Trigger partner module", trigger.module);
	configFile.getParameter("Trigger partner channel", trigger.channel);

	configFile.getParameter("Trigger Low Value", trigger.low);
	configFile.getParameter("Trigger High Value", trigger.high);
	configFile.getParameter("Trigger duration (ns)", trigger.duration_ns);
	configFile.getParameter("Trigger Reset Holdoff (ns)", trigger.resetHoldoff_ns);		//holdoff time before event when LOW is asserted

	configFile.getParameter("STI Data Base Directory", baseDirectory);

	init();
}

void BlackflyDevice::init()
{
	camera->SetIntegerNodeValue("TLParamsLocked", 0);
	camera->SetStringNodeValue("PixelFormat", "Mono10Packed"); //Allowed values: "Mono8" or "Mono10Packed"

	externalTriggerEventsOn = false;
	downsample = 1;

	// Abort program if save directory is not accessibly.
	if(!fs::exists(baseDirectory)) {
		std::cout << std::endl << std::endl 
			<< "Error: Image save directory not accessible.  Check fileserver login."
			<< std::endl;
		exit(1);
	}
}

void BlackflyDevice::initializedNodeValues()
{
	std::shared_ptr<BlackflyNodeValue> value;

	value = std::make_shared<BlackflyStringNodeValue>(camera, 
		"AcquisitionMode", "SingleFrame", "Continuous, SingleFrame, MultiFrame");
	nodeValues.push_back(value);

	//Not an attribute anymore, now a command argument
//	value = std::make_shared<BlackflyFloatNodeValue>(camera, "ExposureTime", 20000);
//	nodeValues.push_back(value);

	gainNodeValue = std::make_shared<BlackflyFloatNodeValue>(camera, "Gain", 0);	//Range 0 to 24 in dB
	nodeValues.push_back(gainNodeValue);

	value = std::make_shared<BlackflyStringNodeValue>(camera, "TriggerMode", "On", "On, Off");
	nodeValues.push_back(value);
	
	value = std::make_shared<BlackflyStringNodeValue>(camera, "TriggerSource", 
		"Line1", "Line1, Software");
	nodeValues.push_back(value);
	
	value = std::make_shared<BlackflyStringNodeValue>(camera, "TriggerActivation",
		"RisingEdge", "RisingEdge, FallingEdge");
	nodeValues.push_back(value);

	//Not supported; only accepts TriggerSelector = AcquisitionStart
	//value = std::make_shared<BlackflyStringNodeValue>(camera, "TriggerSelector",
	//	"AcquisitionStart", "AcquisitionStart, AcquisitionEnd, AcquisitionActive,	FrameStart,	FrameEnd, FrameActive, LineStart, ExposureStart, ExposureEnd, ExposureActive");
	//nodeValues.push_back(value);

	exposureTimeNodeValue = std::make_shared<BlackflyFloatNodeValue>(camera, "ExposureTime", 20000);
}

void BlackflyDevice::defineAttributes()
{
	initializedNodeValues();

	for (auto node : nodeValues) {
		addAttribute(node->key, node->defaultValue, node->allowedValues);
	}

	addAttribute("Generate Trigger Events", (externalTriggerEventsOn ? "True" : "False"), "True, False");
	addAttribute("Downsample", 1);
}

bool BlackflyDevice::updateAttribute(std::string key, std::string value)
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

	if (key.compare("Generate Trigger Events") == 0) {
		externalTriggerEventsOn = value.compare("True") == 0;
		success = true;
	}

	if (key.compare("Downsample") == 0) {
		int ds;
		if (STI::Utils::stringToValue(value, ds) && setDownsample(ds)) {
			success = true;
		}
	}
	return success;
}

bool BlackflyDevice::setDownsample(int ds)
{
	if (ds < 1)
		return false;

	//check for other downsample constraints ...

	downsample = ds;
	return true;
}

void BlackflyDevice::refreshAttributes()
{
	std::string value;

	for (auto node : nodeValues) {
		if (node->getValue(value)) {
			setAttribute(node->key, value);
		}
	}

	bool softTrigger = getAttribute("TriggerSource").compare("Software") == 0;
	setAttribute("Generate Trigger Events", ((externalTriggerEventsOn && !softTrigger) ? "True" : "False"));
}

void BlackflyDevice::defineChannels()
{
	addInputChannel(0, DataVector, ValueVector, "Single Image");		// (exposure time, filename tag)
	addInputChannel(1, DataVector, ValueVector, "Absorption Image");	// (exposure time, filename tag, new_group)
	addInputChannel(2, DataVector, ValueVector, "Group Image");			// (exposure time, filename tag, pane tag, new_group)
	addInputChannel(3, DataVector, ValueVector, "Mean Image");			// (exposure time, filename, new_group)
	addInputChannel(4, DataDouble, ValueNumber, "Photodetector");			// exposure time
}


void BlackflyDevice::definePartnerDevices()
{
	addPartnerDevice("External Trigger", trigger.ip, trigger.module, trigger.name);
	partnerDevice("External Trigger").enablePartnerEvents();
}

std::string BlackflyDevice::getDeviceHelp()
{
	std::stringstream help;

	help << "Syntax for the Value field of meas() command:" << std::endl;
	help << "ch 0: (<exposure time>, <file tag string>)" << std::endl;
	help << "ch 1: (<exposure time>, <file tag string>, <new group bool>)" << std::endl;
	help << "ch 2: (<exposure time>, <file tag string>, <pane tag string>, <new group bool>)" << std::endl;
	help << "ch 3: (<exposure time>, <file tag string>, <new group bool>)" << std::endl;
	help << "ch 4: <exposure time>" << std::endl;
	help << std::endl;

	help << "The file tag is appended to the automatically generated image filename prefix:" << std::endl;
	help << "   YYYY_MM_DD-hh_mm_ss-<file tag>.tif" << std::endl;
	help << "The pane tag is the label for each pane in a multipane tif file.  This string is" << std::endl;
	help << "saved as part of the measurement xml data of each shot, and as meta data in the tif." << std::endl;
	help << std::endl;

	help << "Timing file examples:" << std::endl;
	help << "meas(ch(<dev>, 0), <time>, (<exposureTime>, \"file_tag\"))" << std::endl;
	
	help << std::endl << "Absorption Image:" << std::endl;
	help << "meas(ch(<dev>, 1), <time>, (<exposureTime>, \"file_tag\", True))   #starts new group" << std::endl;
	help << "meas(ch(<dev>, 1), <time>, (<exposureTime>, \"file_tag\", False))  #appended to previous group" << std::endl;

	help << std::endl << "Group Image:" << std::endl;
	help << "meas(ch(<dev>, 2), <time>, (<exposureTime>, \"file_tag\", \"pane1_tag\", True))   #starts new group" << std::endl;
	help << "meas(ch(<dev>, 2), <time>, (<exposureTime>, \"file_tag\", \"pane2_tag\", False))  #appended to previous group" << std::endl;

	help << std::endl << "Photodetector mode:" << std::endl;
	help << "meas(ch(<dev>, 4), <time>, <exposureTime>)" << std::endl;
	help << std::endl << std::endl;

	help << "Exposure time is entered in nanoseconds.  The camera expects a time in microseconds. " << std::endl
		<< "The time recorded in the xml file is in microseconds." << std::endl;


	return help.str();
}

bool BlackflyDevice::parseEventValue(const std::vector<RawEvent>& rawEvents, BlackflyDeviceEventValue& value, std::string& message)
{
	if (rawEvents.size() != 1) {
		message = "Only one image event at a time is allowed.";
		return false;
	}

	auto& rawEvent = rawEvents.at(0);
	value.channel = rawEvent.channel();
	MixedValueVector tuple;

	//All channels require their arguements to be a vector except channel 4.
	//Treat channel 4 separately first and make a tuple of length 1.
	if (value.channel == 4) {
		if (rawEvent.getValueType() != MixedValue::Double) {
			message = "Invalid argument format. Expecting:  <exposure time>";
			return false;
		}
		tuple.push_back(rawEvent.value().getNumber());
	}
	else {	//all other channels
		tuple = rawEvent.value().getVector();
	}

	bool success = true;

	//type checking
	switch (value.channel) {
	case 0:	//single image
		if (tuple.size() != 2 
			|| tuple.at(0).getType() != MixedValue::Double
			|| tuple.at(1).getType() != MixedValue::String) {
			message = "Invalid argument format. Expecting:  (<exposure time>, <file tag string>)";
			success = false;
		}
		break;
	case 1:	//Absorption image
	case 3:	//Mean image
		if (tuple.size() != 3
			|| tuple.at(0).getType() != MixedValue::Double
			|| tuple.at(1).getType() != MixedValue::String
			|| !(tuple.at(2).getType() == MixedValue::Boolean || tuple.at(2).getType() == MixedValue::Double)) {
			message = "Invalid argument format. Expecting:  (<exposure time>, <file tag string>, <new group bool>)";
			success = false;
		}
		break;
	case 2:	//group image
		if (tuple.size() != 4
			|| tuple.at(0).getType() != MixedValue::Double
			|| tuple.at(1).getType() != MixedValue::String
			|| tuple.at(2).getType() != MixedValue::String
			|| !(tuple.at(3).getType() == MixedValue::Boolean || tuple.at(3).getType() == MixedValue::Double)) {
			message = "Invalid argument format. Expecting:  (<exposure time>, <file tag string>, <pane tag string>, <new group bool>)";
			success = false;
		}
		break;
	case 4:	//photodetector
		if (tuple.size() != 1
			|| tuple.at(0).getType() != MixedValue::Double) {
			message = "Invalid argument format. Expecting:  <exposure time>";
			success = false;
		}
		break;
	default:
		message = "Channel number " + STI::Utils::valueToString(value.channel) + " is not supported.";
		return false;
	}

	if (!success) return false;

	value.newGroup = false;

	//decode value

	//check for valid exposure time
	double exposure = tuple.at(0).getDouble() / 1000;
	std::string oldExposure;
	std::string newExposure;
	double exposureCheck;
	
	exposureTimeNodeValue->getValue(oldExposure);
	if (!exposureTimeNodeValue->setValue(STI::Utils::valueToString(exposure)) 
		|| !exposureTimeNodeValue->getValue(newExposure)
		|| !STI::Utils::stringToValue(newExposure, exposureCheck)
		|| (exposureCheck != exposure)) {
		message = "Invalid exposure time.";
		return false;
	}
	else {
		//exposure time is valid; reset to old exposure time for now (exposure time is set during playEvent)
		exposureTimeNodeValue->setValue(oldExposure);
	}

	value.exposureTime = exposure;	//Time is specifed in nanosec but the camera expect microsec
	value.paneTag = "";	//default

	switch (value.channel) {
	case 0:	//single image
		value.fileTag = tuple.at(1).getString();
		break;
	case 3:	//Mean image
	case 1:	//Absorption image
		value.fileTag = tuple.at(1).getString();
		if (tuple.at(2).getType() == MixedValue::Boolean) {
			value.newGroup = tuple.at(2).getBoolean();
		}
		else {
			value.newGroup = (tuple.at(2).getNumber() > 0);
		}
		break;
	case 2:	//group image
	{
		value.fileTag = tuple.at(1).getString();
		value.paneTag = tuple.at(2).getString();
		if (tuple.at(3).getType() == MixedValue::Boolean) {
			value.newGroup = tuple.at(3).getBoolean();
		}
		else {
			value.newGroup = (tuple.at(3).getNumber() > 0);
		}
		break;
	}
	case 4:	//photodetector
		break;
	default:
		message = "Never should get here. Invalid channel or value.";
		success = false;
		break;
	}

	return success;
}

void BlackflyDevice::generateExternalTriggerEvents(double eventTime, const RawEvent& sourceEvt)
{
	if (externalTriggerEventsOn) {
		//add partner events (trigger) at eventTime 
		partnerDevice("External Trigger").event(
			eventTime - trigger.resetHoldoff_ns, trigger.channel, trigger.low, sourceEvt);	// Low	
		partnerDevice("External Trigger").event(eventTime, trigger.channel, trigger.high, sourceEvt);	// High
		partnerDevice("External Trigger").event(
			eventTime + trigger.duration_ns, trigger.channel, trigger.low, sourceEvt);	// Low
	}
}

void BlackflyDevice::parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut)
{
//	status = device->CommandNodeExecute("AcquisitionStart");
	//ch 0: Individual image
	//ch 1: Absorption image (image group with signal, reference, and enventually a background possibly)
	//ch 2: Image group.  By default, groups all images taken in a given shot into one multi-page tif. Has argument "new_group" that starts a new group (false by default)
	//ch 3: Image group that returns the mean image of the group.  Does not save intermediate images.
	//ch 4: photodiode mode; integrate all pixels
	double minimumEventTime = 1000;	//1 us
	double eventTime = minimumEventTime;	//from timing file
	
	double thisEventTime = minimumEventTime;
	double lastEventTime = minimumEventTime;
	double thisWriterEventTime = minimumEventTime;
	double lastWriterEventTime = minimumEventTime;

	double holdoff = 0*10000000;		//10 ms
//	RawEventMap::const_iterator events;

	BlackflyDeviceEventValue value;
	std::string err_message;

	shared_ptr<Image> image;

	unsigned mean_image_count;		//number of images being averaged
	shared_ptr<Image> image_mean;		//shared by multiple events
	shared_ptr<Image> image_mean_result;	//buffer shared by multiple events

	std::unique_ptr<ImageWriterEvent> groupWriterEvent;
	std::unique_ptr<ImageWriterEvent> absorptionWriterEvent;	//ch 1, absorption images
	std::unique_ptr<ImageWriterEvent> groupWriterEvent2;

	unsigned totalImages = 0;
	unsigned absorptionImagePaneCount = 0;		//kind of a hack. Needed to label absorption image panes and check for errors.
	const RawEvent* lastAbsorptionEvent = nullptr;

	std::string filenamebase = "test";
	std::string filenameext = ".tif";
	std::string filename;

	UINT32 sizeX = 1936;
	UINT32 sizeY = 1216;


	for (auto events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (events->first < minimumEventTime) {
			throw EventParsingException(events->second.at(0), "Minimum event time is " + STI::Utils::valueToString(minimumEventTime));
		}

		if (!parseEventValue(events->second, value, err_message)) {
			throw EventParsingException(events->second.at(0), err_message);
		}

		eventTime = events->first - holdoff;	//time of the event requested in timing file

		generateExternalTriggerEvents(eventTime, events->second.at(0));		//conditional on attribute status
		
		totalImages++;		//counted so the camera knows how many frames to acquire
		
		if (isHardwareTriggered()) {
			//For hardware trigger, make the event play immediately after the last event.
			//This way the aquisition starts as soon as possible to avoid missing a trigger.
			thisEventTime = lastEventTime + 10;
			thisWriterEventTime = eventTime;	//the event should be ready to save to disk soon after the trigger
		}
		else {
			//Software triggers occur at the nominal time of the event.
			thisEventTime = eventTime;
			thisWriterEventTime = eventTime + 10;	//try to save to disk as soon as possible
		}
	
		image = std::make_shared<Image>(sizeY, sizeX);
		image->paneTag = value.paneTag;
		image->exposureTime = value.exposureTime;
		image->filename = value.fileTag;
		image->downsample = downsample;
		
		std::string gain = "";
		gainNodeValue->getValue(gain);
		image->gain = gain;

		if (value.channel == 0) {	//Individual image

			auto smartekEvent = std::make_unique<BlackflyEvent>(thisEventTime, this, image);	//schedule event to play immediately after last event
			eventsOut.push_back(smartekEvent.release());

			filename = value.fileTag + filenameext;

			auto writer = std::make_unique<ImageWriterEvent>(thisWriterEventTime, this, filename);
			writer->addImage(image);
			writer->addMeasurement( events->second.at(0) );		//register the measurement with the source RawEvent

			eventsOut.push_back(writer.release());
		}
		if (value.channel == 1) {	//absorption image group
			if (value.newGroup && absorptionWriterEvent != nullptr) {	//New group created; flush the group image buffer
				if (absorptionImagePaneCount != 2 && absorptionImagePaneCount != 3) {
					throw EventParsingException(events->second.at(0),
						"Invalid pane count in absorption image group.  absorptionImagePaneCount=" + STI::Utils::valueToString(absorptionImagePaneCount));
				}
				absorptionWriterEvent->setTime(lastWriterEventTime + 1);	//was lastEventTime + 10
				eventsOut.push_back(absorptionWriterEvent.release());
			}
			if (absorptionWriterEvent == nullptr) {	//new group created
				absorptionImagePaneCount = 0;
				filename = value.fileTag + filenameext;	//TODO: fix
				absorptionWriterEvent = std::make_unique<ImageWriterEvent>(thisWriterEventTime, this, filename);	//overwrites each time	//was lastEventTime + 10
			}

			lastAbsorptionEvent = &(events->second.at(0));	//store for error checking at end of for loop

			auto smartekEvent = std::make_unique<BlackflyEvent>(thisEventTime, this, image);	//schedule event to play immediately after last event

			eventsOut.push_back(smartekEvent.release());

			absorptionImagePaneCount++;		//expecting signal, reference, and possibly a background pane.
			absorptionWriterEvent->addImage(image);
			absorptionWriterEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent

			switch (absorptionImagePaneCount) {
			case 1:
				image->paneTag = "signal";
				break;
			case 2:
				image->paneTag = "reference";
				break;
			case 3:
				image->paneTag = "background";
				break;
			default:
				throw EventParsingException(events->second.at(0), "Invalid pane count in absorption image group.  absorptionImagePaneCount=" + STI::Utils::valueToString(absorptionImagePaneCount));
				break;
			}
		}
		if (value.channel == 2) {	//image group
			if (value.newGroup && groupWriterEvent != nullptr) {	//New group created; flush the group image buffer
				groupWriterEvent->setTime(lastWriterEventTime + 1);	//was lastEventTime + 10
				eventsOut.push_back(groupWriterEvent.release());
			}
			if(groupWriterEvent == nullptr) {	//new group created
				filename = value.fileTag + filenameext;	//TODO: fix
				groupWriterEvent = std::make_unique<ImageWriterEvent>(thisWriterEventTime, this, filename);	//overwrites each time	//was lastEventTime + 10
			}

			auto smartekEvent = std::make_unique<BlackflyEvent>(thisEventTime, this, image);	//schedule event to play immediately after last event

			eventsOut.push_back(smartekEvent.release());

			groupWriterEvent->addImage(image);
			groupWriterEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent
		}
		if (value.channel == 3) {	//image group with operation.  (Mean)

			if (value.newGroup && groupWriterEvent2 != nullptr) {	//New group created; flush the group image buffer
				groupWriterEvent2->setTime(lastWriterEventTime + 1);
				eventsOut.push_back(groupWriterEvent2.release());
			}
			if (groupWriterEvent2 == nullptr) {	//new group created
				filename = value.fileTag + filenameext;	//TODO: fix
				groupWriterEvent2 = std::make_unique<ImageWriterEvent>(thisWriterEventTime, this, filename);	//overwrites each time

				mean_image_count = 0;
				image_mean = std::make_shared<Image>(sizeY, sizeX);	
				image_mean_result = std::make_shared<Image>(sizeY, sizeX);
				groupWriterEvent2->addImage(image_mean_result);	//only one image; construct the mean by adding each new image to this as they are captured.
			}

			auto smartekEvent = std::make_unique<BlackflyEvent>(thisEventTime, this, image_mean, image_mean_result, Mean);

			mean_image_count++;
			smartekEvent->imageCount = mean_image_count;

			eventsOut.push_back(smartekEvent.release());

			groupWriterEvent2->addMeasurement(events->second.at(0));	//register the measurement with the source RawEvent
		}

		if (value.channel == 4) {	// Photodetector mode: integrate all pixels
			image = std::make_shared<Image>(sizeY, sizeX);
			auto smartekEvent = std::make_unique<BlackflyEvent>(thisEventTime, this, image, Photodetector);	//schedule event to plau immediately after last event

			smartekEvent->addMeasurement(events->second.at(0));

			eventsOut.push_back(smartekEvent.release());
		}

		lastEventTime = eventTime;
		lastWriterEventTime = thisWriterEventTime;
	}

	if (groupWriterEvent != nullptr) {	//flush the group image buffer
		groupWriterEvent->setTime(lastEventTime + 10);
		eventsOut.push_back(groupWriterEvent.release());
	}
	if (absorptionWriterEvent != nullptr) {
		if (lastAbsorptionEvent == nullptr) {
			//can't happen
		}
		if (absorptionImagePaneCount != 2 && absorptionImagePaneCount != 3) {
			throw EventParsingException(*lastAbsorptionEvent,
				"Invalid pane count in absorption image group.  absorptionImagePaneCount=" + STI::Utils::valueToString(absorptionImagePaneCount));
		}
		absorptionWriterEvent->setTime(lastEventTime + 10);
		eventsOut.push_back(absorptionWriterEvent.release());
	}
	

	//Initialization event.  Happens before any image events
	auto smartekIniEvent = std::make_unique<BlackflyInitializeEvent>(0, this, totalImages);
	eventsOut.push_back(smartekIniEvent.release());

}

//void operate(const Image& newImage, Image& result, BlackflyDevice::Operation operation)
//{
//	switch (operation)
//	{
//	case Add:
//		for (unsigned p = 0; p < newImage.imageData.size(); ++p) {
//			result.imageData.at(p) += newImage.imageData.at(p);
//		}
//		break;
//	case Subtract:
//		for (unsigned p = 0; p < newImage.imageData.size(); ++p) {
//			result.imageData.at(p) -= newImage.imageData.at(p);
//		}
//		break;
//	case Mean:
//		for (unsigned p = 0; p < newImage.imageData.size(); ++p) {
//			result.imageData.at(p) *= (1.0*operationParameters.n / (operationParameters.n + 1));
//			result.imageData.at(p) += (newImage.imageData.at(p) / (operationParameters.n + 1));
//		}
//		break;
//	default:
//		break;
//
//	}
//}

void BlackflyDevice::BlackflyInitializeEvent::loadEvent()
{
	cameraDevice->camera->ClearImageBuffer();
}


void BlackflyDevice::BlackflyInitializeEvent::playEvent()
{

	cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);	//unlock
	cameraDevice->camera->SetIntegerNodeValue("AcquisitionFrameCount", totalImages);


//	cout << "Parsed::totalImages = " << totalImages << endl;

	cameraDevice->camera->SetImageBufferFrameCount(totalImages);

	//UINT32 pendingImages;
	//INT64 frameCount;

	//cameraDevice->camera->GetIntegerNodeValue("AcquisitionFrameCount", frameCount);

	//pendingImages = cameraDevice->camera->GetPendingImagesCount();
	//cout << "Parsed::Pending: " << pendingImages << endl;
	//cout << "Parsed::frameCount: " << frameCount << endl;
}


BlackflyDevice::BlackflyEvent::BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, const shared_ptr<Image>& imageBuffer, BlackflyEventMode mode)
	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image), imageBuffer(imageBuffer), mode(mode)
{
	imageCount = 0;
}

BlackflyDevice::BlackflyEvent::BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image, BlackflyEventMode mode)

	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image), mode(mode)
{
	image->imageData.reserve(image->getImageSize());	//reserve memory for image data
	imageCount = 0;
}

BlackflyDevice::BlackflyEvent::BlackflyEvent(double time, BlackflyDevice* cameraDevice_, const shared_ptr<Image>& image)
	: BlackflyDevice::BlackflyEvent::BlackflyEvent(time, cameraDevice_, image, Normal)
{
}

void BlackflyDevice::BlackflyEvent::playEvent()
{
	//set exposure time for this image
	cameraDevice->exposureTimeNodeValue->setValue(STI::Utils::valueToString(image->exposureTime));

	bool status;
	// set trigger selector to frame start
	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);
	status = cameraDevice->camera->SetStringNodeValue("TriggerSelector", "FrameStart");

	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 1);
	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStart");

	//Trigger setup
	if (cameraDevice->isHardwareTriggered()) {
		//wait for hardware trigger
		status = cameraDevice->camera->CommandNodeExecute("Line1");		//hardware trigger
	}
	else {
		//send software trigger NOW
		status = cameraDevice->camera->CommandNodeExecute("TriggerSoftware");	//software trigger
	}
}

bool BlackflyDevice::isHardwareTriggered()
{
	//Trigger setup
	std::string trigSource;
	camera->GetStringNodeValue("TriggerSource", trigSource);

	return (trigSource.compare("Software") != 0);
}


void BlackflyDevice::BlackflyEvent::stop()
{
	//Abort
	bool status;
	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStop");
}

void BlackflyDevice::BlackflyEvent::waitBeforeCollectData()
{
	bool success = false;

	while (!success && cameraDevice->running()) {
		success = cameraDevice->camera->WaitForImage(1);	//1 second timeout
//		cout << "WaitForImage: " << getEventNumber()  << " pending: " << cameraDevice->camera->GetPendingImagesCount() << endl;
	}
}


void BlackflyDevice::BlackflyEvent::collectMeasurementData()
{
	//Pulls the image from the camera and stores it in an Image

	smcs::IImageInfo imageInfo = nullptr;
	cameraDevice->camera->GetImageInfo(&imageInfo);

	if (cameraDevice->camera->GetPendingImagesCount() < 1 || imageInfo == 0) {
		//error; image not in buffer
		image->imageData.clear();	//ImageWriter will not include this image
		return;
	}

	//imageInfo is not null

	image->clearMetaData();	//reset;  needed in this Image was used on previous shot
	image->addMetaData("PaneTag", image->paneTag);
	image->addMetaData("Downsample", STI::Utils::valueToString(image->downsample));
	image->addMetaData("ExposureTime", STI::Utils::valueToString(image->exposureTime));
	image->addMetaData("Gain", STI::Utils::valueToString(image->gain));
	image->addMetaData("CameraTimestamp", STI::Utils::valueToString(imageInfo->GetCameraTimestamp()));

	UINT32 sizeX, sizeY;
	imageInfo->GetSize(sizeX, sizeY);

	image->setImageHeight(sizeY);
	image->setImageWidth(sizeX);

	int linesize = imageInfo->GetLineSize();
	int rawdatasize = imageInfo->GetRawDataSize();

	cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);

	image->imageData.clear();	//important if this Image is reused on multiple shots

	//copy image from camera buffer, line by line
	UINT8* lineData;
	std::vector<IMAGEWORD> unpackedLine(sizeX, 0);
	for (unsigned int j = 0; j < sizeY; j++) {	// j is image line number
		//Get next line and insert it into vector
		lineData = imageInfo->GetRawData(j);
		cameraDevice->unpackLine(lineData, unpackedLine);	//convert Mono10Packed format into shorts (16 bits per pixel)
		image->imageData.insert(image->imageData.end(), unpackedLine.begin(), unpackedLine.end());
	}

	cameraDevice->camera->PopImage(imageInfo);	//removes image from camera buffer

	if (mode == Mean) {
		double res;

		if (imageBuffer->imageData.size() < image->imageData.size()) {
			imageBuffer->imageData.assign(image->imageData.size(), 0);
		}

		for (unsigned p = 0; p < image->imageData.size(); ++p) {
			res = static_cast<double>(imageBuffer->imageData.at(p))*(imageCount - 1)
				+ static_cast<double>(image->imageData.at(p));

			imageBuffer->imageData.at(p) = static_cast<IMAGEWORD>(res / imageCount);
		}
	}

	//Max sum is (2^10)*1936*1216 = 2,410,676,224.  This requires 32 bits.
	if (mode == Photodetector && eventMeasurements.size() == 1)
	{
		Int64 total = getTotal(image->imageData);
		double result = static_cast<double>(total);		//MixedValue doesn't support Int64 or unsigned long
		eventMeasurements.at(0)->setData(result);
	}
}

Int64 BlackflyDevice::BlackflyEvent::getTotal(const vector<IMAGEWORD>& data)
{
	Int64 total = 0;
	for (auto& d : data) {
		total += d;
	}
	return total;
}


void BlackflyDevice::unpackLine(UINT8* rawLine, std::vector<IMAGEWORD>& unpackedLine)
{
	unsigned g;	//pixel group (3 bytes each)
	unsigned p;	//pixel number (2 bytes each)

	short mA = 3;		//mask for pixel #1:  0b00000011	(with LSB on far right)
	short mB = 3 << 4;  //mask for pixel #2:  0b00110000	(with LSB on far right)

	for (p=0, g=0; p < unpackedLine.size(); p+=2, g+=3)
	{
		//2 pixels in every 3 bytes
		//unpackedLine is the size of the final image width;  rawLine is 1.5 times longer

		// Byte:   |    g    |   g+1   |    g+2  |
		// Bits:   |2345 6789|01xx 01xx|2345 6789|
		// Pixels: |   pixel #1   |   pixel #2   |
		
		//LSB of camera pixel is bit 0, MSB is bit 9
		//Bits within each byte are little endian
		//NOTE: Left and right shift operators are REVERSED from diagram, since
		//LSB is stored in the right most bit!
		//Below, bits 0 and 1 are extracted from g+1, masked, shifted, and preprended to bits 2-9.

		short pA = (((short)rawLine[g]) << 2) | ((((short)rawLine[g + 1]) & mA) >> 0);

		unpackedLine.at(p)   = (((short)rawLine[g]) << 2)   | ((((short)rawLine[g + 1]) & mA) >> 0);
		unpackedLine.at(p+1) = (((short)rawLine[g+2]) << 2) | ((((short)rawLine[g + 1]) & mB) >> 4);

	}
}


BlackflyDevice::ImageWriterEvent::ImageWriterEvent(double time, BlackflyDevice* cameraDevice_, const std::string& filename)
	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), basefilename(filename)
{
	imageWriter = std::make_shared<ImageWriter>();
}

void BlackflyDevice::ImageWriterEvent::addImage(const std::shared_ptr<Image>& image)
{
	imageWriter->addImage(image);
	images.push_back(image);
}


void BlackflyDevice::ImageWriterEvent::collectMeasurementData()
{
	std::string baseDir = cameraDevice->generateDataDestinationDirectory();

	//filename is specific to this shot, at the time of data collection
	std::string filename = 
		cameraDevice->generateDataDestinationDirectory() + "\\" +
		cameraDevice->generateFileTimestamp() + basefilename;

	MixedData data;


	for (unsigned i = 0; i < eventMeasurements.size() && i < images.size(); ++i) {	//there should be as many images as there are measurements
		data.clear();
		data.addValue(cameraDevice->makeLabeledDataPair("Filename", filename));
		data.addValue(cameraDevice->makeLabeledDataPair("Exposure Time", images.at(i)->exposureTime));
		data.addValue(cameraDevice->makeLabeledDataPair("Downsample", images.at(i)->downsample));
		data.addValue(cameraDevice->makeLabeledDataPair("Gain", images.at(i)->gain));

		if (eventMeasurements.size() > 1) {	//multipane images
			data.addValue(cameraDevice->makeLabeledDataPair("Pane Tag", images.at(i)->paneTag));
			data.addValue(cameraDevice->makeLabeledDataPair("Pane Number", static_cast<int>(i + 1)));
			data.addValue(cameraDevice->makeLabeledDataPair("Image Count", static_cast<int>(eventMeasurements.size())));
			data.addValue(cameraDevice->makeLabeledDataPair("Group ID", static_cast<int>(getEventNumber())));
		}
		
		eventMeasurements.at(i)->setData(data);
	}
	
	imageWriter->write(filename);
}

void BlackflyDevice::ImageWriterEvent::waitBeforeCollectData()
{
	//Make sure all images are extracted from camera.
	//This should happen automatically if this event is after all image events in the set. (?)
}


//TODO: Have the server transmit this information...
std::string BlackflyDevice::generateDataDestinationDirectory()
{
	//	std::string baseDirectory = "C:\\Users\\Jason\\Code\\dev\\stidatatest\\";
	char dirSepChar = baseDirectory.back();

	std::stringstream path;

	std::time_t t = std::time(0);   // get time now
	struct tm * now = std::localtime(&t);

	path << baseDirectory << (now->tm_year + 1900) << dirSepChar
		<< (now->tm_mon + 1) << dirSepChar
		<< now->tm_mday << dirSepChar
		<< "data";

	return path.str();
}

std::string BlackflyDevice::generateFileTimestamp()
{
	std::stringstream file;

	std::time_t t = std::time(0);   // get time now
	struct tm * now = std::localtime(&t);

	char sepChar = '_';

	//Example:  2018_7_3-19_03_55-test.tif
	file
		<< (now->tm_year + 1900) << sepChar
		<< std::setfill('0') << std::setw(2) << (now->tm_mon + 1) << sepChar
		<< std::setfill('0') << std::setw(2) << now->tm_mday
		<< std::setw(1) << "-"
		<< std::setfill('0') << std::setw(2) << now->tm_hour << sepChar
		<< std::setfill('0') << std::setw(2) << now->tm_min << sepChar
		<< std::setfill('0') << std::setw(2) << now->tm_sec
		<< std::setw(1) << "-";

	return file.str();
}
