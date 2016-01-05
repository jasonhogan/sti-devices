/*! \file agilent54621aOscilloscopeDevice.cpp
 *  \author David M.S. Johnson
 *  \brief Source-file for the class agilent54621aOscilloscopeDevice
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



#include "agilent54621aOscilloscopeDevice.h"

agilent54621aOscilloscopeDevice::agilent54621aOscilloscopeDevice(ORBManager*    orb_manager, 
							std::string    DeviceName, 
							std::string    Address, 
							unsigned short ModuleNumber,
							unsigned short primaryGPIBAddress) : 
STI_Device(orb_manager, DeviceName, Address, ModuleNumber)
{ 
	primaryAddress = primaryGPIBAddress; //normally 1
	secondaryAddress = 0;
	powerOn = false; // default to power off
	laserCurrent = 47.9; // in mA
	piezoVoltage = 62.7; // in Volts
	piezoGainHigh = false; // default to low gain
	gpibID = "Have Not Queried"; // initializes with null result - haven't checked yet
	laserHeadHours = "Have Not Queried"; // initializes with null result - haven't checked yet
	controllerHours = "Have Not Queried"; // initializes with null result - haven't checked yet
	laserWavelength = "Have Not Queried"; // initializes with null result - haven't checked yet
}

agilent54621aOscilloscopeDevice::~agilent54621aOscilloscopeDevice()
{
}


void agilent54621aOscilloscopeDevice::defineAttributes() 
{
	addAttribute("GPIB ID", gpibID); //response to the IDN? query
	addAttribute("Laser Head Operating Hours", laserHeadHours); 
	addAttribute("Controller Operating Hours", controllerHours);
	addAttribute("Laser Wavelength", laserWavelength);
	addAttribute("Laser Current (mA)", laserCurrent);
	addAttribute("Piezo Voltage (V)", piezoVoltage);
	addAttribute("Power", "Off", "Off, On");
	addAttribute("Piezo Gain", "Low", "Low, High");
}

void agilent54621aOscilloscopeDevice::refreshAttributes() 
{
	setAttribute("GPIB ID", gpibID); //will send the IDN? query
	setAttribute("Laser Head Operating Hours", laserHeadHours); //will send DHO?
	setAttribute("Controller Operating Hours", controllerHours); //will send SHO?
	setAttribute("Laser Wavelength", laserWavelength); //will send HWAV?
	setAttribute("Laser Current (mA)", laserCurrent);
	setAttribute("Piezo Voltage (V)", piezoVoltage);
	setAttribute("Power", (powerOn ? "On" : "Off"));
	setAttribute("Piezo Gain", (piezoGainHigh ? "High" : "Low"));
}

bool agilent54621aOscilloscopeDevice::updateAttribute(string key, string value)
{
	//converts desired command into GPIB command string and executes via gpib controller partner device
	double tempDouble;
	bool successDouble = stringToValue(value, tempDouble);
	bool commandSuccess;
	bool success = false;
	string result;

	if(key.compare("GPIB ID") == 0)
	{
		gpibID = queryDevice("*idn?");
		if(gpibID.compare("") == 0)
			success = false;
		else
			success = true;
		std::cerr << "Identification: " << gpibID << std::endl;
	}
	else if(key.compare("Laser Head Operating Hours") == 0)
	{
		laserHeadHours = queryDevice(":SYST:INF:DHO?");
		if(laserHeadHours.compare("") == 0)
			success = false;
		else
			success = true;
		std::cerr << "Laser Head Operating Hours: " << laserHeadHours << std::endl;
	}
	else if(key.compare("Controller Operating Hours") == 0)
	{
		controllerHours = queryDevice(":SYST:INF:SHO?");
		if(controllerHours.compare("") == 0)
			success = false;
		else
			success = true;
		std::cerr << "Controller Operating Hours: " << controllerHours << std::endl;
	}
	else if(key.compare("Laser Wavelength")==0)
	{
		laserWavelength = queryDevice(":SYST:INF:HWAV?");
		if(laserWavelength.compare("") == 0)
			success = false;
		else
			success = true;
		std::cerr << "Laser Wavelength: " << laserWavelength << std::endl;
	}
	else if(key.compare("Piezo Voltage (V)") == 0)
	{
		bool successPiezoVoltage = stringToValue(value, newPiezoVoltage);
		if(successPiezoVoltage && newPiezoVoltage < 117.5 && newPiezoVoltage > 0) 
		{
			std::string piezoCommand = ":SOUR:VOLT:PIEZ " + value;
			std::cerr << "piezo_command_str: " << piezoCommand << std::endl;
			commandSuccess = commandDevice(piezoCommand);
			std::cerr << "device successfully commanded"<< std::endl;
			if(commandSuccess)
			{
				result = queryDevice(":SOUR:VOLT:PIEZ?");
				if(result.compare("") == 0)
					success =  false;
				else
				{	
					successPiezoVoltage = stringToValue(result, piezoVoltage);
					success = true;
				}
			}
			else
				success = false;
			}
		else
		{
			std::cerr << "The desired voltage is outside of the allowed range." << std::endl;
			success = false;
		}
	}
	else if(key.compare("Power") == 0)
	{
		if(value.compare("On") == 0)
		{
			commandSuccess = commandDevice(":OUTP 1");
			powerOn = true;
		}
		else
		{
			commandSuccess = commandDevice(":OUTP 0");
			powerOn = false;
		}
		if(commandSuccess)
			success = true;
	/*	
		if(commandSuccess)
			{
				result = queryDevice(":OUTP?");
				int powerStatus;
				bool successPowerStatus = stringToValue(result, powerStatus);
				if(result.compare("") == 0)
					success =  false;
				else
				{	
					std::cerr << "Power Status is: " << result << std::endl;
					if(powerStatus == 1)
					{
						success = true;
						powerOn = true;
						std::cerr << "Laser Turned On" << std::endl;
					}
					if(powerStatus == 0)
					{
						success = true;
						powerOn = false;
						std::cerr << "Laser Turned Off" << std::endl;
					}
					else
					{
						success = false;
					}
				}
			}
		else
			success = false;
			*/
	}
	else if(key.compare("Piezo Gain") == 0)
	{
		if(value.compare("High") == 0)
		{
			//set gain to high (25x)
			commandSuccess = commandDevice(":CONF:GAIN:HIGH");
			std::cerr << "Gain commanded High (25x)." << std::endl;
		}
		else
		{
			//set gain to low (1x)
			commandSuccess = commandDevice(":CONF:GAIN:LOW");
			std::cerr << "Gain commanded Low (1x)." << std::endl;
		}
		if(commandSuccess)
			{
				std::string testResult;
				result = queryDevice(":CONF:GAIN?");
				if(result.compare("") == 0)
					success =  false;
				else
				{	
					testResult.assign(result, 0, 3);
					std::cerr << "Piezo Gain is: " << "***" << testResult << "***" << std::endl;
					if(testResult.compare("HIG") == 0)
					{
						success = true;
						piezoGainHigh = true;
						std::cerr << "set success to true" << std::endl;

					}
					else if(testResult.compare("LOW") == 0)
					{
						success = true;
						piezoGainHigh = false;
						std::cerr << "set success to true" << std::endl;
					}
					else
					{
						success = false;
						std::cerr << "set success to false" << std::endl;
					}
				}
			}
		else
			success = false;
	}
	else if(key.compare("Laser Current (mA)") == 0)
	{
		bool successLaserCurrent = stringToValue(value, newLaserCurrent);
		if(successLaserCurrent && newLaserCurrent < 50.0 && newLaserCurrent > 0) 
		{
			std::string currentCommand = ":SOUR:CURR " + value;
			std::cerr << "current_command_str: " << currentCommand << std::endl;
			commandSuccess = commandDevice(currentCommand);
			std::cerr << "device successfully commanded"<< std::endl;
			if(commandSuccess)
			{
				result = queryDevice(":SOUR:CURR?");
				if(result.compare("") == 0)
					success =  false;
				else
				{	
					commandSuccess = stringToValue(result, laserCurrent);
					success = true;
				}
			}
			else
				success = false;
			}
		else
		{
			std::cerr << "The desired current is outside of the allowed range." << std::endl;
			success = false;
		}
	}

	return success;
}

void agilent54621aOscilloscopeDevice::defineChannels()
{
}


void agilent54621aOscilloscopeDevice::parseDeviceEvents(const RawEventMap& eventsIn, 
        SynchronousEventVector& eventsOut) throw(std::exception)
{
	
}
void agilent54621aOscilloscopeDevice::definePartnerDevices()
{
	addPartnerDevice("gpibController", "eplittletable.stanford.edu", 0, "gpib"); //local name (shorthand), IP address, module #, device name as defined in main function
}

void agilent54621aOscilloscopeDevice::stopEventPlayback()
{
}

std::string agilent54621aOscilloscopeDevice::execute(int argc, char **argv)
{
	string commandString;
	string commandValue;
	
	int query = 0; //true (1) or false (0) if the command is expecting a response
	double measuredValue = 0;
	//bool commandSuccess;
	//double commandValue;
	//bool outputSuccess;
	string result;

	//command comes as "attribute value query?"
	if(argc == 5)
	{
		commandValue = argv[4];
		commandString = ":SOUR:VOLT:PIEZ " + commandValue;
		result = commandDevice(commandString);
	}
	if(argc == 4)
	{
		result = queryDevice(":SOUR:VOLT:PIEZ?");
		return result;
	}
	else
		return "0"; //command needs to contain 2 pieces of information

	/*
	if(commandSuccess)
	{
		outputSuccess = setAttribute(attribute, commandValue); //will only work with attributes that take doubles
		
		if(outputSuccess)
			return "1";
		else
			return "0";
	}
	else
		return "0";	
	*/
}
bool agilent54621aOscilloscopeDevice::deviceMain(int argc, char **argv)
{
	return false;
}
std::string agilent54621aOscilloscopeDevice::queryDevice(std::string query)
{
	std::string queryString;
	std::string result;
	queryString = valueToString(primaryAddress) + " " + valueToString(secondaryAddress) + " " + query + " 1";
	std::cerr << "query_str: " << queryString << std::endl;

	result = partnerDevice("gpibController").execute(queryString.c_str()); //usage: partnerDevice("lock").execute("--e1");

	return result;
}
bool agilent54621aOscilloscopeDevice::commandDevice(std::string command)
{
	std::string commandString;
	std::string result;
	commandString = valueToString(primaryAddress) + " " + valueToString(secondaryAddress) + " " + command + " 0";

	result = partnerDevice("gpibController").execute(commandString.c_str()); //usage: partnerDevice("lock").execute("--e1");

	if(result.compare("1")==0)
		return true;
	else
		return false;
}