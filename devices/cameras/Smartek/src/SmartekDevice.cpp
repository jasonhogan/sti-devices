
#include "SmartekDevice.h"
#include "Image.h"
#include "ImageWriter.h"

#include <thread>

SmartekDevice::SmartekDevice(ORBManager* orb_manager, const ConfigFile& configFile, const smcs::IDevice& camera)
	: STI_Device_Adapter(orb_manager, configFile), camera(camera)
{
	init();

	configFile.getParameter("Trigger partner name", trigger.name);
	configFile.getParameter("Trigger partner IP", trigger.ip);
	configFile.getParameter("Trigger partner module", trigger.module);
	configFile.getParameter("Trigger partner channel", trigger.channel);

	configFile.getParameter("Trigger Low Value", trigger.low);
	configFile.getParameter("Trigger High Value", trigger.high);
	configFile.getParameter("Trigger duration (ns)", trigger.duration_ns);
	configFile.getParameter("Trigger Reset Holdoff (ns)", trigger.resetHoldoff_ns);		//holdoff time before event when LOW is asserted

	configFile.getParameter("STI Data Base Directory", baseDirectory);

}

void SmartekDevice::init()
{
	camera->SetIntegerNodeValue("TLParamsLocked", 0);
	camera->SetStringNodeValue("PixelFormat", "Mono10Packed"); //Allowed values: "Mono8" or "Mono10Packed"

	externalTriggerEventsOn = false;
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

	addAttribute("Generate Trigger Events", (externalTriggerEventsOn ? "True" : "False"), "True, False");
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

	if (key.compare("Generate Trigger Events") == 0) {
		externalTriggerEventsOn = value.compare("True") == 0;
		success = true;
	}

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

	bool softTrigger = getAttribute("TriggerSource").compare("Software") == 0;
	setAttribute("Generate Trigger Events", ((externalTriggerEventsOn && !softTrigger) ? "True" : "False"));
}

void SmartekDevice::defineChannels()
{
	addInputChannel(0, DataString, ValueString, "Single Image");	// filename
	addInputChannel(1, DataString, ValueVector, "Group Image");		//(filename, new_group)
	addInputChannel(2, DataString, ValueVector, "Mean Image");		//(filename, new_group)
	addInputChannel(4, DataDouble, ValueNone, "Photodetector");
}


void SmartekDevice::definePartnerDevices()
{
	addPartnerDevice("External Trigger", trigger.ip, trigger.module, trigger.name);
	partnerDevice("External Trigger").enablePartnerEvents();
}

std::string SmartekDevice::getDeviceHelp()
{
	return "";
}

bool SmartekDevice::parseEventValue(const std::vector<RawEvent>& rawEvents, SmartekDeviceEventValue& value, std::string& message)
{
	if (rawEvents.size() != 1) {
		message = "Only one image event at a time is allowed.";
		return false;
	}

	value.channel = rawEvents.at(0).channel();

	bool success = true;

	//type checking
	switch (value.channel) {
	case 0:	//single image
		if (rawEvents.at(0).getValueType() != MixedValue::String) {
			message = "Camera filename must be a string.";
			success = false;
		}
		break;
	case 1:	//group image
	case 2:	//Mean image
	{
		auto tuple = rawEvents.at(0).value().getVector();
		if (tuple.size() == 2) {
			if (tuple.at(0).getType() != MixedValue::String
				|| (tuple.at(1).getType() != MixedValue::Double && tuple.at(1).getType() != MixedValue::Boolean)) {
				message = "Invalid vector format. Expecting (<string>filename) or (<string>filename, <bool>new_group)";
				success = false;
			}
		}
		else {
			message = "Tuple should be of the the form (<string>filename, <bool>new_group).";
			success = false;
		}
		break;
	}
	case 4:	//photodetector
		break;
	default:
		message = "Channel number " + STI::Utils::valueToString(value.channel) + " is not supported.";
		return false;
	}

	if (!success) return false;

	value.newGroup = false;

	//decode value
	switch (value.channel) {
	case 0:	//single image
		value.baseFilename = rawEvents.at(0).value().getString();
		break;
	case 1:	//group image
	case 2:	//Mean image
	{
		auto tuple = rawEvents.at(0).value().getVector();
		value.baseFilename = tuple.at(0).getString();
		if (tuple.at(1).getType() == MixedValue::Boolean) {
			value.newGroup = tuple.at(1).getBoolean();
		}
		else {
			value.newGroup = (tuple.at(1).getNumber() > 0);
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

void SmartekDevice::generateExternalTriggerEvents(double eventTime, const RawEvent& sourceEvt)
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

void SmartekDevice::parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut)
{
//	status = device->CommandNodeExecute("AcquisitionStart");
	//ch 0: Individual image
	//ch 1: Image group.  By default, groups all images taken in a given shot into one multi-page tif. Has argument "new_group" that starts a new group (false by default)
	//ch 2: Image group with operation. Examples: Average, Absorption (1-2)/3.  Does not save intermediate images.
	//ch 3: Absorption (1-2)/(2-3), 1=Signal, 2=Reference, 3=Background
	//ch 4: photodiode mode; integrate all pixels
	double minimumEventTime = 1000;	//1 us
	double eventTime = minimumEventTime;	//from timing file
	double lastEventTime = minimumEventTime;
	double thisEventTime = minimumEventTime;
	double thisWriterEventTime = minimumEventTime;
	double holdoff = 0*10000000;		//10 ms
//	RawEventMap::const_iterator events;

	SmartekDeviceEventValue value;
	std::string err_message;

	shared_ptr<Image> image;

	shared_ptr<Image> image_mean;		//shared by multiple events
	shared_ptr<Image> image_mean_result;	//buffer shared by multiple events

	shared_ptr<ImageWriter> singleWriter;
	shared_ptr<ImageWriter> groupWriter;
	shared_ptr<ImageWriter> groupWriter2;

	std::unique_ptr<ImageWriterEvent> groupWriterEvent;
	std::unique_ptr<ImageWriterEvent> groupWriterEvent2;

	unsigned totalImages = 0;
	unsigned imageGroupCount = 0;
	unsigned imageGroupCount2 = 0;

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

		if (value.channel == 0) {	//Individual image
			
			generateExternalTriggerEvents(eventTime, events->second.at(0));		//conditional on attribute status

			totalImages++;
			image = std::make_shared<Image>(sizeY, sizeX);

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

			auto smartekEvent = std::make_unique<SmartekEvent>(thisEventTime, this, image);	//schedule event to plau immediately after last event

			eventsOut.push_back(smartekEvent.release());

			singleWriter = std::make_shared<ImageWriter>();		//can be shared by multiple events, if images are to be combined into one file (multipane tif)

			filename = value.baseFilename + filenameext;

			auto writer = std::make_unique<ImageWriterEvent>(thisWriterEventTime, this, singleWriter, filename);
			writer->addImage(image);
			writer->addMeasurement( events->second.at(0) );		//register the measurement with the source RawEvent

			eventsOut.push_back(writer.release());
		}

		if (value.channel == 1) {	//image group
			
			if (value.newGroup && imageGroupCount > 0 && groupWriterEvent != 0) {	//New group created; flush the group image buffer
				groupWriterEvent->setTime(lastEventTime + 10);
				eventsOut.push_back(groupWriterEvent.release());
				imageGroupCount = 0;
			}
			if (imageGroupCount == 0) {	//new group created
				groupWriter = std::make_shared<ImageWriter>();
				filename = value.baseFilename + filenameext;	//TODO: fix
				groupWriterEvent = std::make_unique<ImageWriterEvent>(lastEventTime + 10, this, groupWriter, filename);	//overwrites each time
			}
			
			generateExternalTriggerEvents(eventTime, events->second.at(0));		//conditional on attribute status
			
			imageGroupCount++;
			totalImages++;
			image = std::make_shared<Image>(sizeY, sizeX);

			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime, this, image);	//schedule event to play immediately after last event

			eventsOut.push_back(smartekEvent.release());

			groupWriterEvent->addImage(image);
			groupWriterEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent
		}

		if (value.channel == 2) {	//image group with operation.  (Mean)

			if (value.newGroup && imageGroupCount2 > 0 && groupWriterEvent2 != 0) {	//New group created; flush the group image buffer
				groupWriterEvent2->setTime(lastEventTime + 10);
				eventsOut.push_back(groupWriterEvent2.release());
				imageGroupCount2 = 0;
			}
			if (imageGroupCount2 == 0) {	//new group created
				groupWriter2 = std::make_shared<ImageWriter>();
				filename = value.baseFilename + filenameext;	//TODO: fix
				groupWriterEvent2 = std::make_unique<ImageWriterEvent>(lastEventTime + 10, this, groupWriter2, filename);	//overwrites each time

				image_mean = std::make_shared<Image>(sizeY, sizeX);	
				image_mean_result = std::make_shared<Image>(sizeY, sizeX);
				groupWriterEvent2->addImage(image_mean_result);	//only one image; construct the mean by adding each new image to this as they are captured.
			}

			imageGroupCount2++;
			totalImages++;
			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime, this, image_mean, image_mean_result, Mean);

			smartekEvent->imageCount = imageGroupCount2;

			eventsOut.push_back(smartekEvent.release());

			groupWriterEvent2->addMeasurement(events->second.at(0));	//register the measurement with the source RawEvent
		}

		if (value.channel == 4) {	// Photodetector mode: integrate all pixels
			image = std::make_shared<Image>(sizeY, sizeX);
			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime + 10, this, image, Photodetector);	//schedule event to plau immediately after last event

			smartekEvent->addMeasurement(events->second.at(0));

			eventsOut.push_back(smartekEvent.release());
		}

		lastEventTime = eventTime;
	}

	if (imageGroupCount > 0 && groupWriterEvent != 0) {	//flush the group image buffer
		groupWriterEvent->setTime(lastEventTime + 10);
		eventsOut.push_back(groupWriterEvent.release());
		imageGroupCount = 0;
	}

	//Initialization event.  Happens before any image events
	auto smartekIniEvent = std::make_unique<SmartekInitializeEvent>(0, this, totalImages);
	eventsOut.push_back(smartekIniEvent.release());

}

//void operate(const Image& newImage, Image& result, SmartekDevice::Operation operation)
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

void SmartekDevice::SmartekInitializeEvent::playEvent()
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


SmartekDevice::SmartekEvent::SmartekEvent(double time, SmartekDevice* cameraDevice_, const shared_ptr<Image>& image, const shared_ptr<Image>& imageBuffer, SmartekEventMode mode)
	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image), imageBuffer(imageBuffer), mode(mode)
{
	imageCount = 0;
}

SmartekDevice::SmartekEvent::SmartekEvent(double time, SmartekDevice* cameraDevice_, const shared_ptr<Image>& image, SmartekEventMode mode)

	: SynchronousEventAdapter(time, cameraDevice_), cameraDevice(cameraDevice_), image(image), mode(mode)
{
	image->imageData.reserve(image->getImageSize());	//reserve memory for image data
	imageCount = 0;
}

SmartekDevice::SmartekEvent::SmartekEvent(double time, SmartekDevice* cameraDevice_, const shared_ptr<Image>& image)
	: SmartekDevice::SmartekEvent::SmartekEvent(time, cameraDevice_, image, Normal)
{
}

void SmartekDevice::SmartekEvent::playEvent()
{
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

bool SmartekDevice::isHardwareTriggered()
{
	//Trigger setup
	std::string trigSource;
	camera->GetStringNodeValue("TriggerSource", trigSource);

	return (trigSource.compare("Software") != 0);
}


void SmartekDevice::SmartekEvent::stop()
{
	//Abort
	bool status;
	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStop");
}

void SmartekDevice::SmartekEvent::waitBeforeCollectData()
{
	bool success = false;

	while (!success && cameraDevice->running()) {
		success = cameraDevice->camera->WaitForImage(1);	//1 second timeout
//		cout << "WaitForImage: " << getEventNumber()  << " pending: " << cameraDevice->camera->GetPendingImagesCount() << endl;
	}
}


void SmartekDevice::SmartekEvent::collectMeasurementData()
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

	image->addMetaData("CameraTimestamp", STI::Utils::valueToString(imageInfo->GetCameraTimestamp()));

	UINT32 sizeX, sizeY;
	imageInfo->GetSize(sizeX, sizeY);

	image->setImageHeight(sizeY);
	image->setImageWidth(sizeX);

	int linesize = imageInfo->GetLineSize();
	int rawdatasize = imageInfo->GetRawDataSize();

	cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);

	image->imageData.clear();

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

Int64 SmartekDevice::SmartekEvent::getTotal(const vector<IMAGEWORD>& data)
{
	Int64 total = 0;
	for (auto& d : data) {
		total += d;
	}
	return total;
}


void SmartekDevice::unpackLine(UINT8* rawLine, std::vector<IMAGEWORD>& unpackedLine)
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

void SmartekDevice::ImageWriterEvent::addImage(const std::shared_ptr<Image>& image)
{
	imageWriter->addImage(image);
}


void SmartekDevice::ImageWriterEvent::collectMeasurementData()
{
	std::string baseDir = cameraDevice->generateDataDestinationDirectory();

	//filename is specific to this shot, at the time of data collection
	std::string filename = 
		cameraDevice->generateDataDestinationDirectory() + "\\" +
		cameraDevice->generateFileTimestamp() + basefilename;

	if (eventMeasurements.size() == 1) {
		//single image
		eventMeasurements.at(0)->setData(filename);
	}

	if (eventMeasurements.size() > 1) {
		//group image (multipane tif)
		MixedData data;

		MixedData vec;
		vec.addValue("Group ID");
		vec.addValue(static_cast<int>(getEventNumber()));
		vec.addValue("ImageCount");
		vec.addValue(static_cast<int>(eventMeasurements.size()));
		vec.addValue("Pane");
		vec.addValue(static_cast<int>(0));	//temp; gets overridden

		for (unsigned i = 0; i < eventMeasurements.size(); ++i) {
			data.clear();
			data.addValue(filename);
			vec.getValueAt(5).setValue(static_cast<int>(i + 1));	//pane number
			data.addValue(vec);

			eventMeasurements.at(i)->setData(data);
		}
	}

	imageWriter->write(filename);
}

void SmartekDevice::ImageWriterEvent::waitBeforeCollectData()
{
	//Make sure all images are extracted from camera.
	//This should happen automatically if this event is after all image events in the set. (?)
}


//TODO: Have the server transmit this information...
std::string SmartekDevice::generateDataDestinationDirectory()
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

std::string SmartekDevice::generateFileTimestamp()
{
	std::stringstream file;

	std::time_t t = std::time(0);   // get time now
	struct tm * now = std::localtime(&t);

	char sepChar = '_';

	//Example:  2018_7_3-19_03_55-test.tif
	file
		<< (now->tm_year + 1900) << sepChar
		<< (now->tm_mon + 1) << sepChar
		<< now->tm_mday << sepChar << "-"
		<< now->tm_hour << sepChar
		<< now->tm_min << sepChar
		<< now->tm_sec << sepChar;

	return file.str();
}
