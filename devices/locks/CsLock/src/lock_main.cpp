/*! \file lock_main.cpp
 *  \author Jason Michael Hogan
 *  \brief main() source-file for the lock device program
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
#include "lockDevice.h"

using namespace std;


ORBManager* orbManager;

int main(int argc, char **argv)
{
	orbManager = new ORBManager(argc, argv);    

	//Configuration file
	std::string configFilename = "lockDevice.ini"; //default

	if(argc > 1) {
		configFilename = string( argv[1] );
	}

	unsigned short module = 1;
	//cerr << "Enter module: " << endl;
	//cin >> module;

	//"LVDS Cs Lock board"
	lockDevice lockdevice(orbManager, "Lock", "171.64.56.254", module, configFilename);

	orbManager->run();
	
	return 0;
}

