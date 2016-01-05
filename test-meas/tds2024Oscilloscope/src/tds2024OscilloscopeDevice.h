/*! \file tds2024OscilloscopeDevice.h
 *  \author David M.S. Johnson
 *  \brief header file
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


#ifndef TDS2024DEVICE_H
#define TDS2024DEVICE_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "visa32Controller.h"
#include <STI_Device.h>


class tds2024OscilloscopeDevice : public STI_Device
{
public:
	
	tds2024OscilloscopeDevice(ORBManager* orb_manager, 
		std::string DeviceName, 
		std::string Address, 
		unsigned short ModuleNumber,
		unsigned short comPort, std::string logDirectory);
	~tds2024OscilloscopeDevice();

private:

// Device main()
    bool deviceMain(int argc, char** argv);    //called in a loop while it returns true

    // Device Attributes
    void defineAttributes();
    void refreshAttributes();
    bool updateAttribute(std::string key, std::string value);

    // Device Channels
    void defineChannels();
	bool readChannel(unsigned short channel, const MixedValue& valueIn, MixedData& dataOut);
	bool writeChannel(unsigned short channel, const MixedValue& value) {return false;}

    // Device Command line interface setup
	void definePartnerDevices() {};
    std::string execute(int argc, char** argv);

    // Device-specific event parsing
    void parseDeviceEvents(const RawEventMap& eventsIn, 
		SynchronousEventVector& eventsOut) throw(std::exception) {parseDeviceEventsDefault(eventsIn, eventsOut);};

	// Event Playback control
	void stopEventPlayback() {};	//for devices that require non-generic stop commands
	void pauseEventPlayback() {};
	void resumeEventPlayback() {};

	std::string generateDate();
//	void savedata(std::vector <__int64> &timeVector, std::vector <double> &pressureVector);

private:

	visa32Controller* myVISA32Controller;
	double pressure;
	double voltage;
	double current;
	std::string result;
	bool enableDataLogging;
	std::string deviceName;
	std::string deviceID;

	//define data containers
	std::vector <__int64> timeVector;
	std::vector <double> pressureVector;
};

#endif

