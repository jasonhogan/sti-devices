
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
	camera->SetIntegerNodeValue("TLParamsLocked", 0);
	camera->SetStringNodeValue("PixelFormat", "Mono10Packed"); //Allowed values: "Mono8" or "Mono10Packed"
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
	addInputChannel(1, DataString, ValueVector);
	addInputChannel(4, DataDouble, ValueVector);
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
	if (value.channel == 0) {	//single image
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
	}

	if (value.channel == 1) {	//group image:  (desc); (desc, filename); (desc, new_group); (desc, filename, new_group);
		if (tuple.size() == 3) {
			if (tuple.at(0).getType() != MixedValue::String
				|| tuple.at(1).getType() != MixedValue::String
				|| tuple.at(2).getType() != MixedValue::Boolean) {
				message = "Invalid tuple format. Expecting (<string>description, <string>filename, <bool>new_group)";
				success = false;
			}
		}
		if (tuple.size() == 2) {
			if (tuple.at(0).getType() != MixedValue::String
				|| (tuple.at(1).getType() != MixedValue::String && tuple.at(1).getType() != MixedValue::Boolean)) {
				message = "Invalid tuple format. Expecting (<string>description, <string>filename) or (<string>description, <bool>new_group)";
				success = false;
			}
		}
		if (tuple.size() >= 1) {
			if (tuple.at(0).getType() != MixedValue::String) {
				message = "Invalid tuple format. Expecting (<string>description)";
				success = false;
			}
		}
		else {
			message = "Tuple should be of the the form (<string>description, <string>filename, <bool>new_group).";  // or (int centerPixelX, int centerPixelY, int halfWidth).
			success = false;
		}
	}

	if (!success) return false;

	value.newGroup = false;

	if (value.channel == 0) {	//single image
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
			message = "Never should get here, but Andor camera commands must be a tuple in the form (<string>description, <string>filename)";
			success = false;
			break;
		}
	}

	if (value.channel == 1) {	//group image
		switch (tuple.size())
		{
		case 3:
			value.description = tuple.at(0).getString();
			value.baseFilename = tuple.at(1).getString();
			value.newGroup = tuple.at(2).getBoolean();
			break;
		case 2:
			value.description = tuple.at(0).getString();

			if (tuple.at(1).getType() == MixedValue::String) {
				value.baseFilename = tuple.at(1).getString();
			}
			else {
				value.newGroup = tuple.at(1).getBoolean();
			}
			break;
		case 1:
			value.description = tuple.at(0).getString();
			value.baseFilename = "default";
			break;
		default:
			message = "Never should get here, but Andor camera commands must be a tuple in the form (<string>description, <string>filename, <bool>new_group)";
			success = false;
			break;
		}
	}
	return success;
}

void SmartekDevice::parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut)
{
//	status = device->CommandNodeExecute("AcquisitionStart");
	//ch 0: Individual image
	//ch 1: Image group.  By default, groups all images taken in a given shot into one multi-page tif. Has argument "new_group" that starts a new group (false by default)
	//ch 2: Image group with operation. Examples: Average, Absorption (1-2)/3.  Does not save intermediate images.
	//ch 3: Absorption (1-2)/(2-3), 1=Signal, 2=Reference, 3=Background
	//ch 4: photodiode mode; integrate all pixels

	double eventTime = 0;
	double lastEventTime = 0;
	double holdoff = 10000000;		//10 ms
//	RawEventMap::const_iterator events;

	SmartekDeviceEventValue value;
	std::string err_message;

	shared_ptr<Image> image;

	shared_ptr<ImageWriter> singleWriter;
	shared_ptr<ImageWriter> groupWriter;

	std::unique_ptr<ImageWriterEvent> groupWriterEvent;

	unsigned totalImages = 0;
	unsigned imageGroupCount = 0;

	//std::string filenametemp = "C:\\Users\\Jason\\Code\\dev\\sti-devices\\devices\\cameras\\Smartek\\project\\Release\\test.tif";
	std::string filenamedir = "C:\\Users\\Jason\\Code\\dev\\sti-devices\\devices\\cameras\\Smartek\\project\\Release\\";
	std::string filenamebase = "test";
	std::string filenameext = ".tif";
	std::string filename;

	for (auto events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (!parseEventValue(events->second, value, err_message)) {
			throw EventParsingException(events->second.at(0), err_message);
		}

		eventTime = events->first - holdoff;

		if (value.channel == 0) {	//Individual image
			
			//smcs::IImageInfo imageInfo = nullptr;
			//UINT32 sizeX, sizeY;
			//camera->GetImageInfo(&imageInfo);
			//imageInfo->GetSize(sizeX, sizeY);

			totalImages++;
			image = std::make_shared<Image>(1, 1);
			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime + 10, this, image);	//schedule event to plau immediately after last event

			eventsOut.push_back(smartekEvent.release());

			if (singleWriter == 0 || true) {
				singleWriter = std::make_shared<ImageWriter>();		//can be shared by multiple events, if images are to be combined into one file (multipane tif)
			}

			filename = filenamedir + value.baseFilename + filenameext;	//TODO: fix

			auto writer = std::make_unique<ImageWriterEvent>(eventTime, this, singleWriter, filename);
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
				filename = filenamedir + value.baseFilename + filenameext;	//TODO: fix
				groupWriterEvent = std::make_unique<ImageWriterEvent>(lastEventTime + 10, this, groupWriter, filename);	//overwrites each time
			}
			
			imageGroupCount++;
			totalImages++;
			image = std::make_shared<Image>(1, 1);
			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime, this, image);	//schedule event to plau immediately after last event

			eventsOut.push_back(smartekEvent.release());

			groupWriterEvent->addImage(image);
			groupWriterEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent
		}

		if (value.channel == 2) {	//image group with operation.  (Sum, Mean, Subtract)

			if (value.newGroup && imageGroupCount > 0 && groupWriterEvent != 0) {	//New group created; flush the group image buffer
				groupWriterEvent->setTime(lastEventTime + 10);
				eventsOut.push_back(groupWriterEvent.release());
				imageGroupCount = 0;
			}
			if (imageGroupCount == 0) {	//new group created
				groupWriter = std::make_shared<ImageWriter>();
				filename = filenamedir + value.baseFilename + filenameext;	//TODO: fix
				groupWriterEvent = std::make_unique<ImageWriterEvent>(lastEventTime + 10, this, groupWriter, filename);	//overwrites each time
			}

			imageGroupCount++;
			totalImages++;
			image = std::make_shared<Image>(1, 1);
			auto smartekEvent = std::make_unique<SmartekEvent>(lastEventTime, this, image);	//schedule event to plau immediately after last event

			eventsOut.push_back(smartekEvent.release());

			groupWriterEvent->addImage(image);
			groupWriterEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent
		}

		lastEventTime = eventTime;
	}

	if (imageGroupCount > 0 && groupWriterEvent != 0) {	//flush the group image buffer
		groupWriterEvent->setTime(lastEventTime + 10);
		eventsOut.push_back(groupWriterEvent.release());
		imageGroupCount = 0;
	}

	camera->SetIntegerNodeValue("TLParamsLocked", 0);	//unlock
	camera->SetIntegerNodeValue("AcquisitionFrameCount", totalImages);

	UINT32 pendingImages;

	cout << "Parsed::totalImages = " << totalImages << endl;

	camera->SetImageBufferFrameCount(totalImages);

	INT64 frameCount;
	camera->GetIntegerNodeValue("AcquisitionFrameCount", frameCount);

	pendingImages = camera->GetPendingImagesCount();
	cout << "Parsed::Pending: " << pendingImages << endl;
	cout << "Parsed::frameCount: " << frameCount << endl;


	//camera->WaitForImage()

	//smcs_ICameraAPI_RegisterCallback(smcsApi, smcs_ICallbackEvent_Handler);

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
	cout << "Playing" << endl;

	//TEMP
	bool status;
	// set trigger selector to frame start
	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);
	status = cameraDevice->camera->SetStringNodeValue("TriggerSelector", "FrameStart");

	status = cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 1);
	status = cameraDevice->camera->CommandNodeExecute("AcquisitionStart");

	//Trigger setup
	std::string trigSource;
	status = cameraDevice->camera->GetStringNodeValue("TriggerSource", trigSource);

	if (trigSource.compare("Software") == 0) {
		status = cameraDevice->camera->CommandNodeExecute("TriggerSoftware");	//software trigger
	}
	else {
		status = cameraDevice->camera->CommandNodeExecute("Line1");		//hardware trigger
	}
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
	//query acquisition status, or use callback. possibly use image count when starting acq
	//std::this_thread::sleep_for(std::chrono::seconds(5));	//TEMP


	bool success = false;

	while (!success && cameraDevice->running()) {
		success = cameraDevice->camera->WaitForImage(1);	//1 second timeout
		cout << "WaitForImage: " << success << endl;
	}

	//cout << "WaitForImage: " << success << endl;

	//UINT32 pendingImages;

	//do {
	//	pendingImages = cameraDevice->camera->GetPendingImagesCount();
	//	cout << "Pending: " << pendingImages << endl;
	//} while (pendingImages > 0);


	//INT64 frameCount;

	//int i = 0;
	//do {
	//	cameraDevice->camera->GetIntegerNodeValue("AcquisitionFrameCount", frameCount);
	//	cout << "frameCount = " << frameCount << endl;
	//	std::this_thread::sleep_for(std::chrono::milliseconds(100));	//TEMP
	//	i++;
	//} while (frameCount > 0 && i < 1000);


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

	int linesize = imageInfo->GetLineSize();
	int rawdatasize = imageInfo->GetRawDataSize();


	UINT32 pixelType;
	imageInfo->GetPixelType(pixelType);

	cameraDevice->camera->SetIntegerNodeValue("TLParamsLocked", 0);

	bool status;
	std::string pixelFormat;
	status = cameraDevice->camera->GetStringNodeValue("PixelFormat", pixelFormat);

//	UINT8* data = imageInfo->GetRawData();
//	image->imageData;
	
//	test.data();
//	test.insert(test.end(), &data[0], &data[dataArraySize]);
	
//	std::vector<char> test;
//	test.clear();

	image->imageData.clear();

	//copy image from camera buffer, line by line
	UINT8* lineData;
	std::vector<IMAGEWORD> unpackedLine(sizeX, 0);
	for (unsigned int j = 0; j < sizeY; j++) {	// j is image line number
		//Get next line and insert it into vector
		lineData = imageInfo->GetRawData(j);
		cameraDevice->unpackLine(lineData, unpackedLine);
//		image->imageData.insert(image->imageData.end(), &lineData[0], &lineData[imageInfo->GetLineSize()]);
		image->imageData.insert(image->imageData.end(), unpackedLine.begin(), unpackedLine.end());
	}

	cameraDevice->camera->PopImage(imageInfo);	//removes image from camera buffer

//	for (unsigned int j = 0; j < sizeY; j++)
//		::memcpy(m_svBitmap.GetRawData(j), imageInfo->GetRawData(j), imageInfo->GetLineSize());
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


