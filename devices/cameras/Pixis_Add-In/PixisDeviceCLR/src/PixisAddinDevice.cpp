
#include "PixisAddinDevice.h"


void PixisAddinDevice::defineChannels()
{
	addInputChannel(0, DataVector, ValueVector);
}


void PixisAddinDevice::definePartnerDevices()
{
	//	addPartnerDevice("ext_trigger", "ep-timing1.stanford.edu", 2, "Digital Out");
}

std::string PixisAddinDevice::getDeviceHelp()
{
	return string("Information for PIXIS Addin device.\n\n");
}


void PixisAddinDevice::parseDeviceEvents(const RawEventMap &eventsIn,
	SynchronousEventVector& eventsOut) throw(std::exception)
{
//	return;


	double eventTime;
	double lastEventTime = 0;
	double holdoff = 0;		//100 ms
	RawEventMap::const_iterator events;

	int imageIndex = 0;
//	imagesDone.clear();

	for (events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (events->second.at(0).getValueType() != MixedValue::Vector) {
			throw EventParsingException(events->second.at(0),
				"Invalid data type.  The camera expects a vector.");
		}

	//		auto eventTuple = events->second.at(0).value().getVector();

		eventTime = events->first - holdoff;

		//The Aquire button on Lightfield is pressed immediately (at time 0 for the first event)
		auto pixisEvent = std::make_unique<PixisEvent>(lastEventTime, this, imageIndex);

		//add partner event (trigger) at eventTime 

		imageIndex++;
		lastEventTime = eventTime;	//make the next event happen immediately after this one was supposed to happen
//		imagesDone.push_back(false);

		pixisEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent

		eventsOut.push_back(pixisEvent.release());
	}
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

void PixisAddinDevice::PixisEvent::reset()
{
	SynchronousEvent::reset();

//	return;

//	setSavedImageFilename("");
//	setSavedSPEFilename("");

	{
		//Every event does this; unnecessary, but in practice there aren't very many events in one sequence.
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

	//stop waiting on all events

	//	cameraDevice->camera.StopAcquisition();
}

void PixisAddinDevice::PixisEvent::loadEvent()
{
	cameraDevice->lightfield.incrementImageCount();
//	cameraDevice->lightfield.aquire(123);
//	cameraDevice->lightfield.incrementImageCount();
//	cameraDevice->lightfield.incrementImageCount();
}


void PixisAddinDevice::PixisEvent::playEvent()
{
//	cameraDevice->lightfield.incrementImageCount();

//	cameraDevice->lightfield.aquire(123);

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
//	return;

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

//void PixisAddinDevice::waitForImage(int index)
void PixisAddinDevice::aquireImage(int index)
{
//	lightfield.aquire(index);
//	return;


	//This waits until the previous event is done playing.
//	while (currentImageIndex < index && deviceStatusIs(Playing))
//	{
//		aquire_condition.wait(lock);
//	}

//	waiting = true;

	std::unique_lock<std::mutex> lock(aquire_mutex);
	
	lightfield.aquire(index);

	//This waits until the current event is done (waits for callback from LightField)
	while (currentImageIndex <= index && deviceStatusIs(Playing))
	{
		aquire_condition.wait(lock);
	}

//	waiting = false;

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

