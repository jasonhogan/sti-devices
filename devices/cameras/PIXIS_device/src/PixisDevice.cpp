
#include "PixisDevice.h"

#include <string>
using std::string;


PixisDevice::PixisDevice(ORBManager* orb_manager, std::string configFilename)
	: STI_Device_Adapter(orb_manager, "PIXIS camera", configFilename) 
{
	camera.InitializeCamera();

}

void PixisDevice::defineAttributes()
{
	addAttribute("Camera temperature", camera.getTemperature());
	addAttribute("Cooler setpoint", camera.coolerSetpoint);

	addAttribute("Horizontal bin size", 1);
	addAttribute("Vertical bin size", 1);

}

void PixisDevice::defineChannels()
{
	addInputChannel(0, DataString, ValueVector);
}

void PixisDevice::definePartnerDevices()
{
//	addPartnerDevice("ext_trigger", "ep-timing1.stanford.edu", 2, "Digital Out");
}

std::string PixisDevice::getDeviceHelp()
{
	return "Information for PIXIS device.";

}


bool PixisDevice::updateAttribute(std::string key, std::string value)
{
	int tempInt;
	bool success = true;

	if (key.compare("Cooler setpoint") == 0) {
		if (success = STI::Utils::stringToValue(value, tempInt)) {
			success = camera.setTemperature(tempInt);
		}
	}
	else if (key.compare("Horizontal bin size") == 0) {
		if (success = STI::Utils::stringToValue(value, tempInt)) {
			success = camera.setHBin(tempInt);
		}
	}
	else if (key.compare("Vertical bin size") == 0) {
		if (success = STI::Utils::stringToValue(value, tempInt)) {
			success = camera.setVBin(tempInt);
		}
	}

	return success;
}

void PixisDevice::refreshAttributes()
{
	setAttribute("Camera temperature", camera.getTemperature());
	setAttribute("Cooler setpoint", camera.coolerSetpoint);
	
	setAttribute("Horizontal bin size", camera.hbin);
	setAttribute("Vertical bin size", camera.vbin);

}

bool PixisDevice::parseEventValue(const std::vector<MixedValue>& tuple, PixisDeviceEventValue& value, std::string& message)
{
	message = "";
	return true;
}

void PixisDevice::parseDeviceEvents(const RawEventMap &eventsIn,
	SynchronousEventVector& eventsOut) throw(std::exception)
{
	double eventTime;
	double holdoff = 100000000;		//100 ms
	RawEventMap::const_iterator events;

	PixisDeviceEventValue eventValue;
	std::string err_message;

	int imageIndex = 0;

	shared_ptr<Image> image;
	shared_ptr<ImageWriter> imageWriter;

//	std::vector<int> cropVector(4, 0);

	//Should be default image size here?  Overwritten by the device event value.
	eventValue.cropVector.push_back(0);
	eventValue.cropVector.push_back(0);
	eventValue.cropVector.push_back(0);
	eventValue.cropVector.push_back(0);

	eventValue.cropVector.at(0) = 1;
	eventValue.cropVector.at(1) = 10;
	eventValue.cropVector.at(2) = 1;
	eventValue.cropVector.at(3) = 10;

	for (events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (events->second.at(0).getValueType() != MixedValue::Vector) {
//			std::cerr << "The camera does not support that data type" << std::endl;		
			throw EventParsingException(events->second.at(0),
				"Invalid data type.  The camera expects a vector.");
		}
//		const std::vector<MixedValue>& eVector = events->second.at(0).value().getVector();
		auto eventTuple = events->second.at(0).value().getVector();

		if (!parseEventValue(eventTuple, eventValue, err_message)) {
			throw EventParsingException(events->second.at(0), err_message);
		}

		eventTime = events->first - holdoff;

		image = std::make_shared<Image>(eventValue.cropVector, camera.vbin, camera.hbin);
		image->filename = eventValue.baseFilename + "_time" ;		
		//the filename used by ImageWriter must include the target path.  
		//Not sure a good way to get this here.  
		//The quantix hacks it by having the Camera object independently generate the directory, 
		//assuming the data folder structure.
		//Maybe make the channel return a DataFile object.  Have server populate this with target directory?
		//Could make a bool in DataFile that specifies where the data should be transfered to the server, or just saved directly.
		//Currently the server doesn't retrieve file data.  It also doesn't save it to the XML (DOMNodeWrapper doesn't add file data to XML).
		//Would need to modify server to fix this.

		image->extension = "tif";

		imageWriter = std::make_shared<ImageWriter>();		//can be shared by multiple events, if images are to be combined into one file (multipane tiff)

		//std::unique_ptr<PixisEvent>
		auto ikonEvent = std::make_unique<PixisEvent>(eventTime, this, imageIndex, image, imageWriter);

		ikonEvent->image->rotationAngle = 0;
		
		ikonEvent->addMeasurement( events->second.at(0) );		//register the measurement with the source RawEvent

		eventsOut.push_back(ikonEvent.release());
	}
}

void PixisDevice::PixisEvent::reset()
{
	SynchronousEvent::reset();

	imageWriter->reset();
}

void PixisDevice::PixisEvent::stop()
{
	SynchronousEvent::stop();

	cameraDevice->camera.StopAcquisition();
}

void PixisDevice::PixisEvent::loadEvent()
{
	int errorValue; 
	
//	errorValue = FreeInternalMemory();
//	cameraDevice->camera.checkError(errorValue, "FreeInternalMemory");
}


void PixisDevice::PixisEvent::playEvent()
{
	int errorValue;

	//errorValue = SetImage(image->hbin, image->vbin, 
	//	image->cropVector.at(0), 
	//	image->cropVector.at(1), 
	//	image->cropVector.at(2), 
	//	image->cropVector.at(3));
	////SetImage(int hbin, int vbin, int hstart, int hend, int vstart, int vend)
	//if (!cameraDevice->camera.checkError(errorValue, "SetImage"))
	//	return;

	cameraDevice->camera.StartAcquisition();

}

void PixisDevice::PixisEvent::waitBeforeCollectData()
{

	cameraDevice->camera.WaitForAcquisition();

	//long first;
	//long last;
	//int errorValue;

	//errorValue = GetNumberNewImages(&first, &last);
	//cameraDevice->camera.checkError(errorValue, "GetNumberNewImages");

	//if (last >= imageIndex) {
	//	//Image has been aquired already.  Don't wait anymore.
	//}
	//else {
	//	errorValue = WaitForAcquisition();
	//	cameraDevice->camera.checkError(errorValue, "WaitForAcquisition");
	//}

}

void PixisDevice::PixisEvent::collectMeasurementData()
{
	if (cameraDevice->camera.getImage(imageIndex, image)) {
		if (eventMeasurements.size() == 1)
		{
			eventMeasurements.at(0)->setData(image->getFilename());
		}

		imageWriter->addImage(image);
		imageWriter->write(image->getFilename());
	}
	else {
		if (eventMeasurements.size() == 1)
		{
			eventMeasurements.at(0)->setData("Error getting image.");
		}
	}


}

