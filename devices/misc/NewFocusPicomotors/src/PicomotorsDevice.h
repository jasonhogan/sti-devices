/*! \file PicomotorsDevice.h
 *  \author Jason Hogan
 *  \brief header file for PicomotorsDevice
 *  \section license License
 *
 *  Copyright (C) 2012 Jason Hogan <hogan@stanford.edu>\n
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


#ifndef PICOMOTORS_DEVICE_H
#define PICOMOTORS_DEVICE_H


#include <STI_Device_Adapter.h>
#include "rs232Controller.h"


class PicomotorsDevice : public STI_Device_Adapter
{
public:

	PicomotorsDevice(ORBManager* orb_manager, const ConfigFile& configFile);

	PicomotorsDevice(ORBManager* orb_manager, std::string DeviceName, 
		std::string IPAddress, unsigned short ModuleNumber, unsigned short comPort); //Deprecated

	void init(unsigned short comPort);	//common constructor functions; initialize serial port


	void defineAttributes();
	void refreshAttributes();
	bool updateAttribute(std::string key, std::string value);
	
	void defineChannels();
	bool writeChannel(unsigned short channel, const MixedValue& value);

	std::string execute(int argc, char* argv[]);

private:

	void readMotorParameters();

	void refreshMotorVelocity(unsigned i);
	void refreshMotorAcceleration(unsigned i);

	void setMotorVelocity(unsigned i, unsigned velocity);
	void setMotorAcceleration(unsigned i, unsigned acceleration);

	int motorVelocity[3];
	int motorAcceleration[3];

	rs232Controller* serialController;

	int rs232QuerySleep_ms;
};

#endif
