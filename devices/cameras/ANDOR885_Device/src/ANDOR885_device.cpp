//MUST INCLUDE ATMCD32M.lib among linker files. Keep this lib files and the atmcd32d.h files in the same directory as these files


/*! \file ANDOR885_device.cpp
 *  \author Susannah Dickerson 
 *  \brief Source file for the iXon DV-885 camera
 *  \section license License
 *
 *  Copyright (C) 2009 Susannah Dickerson <sdickers@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
 *	
 *	This structure shamlessly derived from source code originally by Jason
 *	Hogan <hogan@stanford.edu> and David M.S. Johnson <david.m.johnson@stanford.edu>
 *
 *  The STI is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The STI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the STI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "andor885_device.h"


ANDOR885_Device::ANDOR885_Device(
		ORBManager* orb_manager, 
		std::string DeviceName, 
		std::string Address,
		unsigned short ModuleNumber) :
ANDOR885_Camera(),
STI_Device(orb_manager, DeviceName, Address, ModuleNumber)
{
	digitalChannel = 0;
	slowAnalogChannel = 0;
	minimumAbsoluteStartTime = 1000000; //1 ms buffer before any pictures can be taken

	cameraTriggerDevice = SlowAnalogBoard;
}

ANDOR885_Device::~ANDOR885_Device()
{
}

bool ANDOR885_Device::deviceMain(int argc, char **argv)
{
	bool error = false;
	int message;

	ofstream cerrLog;
	char tempChar[MAX_PATH];
	std::string tempString;

	/*tempString = logPath + "cerr.log";
	strcpy(tempChar,tempString.c_str());
	
	cerrLog.open(tempChar);

	streambuf* cerrBuffer = cerr.rdbuf(); // save cerr's output buffer

	cerr.rdbuf (cerrLog.rdbuf()); // redirect output into the file	*/

	std::cout << "Press any key to cleanly shutdown camera and program... ";
	std::cin >> message;
	
	/*cerr.rdbuf (cerrBuffer); // restore old output buffer
	cerrLog.close();*/

	error = deviceExit();

	std::cout << "Camera off" << std::endl;

	return error;
}


void ANDOR885_Device::defineAttributes()
{

	//Attributes
	addAttribute("*Camera status", "On", "On, Off");
	addAttribute(shutterMode_t.name, shutterMode_t.initial, shutterMode_t.makeAttributeString()); // Shutter control
//	addAttribute("Shutter open time (ms)", openTime); //time it takes shutter to open
//	addAttribute("Shutter close time (ms)", closeTime); //time it takes shutter to close
	addAttribute("*Folder Path for saved files", getFilePath());
	addAttribute("*Cooler setpoint", getCoolerSetpt());
	addAttribute("*Cooler status", "On", "On, Off");
	if (!preAmpGain_t.choices.empty())
		addAttribute(preAmpGain_t.name, preAmpGain_t.initial, preAmpGain_t.makeAttributeString()); //PreAmp gain
	if (!verticalShiftSpeed_t.choices.empty())
		addAttribute(verticalShiftSpeed_t.name, verticalShiftSpeed_t.initial, verticalShiftSpeed_t.makeAttributeString()); // Vertical shift speed of pixels
	if (!verticalClockVoltage_t.choices.empty())
		addAttribute(verticalClockVoltage_t.name, verticalClockVoltage_t.initial, verticalClockVoltage_t.makeAttributeString()); // Vertical clock voltage
	if (!horizontalShiftSpeed_t.choices.empty())
		addAttribute(horizontalShiftSpeed_t.name, horizontalShiftSpeed_t.initial, horizontalShiftSpeed_t.makeAttributeString()); // Horizontal shift speed of pixels
	addAttribute("Trigger Select", "Slow Analog Out", "Slow Analog Out, Digital Out");

	try {
		addAttribute("Camera temperature", getCameraTemp());
	}
	catch (ANDOR885_Exception &e) {
		std::cerr << e.printMessage() << std::endl;
		addAttribute("Camera temperature", -999);
	}

	addAttribute("Image Rotation (degrees)","0", "0, 270");
	addAttribute(triggerMode_t.name, triggerMode_t.initial, triggerMode_t.makeAttributeString()); //trigger mode
	addAttribute("Throwaway image time (ms)",minimumAbsoluteStartTime/1000000);
	if(getEMCCDGain() != NOT_AVAILABLE)
	{
		addAttribute("EMCCD Gain", getEMCCDGain());
	}

	if (debugging) {
		addAttribute(acquisitionMode_t.name, acquisitionMode_t.initial, acquisitionMode_t.makeAttributeString());
//	addAttribute("Read mode", "Image","Image, Multi-track, Random-track, Single-track"); //readout mode of data
		addAttribute(readMode_t.name, readMode_t.initial, readMode_t.makeAttributeString()); //readout mode of data
	}

}

void ANDOR885_Device::refreshAttributes()
{
	bool success;
	std::string tempString;

	// All attributes are stored in c++, none are on the fpga
	//Attributes not set in serial commands

	success = !AbortIfAcquiring();
	if (!success)
	{
		std::cerr << "Device: refreshAttributes: Error aborting acquisition" << std::endl;
	}

	setAttribute("*Camera status", (getCameraStat() == ANDOR_ON) ? "On" : "Off");
	setAttribute(shutterMode_t.name, shutterMode_t.choices.find(getShutterMode())->second);	
//	setAttribute("Shutter open time (ms)", getOpenTime()); 
//	setAttribute("Shutter close time (ms)", getCloseTime()); 
	setAttribute("*Folder Path for saved files", getFilePath());
	setAttribute("*Cooler setpoint", getCoolerSetpt());
	setAttribute("*Cooler status", (getCoolerStat() == ANDOR_ON) ? "On" : "Off");
	if (!preAmpGain_t.choices.empty())
		setAttribute(preAmpGain_t.name,preAmpGain_t.choices.find(getPreAmpGain())->second);
	if (!verticalShiftSpeed_t.choices.empty())
	setAttribute(verticalShiftSpeed_t.name,verticalShiftSpeed_t.choices.find(getVerticalShiftSpeed())->second);
	if (!verticalClockVoltage_t.choices.empty())
	setAttribute(verticalClockVoltage_t.name,verticalClockVoltage_t.choices.find(getVerticalClockVoltage())->second);
	if (!horizontalShiftSpeed_t.choices.empty())
		setAttribute(horizontalShiftSpeed_t.name,horizontalShiftSpeed_t.choices.find(getHorizontalShiftSpeed())->second);
	setAttribute("Trigger Select", (cameraTriggerDevice == SlowAnalogBoard) ? "Slow Analog Out" : "Digital Out");

	try {
		setAttribute("Camera temperature", getCameraTemp());
	}
	catch (ANDOR885_Exception& e) {
		std::cerr << e.printMessage() << std::endl;
		setAttribute("Camera temperature", -999);
	}

	tempString = valueToString(getRotationAngle());
	setAttribute("Image Rotation (degrees)", tempString);
	setAttribute(triggerMode_t.name, triggerMode_t.choices.find(getTriggerMode())->second); //trigger mode?
	setAttribute("Exposure time (s)", getExposureTime());
	setAttribute("Throwaway image time (ms)",minimumAbsoluteStartTime/1000000);
	if(getEMCCDGain() != NOT_AVAILABLE)
	{
		setAttribute("EMCCD Gain", getEMCCDGain());
	}

	if (debugging) {
		setAttribute(acquisitionMode_t.name, acquisitionMode_t.choices.find(getAcquisitionMode())->second);	
		setAttribute(readMode_t.name, readMode_t.choices.find(getReadMode())->second);
	}


	//restart acquisition
	success = startAcquisition();

	if (!success) {
		std::cerr << "Device: refreshAttributes: error starting acquisition" << std::endl;
	}
}

/*void ANDOR885_Device::takeThrowawayImage()
{
	MixedValue inVector;
	MixedData outString;
	//Take the saturated picture
	takeSaturatedPic = true;
	
	inVector.addValue((double) getExposureTime()*1000000000);//convert to ns
	inVector.addValue("");		// no description required
	inVector.addValue("");		// no filename required

	//take a saturated pic
	Sleep(500);
	read(0, inVector, outString);
	Sleep(500);

	takeSaturatedPic = false;
}*/

bool ANDOR885_Device::updateAttribute(std::string key, std::string value)
{

	double tempDouble;
	int tempInt;
//	char tempChar[MAX_PATH];
	std::string tempString;

	bool successDouble = stringToValue(value, tempDouble);
	bool successInt = stringToValue(value, tempInt);

	bool success = false;

	success = !AbortIfAcquiring();


	// When the camera is acquiring, the user is prohibited from changing the attributes,
	// except to turn off the acquisition.
	try {
		if (!success) {
			std::cerr << "Device: updateAttribute: Error aborting acquisition" << std::endl;
		} else {

			if(key.compare("*Camera status") == 0) {
				success = true;

				if (value.compare("On") == 0){
					setCameraStat(ANDOR_ON);
				} 
				else if (value.compare("Off") == 0) {
					setCameraStat(ANDOR_OFF);
				} 
				else {
					throw ANDOR885_Exception("Device: Unrecognized Camera Status requested");
				}
			} 

			else if(key.compare("Shutter mode") == 0) {
				success = true;
				setShutterMode(shutterMode_t.inverseFind(value));
			}		

	/*		else if(key.compare("Shutter open time (ms)") == 0 && successInt)
			{
				success = true;
				setOpenTime(tempInt);
			}

			else if(key.compare("Shutter close time (ms)") == 0 && successInt)
			{
				success = true;
				setCloseTime(tempInt);
			}
	*/
			else if(key.compare("*Folder Path for saved files") == 0) {
				success = true;
				setFilePath(value);
			}

			else if (key.compare("*Cooler setpoint") == 0 && successInt) {
				success = true;
				setCoolerSetpt(tempInt);
			}

			else if (key.compare("*Cooler status") == 0) {
				success = true;
				if (value.compare("On") == 0){
					setCoolerStat(ANDOR_ON);
				} 

				else if (value.compare("Off") == 0) {
					setCoolerStat(ANDOR_OFF);
				}
				else {
					throw ANDOR885_Exception("Device: Unerecognized Cooler Status requested");
				}
			}

			else if (key.compare(preAmpGain_t.name)==0){
				success = true;
				setPreAmpGain(preAmpGain_t.inverseFind(value));
			}

			else if(key.compare(verticalShiftSpeed_t.name) == 0) {
				success = true;
				setVerticalShiftSpeed(verticalShiftSpeed_t.inverseFind(value));
			}

			else if(key.compare(verticalClockVoltage_t.name) == 0) {
				success = true;
				setVerticalClockVoltage(verticalClockVoltage_t.inverseFind(value));
			}

			else if(key.compare(horizontalShiftSpeed_t.name) == 0) {
				success = true;
				setHorizontalShiftSpeed(horizontalShiftSpeed_t.inverseFind(value));
			}

			else if (key.compare("Trigger Select") == 0) {
				success = true;
				/*if (value.compare("Digital Out") == 0){
					cameraTriggerDevice = DigitalBoard;
					partnerDevice("Slow Analog Out").disablePartnerEvents();
				} 

				else if (value.compare("Slow Analog Out") == 0) {
					cameraTriggerDevice = SlowAnalogBoard;
					partnerDevice("Digital Out").disablePartnerEvents();
				}
				else {
					success = false;
				}*/
			}

			else if (key.compare("Camera temperature") == 0){
				success = true;
				//This doesn't really get actively set to a value
			} 

			else if (key.compare("Image Rotation (degrees)") == 0 && successDouble)
			{
				success = true;
				setRotationAngle(tempDouble);
			}
			else if (key.compare(triggerMode_t.name) == 0) {
						success = true;
						setTriggerMode(triggerMode_t.inverseFind(value));
			}
			else if(key.compare("Exposure time (s)") == 0 && successDouble) {
						success = true;
						setExposureTime((float) tempDouble);
			}
			else if(key.compare("Throwaway image time (ms)") == 0 && successDouble) {
						success = true;
						minimumAbsoluteStartTime = tempDouble*1000000;
			}
			else if(key.compare("EMCCD Gain") == 0 && successInt) {
						success = true;
						setEMCCDGain(tempInt);
			}

			else {
				if (debugging) {
					if(key.compare(acquisitionMode_t.name) == 0) {
						success = true;
						setAcquisitionMode(acquisitionMode_t.inverseFind(value));
					}

					else if(key.compare(readMode_t.name) == 0) {
						success = true;
						setReadMode(readMode_t.inverseFind(value));
					}

					else {
						throw ANDOR885_Exception("Device: Unrecognized attribute");
					}
				} 
				else {
					throw ANDOR885_Exception("Device: Unrecognized attribute");
				}

			}
		}
	} catch (ANDOR885_Exception& e){
		std::cerr << e.printMessage() << std::endl;
		success = false;
	}
	
	return success;
}



void ANDOR885_Device::defineChannels()
{
	//this->add
	addInputChannel(0, DataString, ValueVector);
}
bool ANDOR885_Device::readChannel(unsigned short channel, const MixedValue& valueIn, MixedData& dataOut) 
{
	return readChannelDefault(channel, valueIn, dataOut, minimumAbsoluteStartTime + 10);
}
std::string ANDOR885_Device::execute(int argc, char **argv)
{
	return "";
}

void ANDOR885_Device::definePartnerDevices()
{
	addPartnerDevice("Digital Out", "ep-timing1.stanford.edu", 2, "Digital Out");
//	partnerDevice("Digital Out").enablePartnerEvents();
	
	addPartnerDevice("Slow Analog Out", "ep-timing1.stanford.edu", 4, "Slow Analog Out");
	partnerDevice("Slow Analog Out").enablePartnerEvents();
}


void ANDOR885_Device::parseDeviceEvents(const RawEventMap &eventsIn, 
		SynchronousEventVector& eventsOut) throw(std::exception)
{ 
	double digitalMinAbsStartTime = 10000;
	double startTimeBuffer = minimumAbsoluteStartTime;
	double prepEventTime; //time when the FPGA should trigger in order to have the output ready in time
	double eventTime;
	double previousTime; //time when the previous event occurred
	double previousExposureTime;
	double ns = 1000000000; // Conversion factor from seconds to ns
	double ms2ns = 1000000; // Conversion factor from ms to ns
	double endTimeBuffer = 1000; // 1 us in ns - buffer after last exposure ends before the clean-up event starts.

	int allowedExposureTimeError = 10; //ns

	RawEventMap::const_iterator events;
	RawEventMap::const_iterator previousEvents;

	Andor885Event* andor885Event;
	Andor885Event* andor885InitEvent;
	Andor885Event* andor885EndEvent;

	//For saving pictures, I need the metadata encoded in the event
	EventMetadatum eventMetadatum;

	//For the image crop vector, I need a vector of ints and a string for error messages
	std::vector <int> cropVector;
	std::string cropVectorMessage;

	//For Exposure Mode (v. External Exposure) pictures, check that all exposures are the same as the first one
	double parsedExposureTime = -1;

	//Minimum absolute start time depends on opening time of camera shutter
	if (getShutterMode() != SHUTTERMODE_OPEN)
	{
		startTimeBuffer += getOpenTime() * ms2ns;
	}

	startAcquisition(); // start acquisition if not already acquiring; this sets takeThrowawayImage

	for(events = eventsIn.begin(); events != eventsIn.end(); events++)
	{	

		// Create initialization event
		if (events == eventsIn.begin())
		{
			
			//Make sure trigger line is initialized
			if(cameraTriggerDevice == DigitalBoard)
			{
				partnerDevice("Digital Out").event(digitalMinAbsStartTime, digitalChannel, 0, events->second.at(0));
			}
			if(cameraTriggerDevice == SlowAnalogBoard)
			{
				partnerDevice("Slow Analog Out").event(digitalMinAbsStartTime, slowAnalogChannel, 0, events->second.at(0));
			}

			//Small hold-off to make sure initialization even occurs after digital line is low
			andor885InitEvent = new Andor885Event(minimumAbsoluteStartTime, this);
			andor885InitEvent->eventMetadatum.assign(INIT_EVENT);
		}

		eventTime = events->first;

		if (events->second.at(0).getValueType() == MixedValue::Vector)
		{
			//Doesn't this create a new sizeOfTuple at each iteration?
			unsigned sizeOfTuple = events->second.at(0).value().getVector().size();

			const std::vector <MixedValue>& eVector = events->second.at(0).value().getVector();

			//Check that each type in tuple is correct. The first two switch statements
			// are deliberately un-break'd.
			switch(sizeOfTuple)
			{
			case 4:
				if(eVector.at(3).getType() != MixedValue::Vector)
				{
					delete andor885InitEvent;
					throw EventParsingException(events->second.at(0),
						"Andor camera crop vector must be a vector");
				}
				

				//Check to see if crop vector has the right form
				cropVectorMessage = testCropVector(eVector.at(3).getVector(), cropVector);
				if (cropVector.empty())
				{
					delete andor885InitEvent;
					throw EventParsingException(events->second.at(0), cropVectorMessage);
				}
				else if (cropVectorMessage.compare("") != 0)
				{
					//requested crop probably got clipped. Print the warning(s)
					std::cout << cropVectorMessage << std::endl;
				}

			case 3:
				if(eVector.at(2).getType() != MixedValue::String)
				{
					delete andor885InitEvent;
					throw EventParsingException(events->second.at(0),
						"Andor camera image description must be a string");
				}
			case 2:
				if(eVector.at(1).getType() != MixedValue::String)
				{
					delete andor885InitEvent;
					throw EventParsingException(events->second.at(0),
						"Andor camera filename must be a string");
				}
			case 1:
				if(eVector.at(0).getType() != MixedValue::Double)
				{
					delete andor885InitEvent;
					throw EventParsingException(events->second.at(0),
						"Andor camera exposure time must be a double");
				}
				break;

			default:
				delete andor885InitEvent;
				throw EventParsingException(events->second.at(0),
					"Andor camera commands must be a tuple in the form (double exposureTime, string description, string filename, vector cropVector). The crop vector can be of the form (int pixelULX, int pixelULY, int fullWidthX, int fullWidthY) or (int centerPixelX, int centerPixelY, int halfWidth)." );
				break;
			}

			//Check that the exposure time is not too short
			if (eVector.at(0).getDouble() < 10000)
			{
				delete andor885InitEvent;
				throw EventParsingException(events->second.at(0), "Andor camera requires an exposure time of greater than 10 us");
			}

			


			if(events == eventsIn.begin())
			{
				parsedExposureTime = eVector.at(0).getDouble();

				//takeThrowawayImage gets set to false ONLY if the event has been played; an event will play even if the file has been parsed twice.
				if (takeThrowawayImage)
				{
					EventMetadatum tempEM;
					tempEM.assign(parsedExposureTime, "throwaway");

					if(cameraTriggerDevice == DigitalBoard)
					{
						sendDigitalLineExposureEvents(andor885InitEvent->getTime(), events->second.at(0), parsedExposureTime);
					}
					else if(cameraTriggerDevice == SlowAnalogBoard)
					{
						sendSlowAnalogLineExposureEvents(andor885InitEvent->getTime(), events->second.at(0), parsedExposureTime);
					}
					//push back metadata so that camera knows to take a picture
					andor885InitEvent->eventMetadata.push_back(tempEM);
				}
			}

			//Check exposure time is consistent
			if (getTriggerMode() != TRIGGERMODE_EXTERNAL_EXPOSURE)
			{
				
				//Check that the exposure time 
				if (events == eventsIn.begin())
				{
					int cameraExposureTime = (int)(getExposureTime()*ns);

					//Uses parsedExposureTime defined in the if statement above
					//try to set the camera exposure if it's not already correct to within 10ns
					if (abs((int) parsedExposureTime - cameraExposureTime) > allowedExposureTimeError) {
						try {
							bool success = !AbortIfAcquiring();
							if (success)
								setExposureTime((float) parsedExposureTime / ns);
							success &= startAcquisition();

							if (!success) {
								delete andor885InitEvent;
								throw EventParsingException(events->second.at(0),
									"Could not start or stop acquisition during parsing.");
							}
							
						}
						catch (ANDOR885_Exception& e){
							std::cerr << e.printMessage() << std::endl;
							delete andor885InitEvent;
							throw EventParsingException(events->second.at(0),
								"Error from Andor: " + e.printMessage() );
						}
					}

					/*cameraExposureTime = (int) (getExposureTime()*(float)ns);

					if (abs(parsedExposureTime - cameraExposureTime) > 10){
						delete andor885InitEvent;
						throw EventParsingException(events->second.at(0), 
								"Camera can provide an exposure time of " + valueToString(cameraExposureTime) +
								" not " + valueToString(parsedExposureTime));
					}*/
				}
				else if (abs((int) parsedExposureTime -  (int) eVector.at(0).getDouble()) > allowedExposureTimeError)
				{
					delete andor885InitEvent;
					throw EventParsingException(events->second.at(0),
						"Camera requires all exposure times to be identical to the first: " +
						valueToString(parsedExposureTime)
						);
				}
			}

			


			switch(sizeOfTuple)
			{
			case 4:
				andor885Event = new Andor885Event(eventTime, this);
				andor885Event->eventMetadatum.assign(eVector.at(0).getDouble(), eVector.at(1).getString(), eVector.at(2).getString(), cropVector);
				break;
			case 3:
				andor885Event = new Andor885Event(eventTime, this);
				andor885Event->eventMetadatum.assign(eVector.at(0).getDouble(), eVector.at(1).getString(), eVector.at(2).getString());
				break;
			case 2:
				andor885Event = new Andor885Event(eventTime, this);
				andor885Event->eventMetadatum.assign(eVector.at(0).getDouble(), eVector.at(1).getString());
				break;
			case 1:
				andor885Event = new Andor885Event(eventTime, this);
				andor885Event->eventMetadatum.assign(eVector.at(0).getDouble());
				break;
				
			default:
				delete andor885InitEvent;
				throw EventParsingException(events->second.at(0), "Never should get here, but Andor camera commands must be a tuple in the form (double exposureTime, string description, string filename)");
				break;
			}

			//Set the base filename
			andor885Event->filenameBase = andor885Event->eventMetadatum.filename;

			//Push back the metadatum onto the init_event's vector
			andor885InitEvent->eventMetadata.push_back(andor885Event->eventMetadatum);
			
		// Check that the camera can keep up with the events
			if(events != eventsIn.begin())
			{
				previousEvents = --events;
				events++;
				previousTime = previousEvents->first;
				previousExposureTime = previousEvents->second.at(0).value().getVector().at(0).getDouble();
				// The kinetic time gets set whenever the exposure time is changed.
				// it depends on the vertical and horizontal shift speeds, and adds on the exposure time
				prepEventTime = eventTime - (getKineticTime() * ns + previousExposureTime) * events->second.size();
			}
			else if(takeThrowawayImage)
			{
				previousEvents = events; //Needed in case of an error. See if clause below
				previousTime = andor885InitEvent->getTime();
				previousExposureTime = andor885InitEvent->eventMetadata.at(0).exposureTime; //parsedExposureTime
				// The kinetic time gets set whenever the exposure time is changed.
				// it depends on the vertical and horizontal shift speeds, and adds on the exposure time
				prepEventTime = eventTime - (getKineticTime() * ns + previousExposureTime) * events->second.size();
			}
			else
			{
				previousEvents = events;
				previousTime = startTimeBuffer;
				prepEventTime = eventTime;
			}

			if( prepEventTime < previousTime  && events != eventsIn.begin())
			{
				delete andor885InitEvent;
				delete andor885Event;
				throw EventConflictException(previousEvents->second.at(0), 
					events->second.at(0), 
					"Given vertical and horizontal shift speeds, the camera cannot take pictures faster than" + valueToString(getKineticTime()) + " s, plus the exposure time" );
			}
			else if (prepEventTime < previousTime && !takeThrowawayImage)
			{
				delete andor885InitEvent;
				delete andor885Event;
				throw EventConflictException(previousEvents->second.at(0), 
					events->second.at(0), 
					"The camera must have a " + valueToString(startTimeBuffer/ns) + " s buffer before the first image." );
			}
			else if (prepEventTime < previousTime && takeThrowawayImage)
			{
				delete andor885InitEvent;
				delete andor885Event;
				throw EventConflictException(previousEvents->second.at(0), 
					events->second.at(0), 
					"Throwaway image required: the camera must have at least " + valueToString((previousTime-prepEventTime)/ns) + " s more time between the throwaway image and the first recorded image." );
			}

			if(cameraTriggerDevice == DigitalBoard)
			{
				sendDigitalLineExposureEvents(eventTime, events->second.at(0), andor885Event->eventMetadatum.exposureTime);
			}
			else if(cameraTriggerDevice == SlowAnalogBoard)
			{
				sendSlowAnalogLineExposureEvents(eventTime, events->second.at(0), andor885Event->eventMetadatum.exposureTime);
			}

			eventsOut.push_back( andor885Event );

			//Add measurement
			eventsOut.back().addMeasurement( events->second.at(0) );

			previousTime = eventTime;
			}
		else
		{
			delete andor885InitEvent;
			std::cerr << "The Andor camera does not support that data type" << std::endl;
			throw EventParsingException(events->second.at(0),
						"The Andor camera does not support that data type.");
		}
		
	}

	//create end event
	events--;
	if (!eventsIn.empty()) {
		andor885EndEvent = new Andor885Event(events->first + events->second.at(0).value().getVector().at(0).getDouble() + endTimeBuffer, this);
		andor885EndEvent->eventMetadatum.assign(END_EVENT);
		eventsOut.push_back( andor885EndEvent );

		//insert initalization event
		eventsOut.insert(eventsOut.begin(), andor885InitEvent);
	}

}

std::string ANDOR885_Device::testCropVector(const MixedValueVector& cropVectorIn, std::vector <int>& cropVectorOut)
{
	int i;
	std::string tempString = "";
	int imageWidth = getImageWidth();
	int imageHeight = getImageHeight();
	int tempCenterX = 1;
	int tempCenterY = 1;
	int tempHalfWidth = 99;

	int validStartPixelX = 1;
	int validStartPixelY = 1;

	cropVectorOut.clear();

	if (cropVectorIn.size() != 4 && cropVectorIn.size() !=  3)
	{
		return ("Andor camera crop vector must have a length of 3 or 4: (int pixelULX, int pixelULY, int fullWidthX, int fullWidthY) or (int centerPixelX, int centerPixelY, int halfWidth)");
	}
	for (i = 0; i < (signed) cropVectorIn.size(); i++)
	{
		if (cropVectorIn.at(i).getType() != MixedValue::Double)
		{
			cropVectorOut.clear();
			return ("Andor camera crop vector element must be an Int.");
		}
		if (cropVectorIn.at(i).getDouble() < 1)
		{
			cropVectorOut.clear();
			return ("Andor camera crop vector elements must be > 1.");
		}
		cropVectorOut.push_back((int) cropVectorIn.at(i).getDouble());
	}

	// convert center-referenced vector to a corner vector
	if (cropVectorIn.size() == 3) {
		tempCenterX = cropVectorOut.at(0);
		tempCenterY = cropVectorOut.at(1);
		tempHalfWidth = cropVectorOut.at(2);
		
		cropVectorOut.clear();

		cropVectorOut.push_back(tempCenterX - tempHalfWidth);
		cropVectorOut.push_back(tempCenterY - tempHalfWidth);
		cropVectorOut.push_back(tempHalfWidth * 2 + 1);
		cropVectorOut.push_back(tempHalfWidth * 2 + 1);
	}

	//check input; clip if necessary
	i = cropVectorOut.at(0);
	if(i < validStartPixelX)
	{
		tempString += "Crop vector starts at an invalid x-pixel: " + 
			STI::Utils::valueToString(i) + " < " + 
			STI::Utils::valueToString(validStartPixelX) + ". Clipping crop vector...  ";
		
		cropVectorOut.at(0) = validStartPixelX;
	}

	i = cropVectorOut.at(1);
	if(i < validStartPixelY)
	{
		tempString += "Crop vector starts at an invalid y-pixel: " + 
			STI::Utils::valueToString(i) + " < " + 
			STI::Utils::valueToString(validStartPixelY) + ". Clipping crop vector...  ";
		
		cropVectorOut.at(1) = validStartPixelY;
	}

	i = cropVectorOut.at(0) + cropVectorOut.at(2) - 1;
	if (i > imageWidth) {
		tempString += "Crop vector exceeds image width: " + 
				STI::Utils::valueToString(i) + " > " + 
				STI::Utils::valueToString(imageWidth) + ". Clipping crop vector...  ";

		cropVectorOut.at(2) -= i - imageWidth;
	}
	

	i = cropVectorOut.at(1) + cropVectorOut.at(3) - 1;
	if (i > imageHeight) 
	{
		tempString += "Crop vector exceeds image height: " + 
			STI::Utils::valueToString(i) + " > " + 
			STI::Utils::valueToString(imageHeight) + ". Clipping crop vector...  ";

		cropVectorOut.at(3) -= i - imageHeight;
	}

	// convert user-friendly pixel definitions to computer-friendly ones
	cropVectorOut.at(0) -= 1; // now referenced from 0
	cropVectorOut.at(1) -= 1; // now referenced from 0
	cropVectorOut.at(2) -= 1; // now adding to 0 yeilds location of last pixel
	cropVectorOut.at(3) -= 1; // now adding to 1 yeilds location of last pixel

	return (tempString);
}
void ANDOR885_Device::sendDigitalLineExposureEvents(double eventTime, const RawEvent& evt, double exposureTime)
{
	partnerDevice("Digital Out").event(eventTime, 
					digitalChannel, 1, evt);
	partnerDevice("Digital Out").event(eventTime + exposureTime, 
					digitalChannel, 0, evt);
}
void ANDOR885_Device::sendSlowAnalogLineExposureEvents(double eventTime, const RawEvent& evt, double exposureTime)
{
	partnerDevice("Slow Analog Out").event(eventTime, 
					slowAnalogChannel, 5, evt);	//5 volts
	partnerDevice("Slow Analog Out").event(eventTime + exposureTime, 
					slowAnalogChannel, 0, evt);
}

void ANDOR885_Device::stopEventPlayback()
{
	stopEventMutex->lock();
		stopEvent = true;
		waitForCleanupEventMutex->lock();
			cleanupEvent = true;
		waitForCleanupEventMutex->unlock();
		AbortIfAcquiring();
	stopEventMutex->unlock();

	numAcquiredMutex->lock();
		numAcquiredCondition->broadcast();
	numAcquiredMutex->unlock();
}


void ANDOR885_Device::Andor885Event::playEvent()
{
	std::string fn;
	int decrement = 1;

	
	std::cout << "Playing event " << getEventNumber() << std::endl;

	//For initialization event, setup camera for acquisition
	if (eventMetadatum.exposureTime == INIT_EVENT)
	{
		std::cout << "Starting setup" << std::endl;
		ANDORdevice_->setupEventAcquisition(&eventMetadata);
		std::cout << "Finished setup" << std::endl;
		std::cout<< eventMetadata.size() << std::endl;
	}
	else if (eventMetadatum.exposureTime == END_EVENT)
	{
		std::cout << "Starting cleanup" << std::endl;
		ANDORdevice_->cleanupEventAcquisition();
		std::cout << "Finished cleanup" << std::endl;
	}
	else
	{
		//Add timestamp to device and camera's copy of the filename
		ANDORdevice_->stopEventMutex->lock();
		if (!(ANDORdevice_->stopEvent) || 
			((ANDORdevice_->stopEvent) && (ANDORdevice_->numAcquired >= (signed) getEventNumber()))) {
			eventMetadatum.filename = ANDORdevice_->timeStampFilename(filenameBase);

			if (ANDORdevice_->eventMetadata->at(0).description.compare("throwaway")==0)
				decrement = 0;
			ANDORdevice_->eventMetadata->at((signed) getEventNumber() - decrement).filename = 
				eventMetadatum.filename;
			eventMetadatum.filename = eventMetadatum.filename + ANDORdevice_->extension;

			std::cout << eventMetadatum.filename << std::endl;

		} else {
			eventMetadatum.filename = ANDORdevice_->timeStampFilename(filenameBase + " should not have been saved");
			
			if (ANDORdevice_->eventMetadata->at(0).description.compare("throwaway")==0)
				decrement = 0;
			ANDORdevice_->eventMetadata->at((signed) getEventNumber() - decrement).filename = 
				eventMetadatum.filename;
			eventMetadatum.filename = eventMetadatum.filename + ANDORdevice_->extension;
		}
		ANDORdevice_->stopEventMutex->unlock();
	}

}

void ANDOR885_Device::Andor885Event::waitBeforeCollectData()
{
	std::cout << "Waiting for event " << getEventNumber() << std::endl;
	if (eventMeasurements.size() == 1)
	{
		ANDORdevice_->numAcquiredMutex->lock();
			while (ANDORdevice_->numAcquired < (signed) getEventNumber() && !(ANDORdevice_->stopEvent))
			{
				ANDORdevice_->numAcquiredCondition->wait();
			}
		ANDORdevice_->numAcquiredMutex->unlock();
	}
	else if (eventMeasurements.size() > 1)
	{
		std::cout << "There should be only one measurement per event: event " << getEventNumber() << std::endl;
	}
	std::cout << "Done waiting for event " << getEventNumber() << std::endl;
}

void ANDOR885_Device::Andor885Event::collectMeasurementData()
{
	if (eventMeasurements.size() == 1 && !(ANDORdevice_->stopEvent))
	{
		eventMeasurements.at(0)->setData(eventMetadatum.filename);
	}
	else if (eventMetadatum.exposureTime < 0)
	{
		//do nothing; Not a measurement event;
	}
	else 
	{
		if(!(ANDORdevice_->stopEvent)) {
		std::cerr << "Error in collectMeasurementData: expect only one measurement. Received " 
			<< eventMeasurements.size() << std::endl;
		}
		else {
			std::cerr << "Stopped collecting data" << std::endl;
		}
	}
}
