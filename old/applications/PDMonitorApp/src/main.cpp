/*! \file main.cpp
 *  \author Susannah Dickerson 
 *  \brief main file for PDMonitorDevice
 *  \section license License
 *
 *  Copyright (C) 2010 Susannah M Dickerson <sdickers@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
 *	
 *	This structure shamlessly derived from source code originally by Jason
 *	Hogan <hogan@stanford.edu>
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
#include "PDMonitorDevice.h"

using namespace std;

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);    

	//Get path of config file
	/*
	boost::filesystem::path abs_path = boost::filesystem::complete("../../PDMonitorApp/src/PDMonitorApp.ini");
	abs_path = abs_path.normalize();
	std::string appConfigFile = abs_path.native_file_string();

	abs_path = boost::filesystem::complete("../../PDMonitorApp/src/PDMonitorServer.ini");
	abs_path = abs_path.normalize();
	std::string configFile = abs_path.native_file_string();
	*/

//	string ipAddress = "ep-timing1.stanford.edu";
	string configFile = "C:\\STI SVN\\sti\\applications\\PDMonitorApp\\src\\PDMonitorServer.ini";
	string appConfigFile = "C:\\STI SVN\\sti\\applications\\PDMonitorApp\\src\\PDMonitorApp.ini";

	PDMonitorDevice pdMonitorDevice(orbManager, "PD Monitor", configFile, appConfigFile); // last argument is com port #

	if (pdMonitorDevice.initialized) {
		orbManager->run();
	} else {
		std::cerr << "Error initializing PD Monitor" << std::endl;
	}

	return 0;
}

