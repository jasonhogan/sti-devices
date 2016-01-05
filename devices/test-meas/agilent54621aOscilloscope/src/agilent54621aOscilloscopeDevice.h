/*! \file agilent54621aOscilloscopeDevice.h
 *  \author David M.S. Johnson
 *  \brief header file for agilent54621aOscilloscopeDevice class
 *  \section license License
 *
 *  Copyright (C) 2009 David Johnson <david.m.johnson@stanford.edu>\n
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


#ifndef AGILENT54621ADEVICE_H
#define AGILENT54621ADEVICE_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <STI_Device.h>
#include <map>
#include <string>
//#include <iostream.h>

class agilent54621aOscilloscopeDevice : public STI_Device
{
public:
	
	agilent54621aOscilloscopeDevice(ORBManager* orb_manager, 
		std::string DeviceName, 
		std::string Address, 
		unsigned short ModuleNumber,
		unsigned short primaryGPIBAddress);
	~agilent54621aOscilloscopeDevice();

private:

// Device main()
    bool deviceMain(int argc, char** argv);    //called in a loop while it returns true

    // Device Attributes
    void defineAttributes();
    void refreshAttributes();
    bool updateAttribute(std::string key, std::string value);

    // Device Channels
    void defineChannels();
	bool readChannel(unsigned short channel, const MixedValue& valueIn, MixedData& dataOut) {return false;}
	bool writeChannel(unsigned short channel, const MixedValue& value) {return false;}

    // Device Command line interface setup
    void definePartnerDevices();
    std::string execute(int argc, char** argv);

    // Device-specific event parsing
    void parseDeviceEvents(const RawEventMap& eventsIn, 
        SynchronousEventVector& eventsOut) throw(std::exception);

	// Event Playback control
	void stopEventPlayback();	//for devices that require non-generic stop commands
	void pauseEventPlayback() {};
	void resumeEventPlayback() {};

	//functions for generating commands
	std::string agilent54621aOscilloscopeDevice::queryDevice(std::string query); //returns query result if worked, else ""
	bool agilent54621aOscilloscopeDevice::commandDevice(std::string command); //returns true if it worked

	unsigned short primaryAddress;
	unsigned short secondaryAddress;
	std::string gpibID;
	std::string laserHeadHours; 
	std::string controllerHours; 
	std::string laserWavelength;
	bool powerOn;
	double laserCurrent;
	double newLaserCurrent;
	double piezoVoltage;
	double newPiezoVoltage;
	bool piezoGainHigh;

	//static enum StringValue {	PiezoVoltageString,
	//							laserCurrentString };

	//static std::map<std::string, StringValue> s_mapStringValues;


};

#endif

