/*! \file gpib_hub_device.cpp
 *  \author David M.S. Johnson
 *  \brief Source-file for the class gpib_hub
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



#include "gpibControllerDevice.h"
#include "GPIB_device.h"



gpibControllerDevice::gpibControllerDevice(ORBManager*    orb_manager, 
							std::string    DeviceName, 
							std::string    Address, 
							unsigned short ModuleNumber, std::string gpibHost) : 
STI_Device(orb_manager, DeviceName, Address, ModuleNumber)
{
	gpibController = new GPIB_device(gpibHost);
}

gpibControllerDevice::~gpibControllerDevice()
{
}

void gpibControllerDevice::defineChannels()
{
	addInputChannel(0, STI::Types::DataVector, ValueVector);
}
bool gpibControllerDevice::readChannel(unsigned short channel, const MixedValue& valueIn, MixedData& dataOut)
{
	if (channel == 0)
	{
		if(valueIn.getType() != MixedValue::Vector)
		{
			std::cerr << "Error: expecting vector as input: (primary address, secondary address, command string, size to read)" << std::endl;
			return false;
		}
		
		MixedValueVector inVector = valueIn.getVector();
		if (inVector.size() != 4)
		{
			std::cerr << "Error: expecting vector as input: (primary address, secondary address, command string, size to read)" << std::endl;
			return false;
		}

		if (inVector.at(0).getType() != MixedValue::Double || inVector.at(1).getType() != MixedValue::Double || inVector.at(2).getType() != MixedValue::String || inVector.at(3).getType() != MixedValue::Double)
		{
			std::cerr << "Error: expecting vector as input: (primary address, secondary address, command string, size to read)" << std::endl;
			return false;
		}

		int primaryAddress = inVector.at(0).getInt();
		int secondaryAddress = inVector.at(1).getInt();
		std::string command = inVector.at(2).getString();
		int readSize = inVector.at(3).getInt();

		std::vector <unsigned char> result;

		gpibController->QueryAndReadChars(primaryAddress, secondaryAddress, const_cast<char*>(command.c_str()), result, readSize);
		
		dataOut.setValue(result);
		return true;

	}
	else
		return false;
}
std::string gpibControllerDevice::execute(int argc, char **argv)
{
	std::vector<std::string> argvOutput;
	STI::Utils::convertArgs(argc, argv, argvOutput);
	
	int primaryAddress;
	int secondaryAddress;
	string command;
	int query = 0; //true (1) or false (0) if the command is expecting a response
	int commandLength = argvOutput.size() - 4; //one for name, 3 for prim, sec, and query?

	if(argvOutput.size() < 5)
		return ""; //command needs to contain 4 pieces of information
	
	bool querySuccess = stringToValue(argvOutput.back(), query); //gets the last element in the vector, which is the query entry
	bool primaryAddressSuccess = stringToValue(argvOutput.at(1), primaryAddress);
	bool secondaryAddressSuccess = stringToValue(argvOutput.at(2), secondaryAddress);

	for(int j = 0; j < commandLength; j++)
	{
		if(j==0)
			command = argvOutput.at(3 + j);
		else
			command = command + " " + argvOutput.at(3 + j);
	}

	std::cerr << "command is: " << command << std::endl;

	if( (query == 1) && querySuccess)
	{
		//query device
		gpibController->Query_Device(primaryAddress, secondaryAddress, const_cast<char*>(command.c_str()), result, 100);
		std::cerr << "result is: " << result << std::endl;
		return result;
	}
	else if( (query == 0) && querySuccess)
	{
		//command device
		gpibController->Command_Device(primaryAddress, secondaryAddress, const_cast<char*>(command.c_str()), result, 100);
		std::cerr << "result is: " << "worked" << std::endl;
		return "1";
	}
	else if( (query == 2) && querySuccess)
	{
		// this is my "clever" solution for parsing and handling readUntilNewLine via partners
		gpibController->readUntilNewLine(primaryAddress, secondaryAddress, const_cast<char*>(command.c_str()), result);
		return result;
	}
	else if( (query == 3) && querySuccess)
	{
		// this is my "clever" solution for parsing and handling readUntilTerminationCharacter via partners
		gpibController->readUntilTerminationCharacter(primaryAddress, secondaryAddress, const_cast<char*>(command.c_str()), result);
		return result;
	}
	/*else if( (query == 4) && querySuccess)
	{
		// this is my "clever" solution for parsing and handling arbitrary length queries via partners
		size_t found = command.find_last_of(" ");
		int readSize;
		bool success = stringToValue(command.substr(found + 1, command.size() - found),readSize);
		gpibController->QueryAndReadDoubles(primaryAddress, secondaryAddress, const_cast<char*>(command.substr(0, found).c_str()), result, readSize);
		return result;
	}*/
	else
		return "";
	
}
bool gpibControllerDevice::deviceMain(int argc, char **argv)
{
	return false;
}