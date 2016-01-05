/*! \file VCO_Devices.cpp
 *  \author Jason Michael Hogan
 *  \brief main() entry point for ADF4360_Device (for controling VCOs)
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

#include <string>
#include <iostream>

#include <ORBManager.h>
#include "ADF4360_Device.h"

using namespace std;


ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);    

	unsigned short module = 0;

	string ipAddress = "eplittletable.stanford.edu";

//*** Example VCO that uses Etrax (for hard timing):
//	unsigned int memAddress = 0x90000068;
//	Analog_Devices_VCO::ADF4360_Device adf4360_0_ch0(
//		orbManager, "ADF4360-0", ipAddress, module, 0, memAddress, 0);

	//Using the parallel port for communication:

	//"ADF4360-0" on VCO channel 0
	Analog_Devices_VCO::ADF4360_Device adf4360_0_ch0(
		orbManager, "ADF4360-0", ipAddress, module, 0, 0);

	//"ADF4360-5" on VCO channel 1
	Analog_Devices_VCO::ADF4360_Device adf4360_5_ch1(
		orbManager, "ADF4360-5", ipAddress, module + 1, 1, 5);

	//"ADF4360-4" on VCO channel 2
	Analog_Devices_VCO::ADF4360_Device adf4360_4_ch2(
		orbManager, "ADF4360-4", ipAddress, module + 2, 2, 4);

	//"ADF4360-6" on VCO channel 3
	Analog_Devices_VCO::ADF4360_Device adf4360_6_ch3(
		orbManager, "ADF4360-6", ipAddress, module + 3, 3, 6);

	
	adf4360_0_ch0.setPCParallelAddress(0x0258); //for the Star-Tech parallel-pci port //for LAVA use 0xEC00 //for legacy parallel ports that use the standard 0x378 address
	adf4360_5_ch1.setPCParallelAddress(0x0258);
	adf4360_4_ch2.setPCParallelAddress(0x0258);
	adf4360_6_ch3.setPCParallelAddress(0x0258);


	adf4360_0_ch0.setMuteTillLockDetect(true);
	//adf4360_5_ch1.setMuteTillLockDetect(true);
	//adf4360_5_ch2.setMuteTillLockDetect(true);
	adf4360_6_ch3.setMuteTillLockDetect(true);

	orbManager->run();

	return 0;
}

