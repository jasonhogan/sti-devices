/*! \file stf_da_slow_device.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class stf_da_slow_device
 *  \section license License
 *
 *  Copyright (C) 2008 Jason Hogan <hogan@stanford.edu>\n
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


#include "stf_da_slow_device.h"


stf_da_slow_device::stf_da_slow_device(ORBManager* orb_manager, std::string configFilename) : 
FPGA_Device(orb_manager, "Slow Analog Out", configFilename)
{
	//slow analog out event holdoff parameters
	minimumEventSpacing = 1050; //1.05*us in nanoseconds - this is experimentally verified
	minimumAbsoluteStartTime = 10000; //10*us in nanoseconds - this is a guess right now to let everything get sorted out
	holdoff = minimumEventSpacing + 2000 + 7100 - 5000; //the holdoff is equal to the minimum event spacing + 1*us - experimentally determined

}

stf_da_slow_device::~stf_da_slow_device()
{
}

bool stf_da_slow_device::deviceMain(int argc, char **argv)
{
	return false;
}
	
void stf_da_slow_device::defineAttributes()
{
}

void stf_da_slow_device::refreshAttributes()
{
}

bool stf_da_slow_device::updateAttribute(std::string key, std::string value)
{
	return false;
}

void stf_da_slow_device::defineChannels()
{
	for( int i = 0; i < 40; i++ )
	{
		addOutputChannel(i, ValueNumber);
	}
}

void stf_da_slow_device::definePartnerDevices()
{
}

std::string stf_da_slow_device::execute(int argc, char **argv)
{
	std::vector<std::string> argvOutput;
	STI::Utils::convertArgs(argc, argv, argvOutput);

	uInt32 channel;
	double value;
	bool convertSuccess = true;

	if(argvOutput.size() == 3)
	{
		// expect channel, value
		convertSuccess = stringToValue(argvOutput.at(1), channel);
		if(!convertSuccess)
			return "Error: Unable to convert channel argument.";

		convertSuccess &= stringToValue(argvOutput.at(2), value);
		if(!convertSuccess)
			return "Error: Unable to convert value argument.";
	}
	else
		return "Error: Invalid argument list. Expecting 'channel' and 'value'."; // don't know what the user was trying to do

	if(write(channel, value)) //runs parseDeviceEvents on rawEvent and executes a short timing sequence
		return "";
	else
		return "Error: Failed when attempting to write.";
}

void stf_da_slow_device::parseDeviceEventsFPGA(const RawEventMap &eventsIn, SynchronousEventVector& eventsOut) 
throw(std::exception)
{
	RawEventMap::const_iterator events;

	double eventTime; //time when the FPGA should trigger in order to have the output ready in time
	double previousTime; //time when the previous event occurred

	for(events = eventsIn.begin(); events != eventsIn.end(); events++)
	{
		if(events != eventsIn.begin())
		{
			events--;
			previousTime = events->first;
			events++;
		}
		else {
			//triggerOffset accounts for delay due to external triggering

			previousTime = (minimumAbsoluteStartTime + triggerOffset) - minimumEventSpacing;
		}
		
		eventTime = events->first - holdoff; //we can put events closer together than this, but they don't happen until 2*us later
		
		if( (events->first - minimumEventSpacing) < previousTime)
		{
			if(events != eventsIn.begin())
				throw EventParsingException(events->second.at(0),
						"The Slow Analog Out board needs " + 
						valueToString(minimumEventSpacing) + 
						" ns between events.");
			else
				throw EventParsingException(events->second.at(0),
						"The Slow Analog Out board needs " + 
						valueToString(minimumAbsoluteStartTime) + 
						" ns at the beginning of triggered sequence. Trigger offset: " 
						+ valueToString(triggerOffset) + " ns.");

			//" ns at the beginning of the timing file.");
		}
	
		if(events->second.size() > 1)	//we only want one event per time
		{
			throw EventConflictException(events->second.at(0), 
				events->second.at(1), 
				"The Slow Analog Out cannot currently have multiple events at the same time." );
		}

		for(unsigned i = 0; i < events->second.size(); i++)
		{
			if(events->second.at(i).numberValue() > 10 || events->second.at(i).numberValue() < -10)
			{
				throw EventParsingException(events->second.at(i),
					"The Slow Analog Out board only supports voltages between -10 and 10 Volts.");
			}
		}

		eventsOut.push_back( 
			new SlowAnalogOutEvent(eventTime - triggerOffset, events->second, this) );

	}
}

stf_da_slow_device::SlowAnalogOutEvent::SlowAnalogOutEvent(
	double time, const std::vector<RawEvent>& sourceEvents, FPGA_Device* device)
	: FPGA_DynamicEvent(time, device)
{
	eventNotBeingConstructed = false;

	addSourceEvents(sourceEvents);

	uInt32 channelBits = sourceEvents.at(0).channel();
	bool reset = false;
	bool update = true;
	uInt32 registerBits = 3;

	//Most of the command bits can be set once and for all
	setBits(update, 14, 14); 
	setBits(flipChannelBits(channelBits), 15, 20);
	setBits(registerBits, 21, 22); 
	setBits(reset, 23, 23);

	//Use the updateValue function to set the actual voltage. This function gets called again
	//if the value is a DynamicValue and it gets changed remotely.
	updateValue(sourceEvents);		//For setting the voltage.

	eventNotBeingConstructed = true;
}

int stf_da_slow_device::SlowAnalogOutEvent::flipChannelBits(int channel)
{
	//Exchange lowest two bits
	int mask = 60;	//binary: 0b111100
	int newChan = (channel & mask) + ((2 & channel) >> 1) + ((1 & channel) << 1);
	return newChan;
}

void stf_da_slow_device::SlowAnalogOutEvent::updateValue(const std::vector<RawEvent>& sourceEvents)
{
	uInt32 voltageInt = static_cast<int>((-1*(sourceEvents.at(0).numberValue()) + 10.)*16383./20.);

	if(eventNotBeingConstructed) {
		cout << "SlowAnalogOutEvent::updateValue: " << sourceEvents.at(0).numberValue() << endl;
	}

	setBits(voltageInt, 0, 13); 
}
