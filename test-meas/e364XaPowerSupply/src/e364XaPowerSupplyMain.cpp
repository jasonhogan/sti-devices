/*! \file usb1408fsMain.cpp
 *  \author David M.S. Johnson
 *  \brief main()
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

#include <string>
#include <iostream>

#include <ORBManager.h>
#include "e364XaPowerSupplyDevice.h"


using namespace std;


ORBManager* orbManager;

int main(int argc, char **argv)
{
	orbManager = new ORBManager(argc, argv);    

	//Using the Module number as the COM port number.  The COM port is assigned automatically
	//(by Windows?) and could change if the supplies are plugged into different USB port
	//or possibly even when the computer reboots...

//	e364XaPowerSupplyDevice supply3D_Y_L(orbManager, "E3642a 3D:Y:L", "171.64.56.96", 5);
//	e364XaPowerSupplyDevice supply3D_Y_R(orbManager, "E3642a 3D:Y:R", "171.64.56.96", 10);
//
//	e364XaPowerSupplyDevice supply3D_X_L(orbManager, "E3642a 3D:X:L", "171.64.56.96", 8);
//	e364XaPowerSupplyDevice supply3D_X_R(orbManager, "E3642a 3D:X:R", "171.64.56.96", 9);

///////////////////

	string configFilename = "e364XaPowerSupply.ini"; //default
	ConfigFile configFile(configFilename);

	string IPaddress = "*Missing IP Address*";	//default
	configFile.getParameter("IP Address", IPaddress);

	std::string deviceName = "";
	unsigned short module = 0;
	unsigned short COMport = 0;

	configFile.getParameter("DeviceName:Y:L", deviceName);
	configFile.getParameter("Module:Y:L", module);
	configFile.getParameter("COM:Y:L", COMport);
	e364XaPowerSupplyDevice supply3D_Y_L(orbManager, deviceName, IPaddress, module, COMport);

	configFile.getParameter("DeviceName:Y:R", deviceName);
	configFile.getParameter("Module:Y:R", module);
	configFile.getParameter("COM:Y:R", COMport);
	e364XaPowerSupplyDevice supply3D_Y_R(orbManager, deviceName, IPaddress, module, COMport);

	configFile.getParameter("DeviceName:X:L", deviceName);
	configFile.getParameter("Module:X:L", module);
	configFile.getParameter("COM:X:L", COMport);
	e364XaPowerSupplyDevice supply3D_X_L(orbManager, deviceName, IPaddress, module, COMport);

	configFile.getParameter("DeviceName:X:R", deviceName);
	configFile.getParameter("Module:X:R", module);
	configFile.getParameter("COM:X:R", COMport);
	e364XaPowerSupplyDevice supply3D_X_R(orbManager, deviceName, IPaddress, module, COMport);

	orbManager->run();

	return 0;
}

