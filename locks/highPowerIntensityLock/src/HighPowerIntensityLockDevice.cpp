/*! \file HighPowerIntensityLock.cpp
 *  \author Tim Kovachy
 *  \author Jason Hogan
 *  \brief Source-file for the class HighPowerIntensityLock
 *  \section license License
 *
 *  Copyright (C) 2013 Jason Hogan <hogan@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
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

#include "HighPowerIntensityLockDevice.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using STI::Utils::valueToString;
using namespace std;

HighPowerIntensityLockDevice::HighPowerIntensityLockDevice(ORBManager* orb_manager, std::string DeviceName, std::string configFilename) : 
STI_Device_Adapter(orb_manager, DeviceName, configFilename),
emissionStatusBitNum(2)
{
	gain = 1;

//	nextVCA = 0;
//	lastVCA = 0;
//	lockSetpoint = 1;
//	vcaSetpoint = 0;
	photodiodeSetpoint = 0;

	dynamicIntensitySetpoint = DynamicValue_ptr(new DynamicValue());

	configFile = ConfigFile_ptr(new ConfigFile(configFilename));

}

HighPowerIntensityLockDevice::~HighPowerIntensityLockDevice()
{
}

void HighPowerIntensityLockDevice::defineChannels()
{
	addInputChannel(0, DataVector, ValueNumber, "Lock Loop");
}

void HighPowerIntensityLockDevice::definePartnerDevices()
{

	std::string analogInIP = "";
	short analogInModule = 0;
	std::string analogInDeviceName = "";

	configFile->getParameter("analogInIP", analogInIP);
	configFile->getParameter("analogInModule", analogInModule);
	configFile->getParameter("analogInDeviceName", analogInDeviceName);
	configFile->getParameter("analogInChannel", analogInChannel);

	addPartnerDevice("Sensor", analogInIP, analogInModule, analogInDeviceName);


	std::string fastIP = "";
	short fastModule = 0;
	std::string fastDeviceName = "";

	configFile->getParameter("fastAnalogOutIP", fastIP);
	configFile->getParameter("fastAnalogOutModule", fastModule);
	configFile->getParameter("fastAnalogOutDeviceName", fastDeviceName);
	configFile->getParameter("fastAnalogOutChannel", fastChannel);

	addPartnerDevice("Actuator", fastIP, fastModule, fastDeviceName);


	//addPartnerDevice("Sensor", "ep-timing1.stanford.edu", 3, "Analog In");
	//addPartnerDevice("Actuator", "ep-timing1.stanford.edu", 6, "Fast Analog Out");

	partnerDevice("Sensor").enablePartnerEvents();
	partnerDevice("Actuator").enablePartnerEvents();
}


void HighPowerIntensityLockDevice::defineAttributes()
{
	addAttribute("Gain", gain);
	addAttribute("VCA Setpoint", 0);
}


void HighPowerIntensityLockDevice::refreshAttributes()
{
	setAttribute("Gain", gain);
	setAttribute("VCA Setpoint", vcaSetpoint);
}

bool HighPowerIntensityLockDevice::updateAttribute(std::string key, std::string value)
{
	bool success = false;
	std::string result;
	
	if( key.compare("Gain") == 0 ) {
		double newGain;
		if( STI::Utils::stringToValue(value, newGain) && newGain > 0 ) {
			gain = newGain;
			success = true;
		}
	}
	else if( key.compare("VCA Setpoint") == 0 ) {
		double newSetpoint;
		if( STI::Utils::stringToValue(value, newSetpoint) ) {
			vcaSetpoint = newSetpoint;
			success = true;
		}
	}

	return success;
}

void HighPowerIntensityLockDevice::parseDeviceEvents(const RawEventMap& eventsIn, 
	SynchronousEventVector& eventsOut) throw(std::exception)
{
	RawEventMap::const_iterator events;
	RawEventMap::const_iterator lastListEvent;

//	double lockSetpoint = 0;
	unsigned lockLoopChannel = 0;
	unsigned vcaSetpointChannel = 10;

	//unsigned short analogChannel = 0;

	double dtFeedback = 100.0e6;

	int nAverage;
	double dtAverage;
	configFile->getParameter("numberOfPointsAveraged", nAverage);
	configFile->getParameter("timeBetweenPointsAveraged", dtAverage);

	for(events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if(events->second.size() > 1) {
			throw EventConflictException(events->second.at(0), events->second.at(1), 
				"The HP Sideband Lock cannot currently have multiple events at the same time.");
		}
		
		if(events->second.at(0).channel() == lockLoopChannel) {
//			lockSetpoint = events->second.at(0).value().getNumber();

			photodiodeSetpoint = events->second.at(0).value().getNumber();

			dynamicIntensitySetpoint->setValue(vcaSetpoint);

			sensorCallback = MeasurementCallback_ptr(new HPLockCallback(this, nAverage));

			int i;
			for(i=0; i<nAverage; i++)
			{
				partnerDevice("Sensor").
					meas(events->first + i*dtAverage, analogInChannel, 1.1, events->second.at(0), sensorCallback, "Measure PD voltage");
			}

			partnerDevice("Actuator").
				event(events->first + dtFeedback, fastChannel, dynamicIntensitySetpoint, events->second.at(0), "Feedback on VCA");

//			partnerDevice("Actuator").event(events->first + dtFeedback, 0, nextVCA, events->second.at(0), "Feedback on VCA");

			//Add an measurement event to the HP Intensity Lock Device (this will record the VCA feedback value as a measuremnt)
			eventsOut.push_back( new HPIntensityLockEvent(events->first + 1.0e9, this) );
			eventsOut.back().addMeasurement( events->second.at(0) );
		}
	}
}

void HighPowerIntensityLockDevice::intensityLockLoop(double errorSignal)
{	
//	nextVCA = lastVCA + gain * errorSignal;
//	lastVCA = nextVCA;

	vcaSetpoint += gain * errorSignal;

	dynamicIntensitySetpoint->setValue(vcaSetpoint);

	refreshDeviceAttributes();	//update the attribute text file and the client
}

void HighPowerIntensityLockDevice::HPLockCallback::handleResult(const STI::Types::TMeasurement& measurement)
{
//	using namespace std;
//	double gainLocal = _this->gain;
//	double setpointLocal = _this->photodiodeSetpoint;

	boost::lock_guard<boost::mutex> lock(callbackMutex);

	double error;

	//Only keep measurements for averaging if valid
	if(measurement.data.doubleVal() > -9.9) {
		runningTotal += measurement.data.doubleVal();
		successfulMeasurements++;
	}

	//Take average on final callback
	//_nAverage--;
	numberOfResults++;
	if(_nAverage == numberOfResults) {

		if(successfulMeasurements > 0) {
			//Take the average PD voltage and apply feedback
			_this->photodiodeVoltage = runningTotal / successfulMeasurements;
			std::cerr << "PD Voltage: " << _this->photodiodeVoltage << std::endl;
			
			error = (_this->photodiodeSetpoint) - (_this->photodiodeVoltage);
			_this->intensityLockLoop(error);
		}
		runningTotal = 0;
		successfulMeasurements = 0;
		numberOfResults = 0;
	}


}


void HighPowerIntensityLockDevice::HPIntensityLockEvent::collectMeasurementData()
{
	//save the current value of the VCA setpoint
	MixedData vcaData;
	vcaData.addValue(std::string("VCA Setpoint"));
	vcaData.addValue(_this->vcaSetpoint);

	//Also save the PD voltage:
	MixedData pdData;
	pdData.addValue(std::string("PD Voltage"));
	pdData.addValue(_this->photodiodeVoltage);

	MixedData feedbackLoopData;
	feedbackLoopData.addValue( vcaData );
	feedbackLoopData.addValue( pdData );

	//Save feedbackLoopData as a measurement for the HP Intensity Lock device
	eventMeasurements.at(0)->setData( feedbackLoopData );
}
//
//HighPowerIntensityLockDevice::HPLockCallback:HPLockCallback(HighPowerIntensityLockDevice* thisDevice, int nAverage) : 
//_this(thisDevice) 
//{
//	//		_this = thisDevice;
//}
