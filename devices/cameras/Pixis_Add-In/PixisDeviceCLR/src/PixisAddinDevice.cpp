
#include "PixisAddinDevice.h"


void PixisAddinDevice::defineChannels()
{
	addInputChannel(0, DataString, ValueVector);
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
	double eventTime;
	double holdoff = 100000000;		//100 ms
	RawEventMap::const_iterator events;

	for (events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if (events->second.at(0).getValueType() != MixedValue::Vector) {
			throw EventParsingException(events->second.at(0),
				"Invalid data type.  The camera expects a vector.");
		}

		//		auto eventTuple = events->second.at(0).value().getVector();

		eventTime = events->first - holdoff;

		auto pixisEvent = std::make_unique<PixisEvent>(eventTime, this, 0);

		pixisEvent->addMeasurement(events->second.at(0));		//register the measurement with the source RawEvent

		eventsOut.push_back(pixisEvent.release());
	}
}

void PixisAddinDevice::PixisEvent::stop()
{
	SynchronousEvent::stop();

	//	cameraDevice->camera.StopAcquisition();
}

void PixisAddinDevice::PixisEvent::loadEvent()
{
}


void PixisAddinDevice::PixisEvent::playEvent()
{
//	cameraDevice->aquireHandler("aquire!");
}

void PixisAddinDevice::PixisEvent::waitBeforeCollectData()
{
	cameraDevice->wait();
}

void PixisAddinDevice::wait()
{
	std::unique_lock<std::mutex> lock(aquire_mutex);

	waiting = true;

	while (waiting)
	{
		aquire_condition.wait(lock);
	}
}

void PixisAddinDevice::stopWaiting()
{
	std::unique_lock<std::mutex> lock(aquire_mutex);
	waiting = false;
	aquire_condition.notify_one();
}

void PixisAddinDevice::PixisEvent::collectMeasurementData()
{
	if (eventMeasurements.size() == 1)
	{
		eventMeasurements.at(0)->setData("imageFileName");
		//		eventMeasurements.at(0)->setData("Error getting image.");
	}
}

