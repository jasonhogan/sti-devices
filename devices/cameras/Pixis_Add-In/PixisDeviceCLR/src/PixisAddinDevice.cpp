
#include "PixisAddinDevice.h"

//#include <iostream>
//#include <fstream> 
#include <sstream>
#include <ctime>

//#include <stdio.h>
//#include <stdlib.h>

PixisAddinDevice::PixisAddinDevice(ORBManager* orb_manager)
	: STI_Device_Adapter(orb_manager, "Pixis Test", "eppixis", 0)
{
	trigger.name = "";
	trigger.ip = "";
	trigger.module = 0;
	trigger.channel = 0;

	trigger.low = 0;
	trigger.high = 5;
	trigger.duration_ns = 10000;
	trigger.resetHoldoff_ns = 10000;

	baseDirectory = "";
}

PixisAddinDevice::PixisAddinDevice(ORBManager* orb_manager, const ConfigFile& configFile)
	: STI_Device_Adapter(orb_manager, configFile)
{
	currentImageIndex = 0;

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

void PixisAddinDevice::defineChannels()
{
	addInputChannel(0, DataVector, ValueVector);
}


void PixisAddinDevice::definePartnerDevices()
{
	addPartnerDevice("External Trigger", trigger.ip, trigger.module, trigger.name);
	partnerDevice("External Trigger").enablePartnerEvents();
}

std::string PixisAddinDevice::getDeviceHelp()
{
	return string("Information for PIXIS Addin device.\n\n");
}


void PixisAddinDevice::parseDeviceEvents(const RawEventMap &eventsIn,
	SynchronousEventVector& eventsOut) throw(std::exception)
{

	double eventTime;
	double lastEventTime = 0;
	double holdoff = 0;		//100 ms
	RawEventMap::const_iterator events;

	int imageIndex = 0;

	for (events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (events->second.at(0).getValueType() != MixedValue::Vector) {
			throw EventParsingException(events->second.at(0),
				"Invalid data type.  The camera expects a vector.");
		}

		eventTime = events->first - holdoff;

		//The Aquire button on Lightfield is pressed immediately (at time 0 for the first event)
		auto pixisEvent = std::make_unique<PixisEvent>(lastEventTime, this, imageIndex);

		if (lightfield.externalTriggerOn()) {
			//add partner events (trigger) at eventTime 
			partnerDevice("External Trigger").event(
				eventTime - trigger.resetHoldoff_ns, trigger.channel, trigger.low, events->second.at(0));	// Low	
			partnerDevice("External Trigger").event(eventTime, trigger.channel, trigger.high, events->second.at(0));	// High
			partnerDevice("External Trigger").event(
				eventTime + trigger.duration_ns, trigger.channel, trigger.low, events->second.at(0));	// Low
		}

		imageIndex++;
		lastEventTime = eventTime;	//make the next event happen immediately after this one was supposed to happen

		pixisEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent

		eventsOut.push_back(pixisEvent.release());
	}

}

//TODO: Have the server transmit this information...
std::string PixisAddinDevice::generateDataDestinationDirectory()
{
//	std::string baseDirectory = "C:\\Users\\Jason\\Code\\dev\\stidatatest\\";
	char dirSepChar = baseDirectory.back();

	std::stringstream path;

	std::time_t t = std::time(0);   // get time now
	struct tm * now = std::localtime(&t);

	path << baseDirectory << (now->tm_year + 1900) << dirSepChar
		<< (now->tm_mon + 1) << dirSepChar
		<< now->tm_mday << dirSepChar
		<< "data" 
		<< endl;

	return path.str();
}

std::string PixisAddinDevice::getFilename(int index, const std::map<int, std::string>& filenames)
{
	std::unique_lock<std::mutex> lock(filename_mutex);

	std::string name = ""; //default
	auto it = filenames.find(index);

	if (it != filenames.end()) {
		name = it->second;
	}

	return name;
}

void PixisAddinDevice::setSavedImageFilename(int index, const std::string& filename)
{
	std::unique_lock<std::mutex> lock(filename_mutex);
	imageFilenames.insert({ index, filename });
}

void PixisAddinDevice::setSavedSPEFilename(int index, const std::string& filename)
{
	std::unique_lock<std::mutex> lock(filename_mutex);
	speFilenames.insert({ index, filename });
}

void PixisAddinDevice::print(const std::string& message)
{
	lightfield.print(message);
}

void PixisAddinDevice::PixisEvent::reset()
{
	SynchronousEvent::reset();

	if (imageIndex == 0) {
		std::unique_lock<std::mutex> lock(cameraDevice->filename_mutex);
		cameraDevice->imageFilenames.clear();
		cameraDevice->speFilenames.clear();
	}
	
	{
		std::unique_lock<std::mutex> lock(collect_mutex);
		dataReady = false;
	}

	cameraDevice->resetImageIndex();
}
 
void PixisAddinDevice::PixisEvent::stop()
{
	SynchronousEvent::stop();

	cameraDevice->lightfield.stop();
}

void PixisAddinDevice::PixisEvent::setupEvent()
{
	if (imageIndex == 0) {
		std::string test = cameraDevice->generateDataDestinationDirectory();
		cameraDevice->lightfield.setSaveDir(test);
	}
}


void PixisAddinDevice::PixisEvent::loadEvent()
{
	cameraDevice->lightfield.incrementImageCount();
}

void PixisAddinDevice::PixisEvent::playEvent()
{
	cameraDevice->aquireImage(imageIndex);

	//When leaving aquireImage, the filenames have already been transferred, so data is ready.
	{
		std::unique_lock<std::mutex> lock(collect_mutex);
		dataReady = true;
	}

	collect_condition.notify_one();
}

void PixisAddinDevice::PixisEvent::waitBeforeCollectData()
{
	std::unique_lock<std::mutex> lock(collect_mutex);

	while (!dataReady && cameraDevice->deviceStatusIs(Playing))
	{
		collect_condition.wait(lock);
	}
}

void PixisAddinDevice::resetImageIndex() 
{
	std::unique_lock<std::mutex> lock(aquire_mutex);

	currentImageIndex = 0;

	lightfield.clearImageCount();
}


void PixisAddinDevice::aquireImage(int index)
{
	std::unique_lock<std::mutex> lock(aquire_mutex);
	
	lightfield.aquire(index);

	//This waits until the current event is done (waits for callback from LightField)
	while (currentImageIndex <= index && deviceStatusIs(Playing))
	{
		aquire_condition.wait(lock);
	}
}

void PixisAddinDevice::stopWaiting(int index)
{
	{
		std::unique_lock<std::mutex> lock(aquire_mutex);
		currentImageIndex = index + 1;
	}
	aquire_condition.notify_all();
}

void PixisAddinDevice::PixisEvent::collectMeasurementData()
{
	if (eventMeasurements.size() == 1)
	{
		MixedData data;

		data.addValue(cameraDevice->getFilename(imageIndex, cameraDevice->imageFilenames));
		data.addValue(cameraDevice->getFilename(imageIndex, cameraDevice->speFilenames));
		
		eventMeasurements.at(0)->setData(data);
	}
}

