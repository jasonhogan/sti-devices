
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
	addAttribute("Exposure time", camera.getExposureTime());

	addAttribute("Camera temperature", camera.getTemperature());
	addAttribute("Cooler setpoint", camera.coolerSetpoint);

	addAttribute("Horizontal bin size", 1);
	addAttribute("Vertical bin size", 1);

//	int roiI[] = {0,0,1340,400};
//	vector<int> v(roiI, roiI+3);
	vector<int> v {0, 0, 1340, 400};
	addAttribute("ROI", vectorToString(v));

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
	return string("Information for PIXIS device.\n\n")
		+ "The crop vector is of the form(int X, int Y, int WidthX, int HeightY)\n"
		+ "The ROI is defined by the corner positied at {X,Y} and sizes WidthX and HeightY.";
}


bool PixisDevice::updateAttribute(std::string key, std::string value)
{
	int tempInt;
	double tempDouble;
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
	else if (key.compare("Exposure time") == 0) {
		if (success = STI::Utils::stringToValue(value, tempDouble)) {
			success = camera.setExposureTime(tempDouble);
		}
	}
	return success;
}

void PixisDevice::refreshAttributes()
{
	setAttribute("Exposure time", camera.getExposureTime());

	setAttribute("Camera temperature", camera.getTemperature());
	setAttribute("Cooler setpoint", camera.coolerSetpoint);
	
	setAttribute("Horizontal bin size", camera.hbin);
	setAttribute("Vertical bin size", camera.vbin);

	setAttribute("ROI", refreshROI());

}

std::string PixisDevice::refreshROI()
{
	camera.refreshROI();

	std::vector<int> roi;
	//roi.push_back(camera.roi.X);
	//roi.push_back(camera.roi.Y);
	//roi.push_back(camera.roi.widthH);
	//roi.push_back(camera.roi.widthV);

	camera.getROI(roi);

	return vectorToString(roi);
}


bool PixisDevice::parseEventValue(const std::vector<MixedValue>& tuple, PixisDeviceEventValue& value, std::string& message)
{
	bool success = true;

	//Check that each type in tuple is correct. The first case statements
	//are deliberately not breaked.

	if (tuple.size() >= 3) {
		if (tuple.at(2).getType() != MixedValue::Vector) {
			message = "Camera crop vector must be a vector";
			success = false;
		}
	}
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
		message = "Camera commands must be a tuple in the form (string description, string filename, vector cropVector). The crop vector can be of the form (int X, int Y, int WidthX, int HeightY)";  // or (int centerPixelX, int centerPixelY, int halfWidth).
		success = false;
	}

	if (!success) return success;

	switch (tuple.size())
	{
	case 3:
		value.description = tuple.at(0).getString();
		value.baseFilename = tuple.at(1).getString();
		//		value.cropVector = tuple.at(2).getVector();
		break;
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

//	int imageIndex = 0;

	shared_ptr<Image> image;
	shared_ptr<ImageWriter> imageWriter;

	std::vector<int> cropVector;

	////Should be default image size here?  Overwritten by the device event value.
	//eventValue.cropVector.push_back(0);
	//eventValue.cropVector.push_back(0);
	//eventValue.cropVector.push_back(0);
	//eventValue.cropVector.push_back(0);

	//eventValue.cropVector.at(0) = 1;
	//eventValue.cropVector.at(1) = 100;
	//eventValue.cropVector.at(2) = 1;
	//eventValue.cropVector.at(3) = 100;

	resetImageIndex();	//global image index increments each time an image is added, to ensure a unique image index for each image

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

		camera.getROI(cropVector);
		image = std::make_shared<Image>(cropVector, camera.vbin, camera.hbin);
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
		auto ikonEvent = std::make_unique<PixisEvent>(eventTime, this, getNextImageIndex(), image, imageWriter);

		ikonEvent->image->rotationAngle = 0;
		
		ikonEvent->addMeasurement( events->second.at(0) );		//register the measurement with the source RawEvent

		eventsOut.push_back(ikonEvent.release());
	}
}

void PixisDevice::resetImageIndex()
{
	globalImageIndex = 0;
}

int PixisDevice::getNextImageIndex() {
	globalImageIndex++;
	return globalImageIndex;
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

	cameraDevice->camera.StartAcquisition(imageIndex);

}

void PixisDevice::PixisEvent::waitBeforeCollectData()
{

	cameraDevice->camera.WaitForAcquisition(imageIndex);

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



//**** These functions belong in STI::Utils *****//
std::string PixisDevice::vectorToString(const vector<int>& vec)
{
	std::stringstream strm;
	strm << "(";

	for (unsigned i = 0; i < vec.size(); i++) {
		strm << STI::Utils::valueToString(vec.at(i));
		if (i < vec.size() - 1) {
			strm << ", ";
		}
	}
	strm << "(";

	return strm.str();
}

bool PixisDevice::stringToVector(const std::string& input, vector<int>& outVec)
{
	std::vector<string> tmp;
	STI::Utils::splitString(input, "(", tmp);
	if (tmp.size() < 2) return false;
	STI::Utils::splitString(tmp.at(1), ")", tmp);
	if (tmp.size() < 2) return false;

	STI::Utils::splitString(tmp.at(0), ",", tmp);

	outVec.clear();
	int val;
	for (auto it : tmp) {
		STI::Utils::stringToValue(it, val);
		outVec.push_back(val);
	}
}
