/*! \file MOTMagn_Device.cpp
 *  \author Susannah Dickerson
 *  \brief Template for STI_Devices
 *  \section license License
 *
 *  Copyright (C) 2010 Susannah Dickerson <sdickers@stanford.edu>\n
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



#include "MOTMagn_Device.h"


//MOTMagn_Device::MOTMagn_Device(ORBManager*    orb_manager, 
//					   std::string    DeviceName, 
//					   std::string    Address, 
//					   unsigned short ModuleNumber, 
//					   unsigned short comPort, std::string logDirectory, std::string configFilename) : 
MOTMagn_Device::MOTMagn_Device(ORBManager* orb_manager, const ConfigFile& configFile) : 
STI_Device(orb_manager, configFile)
{
	//Initialization of device
	std::string IDstring;
	int pos;
	unsigned short comPort;
	enableDataLogging = false;

	initialized = true;

	if (!(configFile.getParameter("ComPort", comPort))) {
		deviceShutdown();
		cerr << "Could not find ComPort number in inialization file." << endl;
		return;
	}


	myRS485Controller = new rs232Controller(std::string("COM" + valueToString(comPort)), 9600, 8, "None", 1, "Software");
//	myRS485Controller = new rs232ControllerNT("COM" + valueToString(comPort));

	double x;
	double y;
	double z;
	std::string ID;
	bool parseSuccess;
	Magnetometer magnetometer;
	std::vector <double> calibrationVec;

	std::string error_msg;

	//Check that we can talk to the device
	if (!myRS485Controller->initialized)
	{
		initialized = false;
		return;
	}

	//Determine the number and IDs of all magnetometers attached
	IDstring = myRS485Controller->queryDevice("*99ID\r", 250);

	pos = IDstring.find("ID= ");
	while(std::string::npos != pos)
	{
		ID = IDstring.substr(pos + 4,2);
		parseSuccess = configFile.getParameter(ID + " x", x);
		if (!parseSuccess)
			x = 0;
		parseSuccess = configFile.getParameter(ID + " y", y);
		if (!parseSuccess)
			y = 0;
		parseSuccess = configFile.getParameter(ID + " z", z);
		if (!parseSuccess)
			z = 0;

		calibrationVec.clear();
		calibrationVec.push_back(x);
		calibrationVec.push_back(y);
		calibrationVec.push_back(z);

		magnetometer.setMagnetometer(ID, calibrationVec);
		magnetometers[magnetometer.getIDNumber()]=magnetometer;
		IDstring.erase(0,7);
		pos = IDstring.find("ID= ");

		std::map<short, Magnetometer>::iterator it;
		//Setup magnetometer for taking data
		error_msg = myRS485Controller->queryDevice("*" + ID + "WE");
		if (error_msg.compare(0,2,"OK") != 0){
			std::cerr << "Error setting magnetometer " << ID << " to write enable: "<< error_msg << std::endl;
			std::cerr << "Removing magnetometer " << ID << std::endl;
			it = magnetometers.find(magnetometer.getIDNumber());
			magnetometers.erase(it);
		}

		error_msg = myRS485Controller->queryDevice("*" + ID + "B");
		if (error_msg.compare(0,9,"BINARY ON") != 0){
			std::cerr << "Error setting magnetometer " << ID << "to binary read mode" << error_msg << std::endl;
			std::cerr << "Removing magnetometer " << ID << " if necessary" << std::endl;
			it = magnetometers.find(magnetometer.getIDNumber());
			if (it != magnetometers.end())
				magnetometers.erase(it);
		}
	};

	if (magnetometers.empty()) {
		initialized = false;
		return;
	}
}

MOTMagn_Device::~MOTMagn_Device()
{
	delete myRS485Controller;
}


bool MOTMagn_Device::deviceMain(int argc, char **argv)
{

	refreshAttributes();
	
	MixedData data1;
	MixedData data2;
	MixedData data3;
	std::string str = "str";
	MixedData data4 = str;
	std::vector <double> intVec1;
	std::vector <int> intVec2;

	intVec1.push_back(1.1);
	intVec1.push_back(2);
	intVec1.push_back(3);

	intVec2.push_back(2);
	intVec2.push_back(2);
	intVec2.push_back(2);

	data1.addValue(1);
	data1.addValue(intVec1);
	data2.addValue(2);
	data2.addValue(intVec2);

	data3 = data1 + data2;

	std::cerr << data3.print() << std::endl;

	data3 = data1 - data2;

	std::cerr << data3.print() << std::endl;

	data3 = data1 * data2;

	std::cerr << data3.print() << std::endl;

	data3 = data1 / data2;

	std::cerr << data3.print() << std::endl;

	data3 = MixedData(1)/data2;

	std::cerr << data3.print() << std::endl;

	data3 = data2 - 1;

	std::cerr << data3.print() << std::endl;

	data3.setValue(1);
	data3.addValue(intVec2);

	std::cerr << ((data3 < data2) ? "True" : "False")  << std::endl;

	return false;
}

void MOTMagn_Device::defineAttributes()
{
	std::vector<Magnetometer>::iterator it;

	/*for (it = magnetometers.begin(); it != magnetometers.end(); it++)
	{
		addAttribute(it->ID + " x", 0);
		addAttribute(it->ID + " y", 0);
		addAttribute(it->ID + " z", 0);
	}*/

	addAttribute("Enable Data Logging", "Off", "On, Off");
}

void MOTMagn_Device::refreshAttributes() 
{
	std::vector<Magnetometer>::iterator it;
	std::vector <double> measurement;
	//bool error;

	//for (it = magnetometers.begin(); it != magnetometers.end(); it++)
	//{
	//	error = measureField(*it, measurement);
	//	setAttribute(it->ID + " x", error ? 0 : measurement.at(0));
	//	setAttribute(it->ID + " y", error ? 0 : measurement.at(1)); 
	//	setAttribute(it->ID + " z", error ? 0 : measurement.at(2)); 
	//	measurement.clear();
	//}
	
	setAttribute("Enable Data Logging", (enableDataLogging ? "On" : "Off")); //response to the IDN? query
}

bool MOTMagn_Device::updateAttribute(string key, string value)
{
	double tempDouble;  //the value entered, converted to a number
	int tempInt;

	bool successDouble = stringToValue(value, tempDouble);
	bool successInt = stringToValue(value, tempInt);

	bool success = successDouble || successInt;

	std::vector<Magnetometer>::iterator it;

	//Doesn't actually do anything; this device monitors only
	//for (it = magnetometers.begin(); it != magnetometers.end(); it++) {

	//	if(key.compare(it->ID + " x") == 0)
	//	{
	//		success = true;
	//	}
	//	else if(key.compare(it->ID + " y") == 0)
	//	{
	//		success = true;
	//	}
	//	else if(key.compare(it->ID + " z") == 0)
	//	{
	//		success = true;
	//	}
	//}

	if(key.compare("Enable Data Logging") == 0)
	{
		if(value.compare("On") == 0)
		{
			enableDataLogging = true;
			startDataLogging();
		}
		else
		{
			enableDataLogging = false;
			stopDataLogging();
		}

		success = true;
	}

	return success;
}

void MOTMagn_Device::defineChannels()
{
	std::map<short, Magnetometer>::iterator it;
	for (it = magnetometers.begin(); it != magnetometers.end(); it++)
	{
		addInputChannel(it->first, DataString, ValueNumber);
		addLoggedMeasurement(it->first, 5, 5);
	}
	addOutputChannel(100, ValueString); // The Honeywell magnetometer IDs never exceed 99
}

bool MOTMagn_Device::readChannel(unsigned short channel, const MixedValue& valueIn, MixedData& dataOut)
{
	std::vector<std::string>::iterator it;
	std::vector <double> measurement;
	bool success = true;

	if (magnetometers.find(channel) == magnetometers.end())
	{
		std::cerr << "Expect a channel between 0 and " << valueToString(magnetometers.size()) << ", not " << valueToString(channel) << std::endl;
		return false;
	}

	success = !measureField(magnetometers.find(channel)->second, measurement);

//	std::cerr << measurement.at(0) << std::endl;
//	std::cerr << measurement.at(1) << std::endl;
//	std::cerr << measurement.at(2) << std::endl;
	dataOut.setValue(measurement);

	measurement.clear();

	return success;
}

bool MOTMagn_Device::writeChannel(unsigned short channel, const MixedValue& valuet)
{
	bool success = true;
	std::string msg = "";
	std::string valueString;

	if (channel < magnetometers.size())
	{
		std::cerr << "Expect a channel greater than or equal to " << valueToString(magnetometers.size()) << ", not " << valueToString(channel) << std::endl;
		return false;
	} else if (valuet.getType() != MixedValue::String)
	{
		std::cerr << "Expect a string as input to writeChannel" << std::endl;
		return false;
	}
 
	valueString = valuet.getString();

	if(valueString.compare(0,1,"s")==0 || valueString.compare(0,1,"S")==0 ||
		valueString.compare(0,3,"set")==0 || valueString.compare(0,3,"Set")==0)
	{
		setMagnetometers(valueString);
	} else if(valueString.compare(0,1,"r")==0 || valueString.compare(0,1,"R")==0 ||
		valueString.compare(0,5,"reset")==0 || valueString.compare(0,5,"Reset")==0)
	{
		resetMagnetometers(valueString);
	} else if(valueString.compare(0,2,"sr")==0 || valueString.compare(0,2,"SR")==0 ||
		valueString.compare(0,3,"s/r")==0 || valueString.compare(0,3,"S/R")==0 ||
		valueString.compare(0,9,"set/reset")==0 || valueString.compare(0,9,"Set/Reset")==0)
	{
		setMagnetometers(valueString);
		resetMagnetometers(valueString);
	}

	return success;
}

void MOTMagn_Device::setMagnetometers(std::string inString)
{
	std::string ID;
	size_t pos;

	pos = inString.find("ID=");
	if(pos != std::string::npos)
	{
		ID = inString.substr(pos + 3,2);
		myRS485Controller->queryDevice("*" + ID + "]S");
	} else
	{
		std::map<short,Magnetometer>::iterator it;
		for (it = magnetometers.begin(); it != magnetometers.end(); it++)
		{
			myRS485Controller->queryDevice("*" + it->second.ID + "]S");
		}
	}
}

void MOTMagn_Device::resetMagnetometers(std::string inString)
{
	std::string ID;
	size_t pos;

	pos = inString.find("ID=");
	if(pos != std::string::npos)
	{
		ID = inString.substr(pos + 3,2);
		myRS485Controller->queryDevice("*" + ID + "]R");
	} else
	{
		std::map<short,Magnetometer>::iterator it;
		for (it = magnetometers.begin(); it != magnetometers.end(); it++)
		{
			myRS485Controller->queryDevice("*" + it->second.ID + "]R");
		}
	}
}

void MOTMagn_Device::definePartnerDevices()
{
}

std::string MOTMagn_Device::execute(int argc, char **argv)
{
	return "";
}

void MOTMagn_Device::parseDeviceEvents(const RawEventMap& eventsIn, 
        SynchronousEventVector& eventsOut) throw(std::exception)
{
	parseDeviceEventsDefault(eventsIn, eventsOut);
}

void MOTMagn_Device::stopEventPlayback()
{
}

bool MOTMagn_Device::measureField(Magnetometer &magnetometer, std::vector <double> & measurement)
{    	
	bool error = false;
	int signedResult;
	unsigned int shortResult;
	double result;

	std::vector <int> output = myRS485Controller->binaryQueryDevice("*" + magnetometer.ID + "P");  //binaryQueryDevice

	//We expect 2 ints for each axis, plus the end character, ASCII(13).
	if (output.size() == 7) {
		for(unsigned int i=0;i<3;i++){
			signedResult = (signed int)output[2*i];
			shortResult = (unsigned int) (unsigned char) output[2*i+1];

			if(signedResult<0)
					result = -(double)(( abs( signedResult))*256 + shortResult);
			else
					result = (double)((abs( signedResult))*256 +  shortResult);
			
			//The HMR2300 scaling is 15000 pts / Gauss.
//			measurement.push_back((result-magnetometer.calibration.at(i))/15);	
			measurement.push_back((result)/15);
		};
	} else {
		error = true;
	}
   
    return error; 	
	
}