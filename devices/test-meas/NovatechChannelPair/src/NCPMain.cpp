/*! \file main.cpp
 *  \author Susannah Dickerson 
 *  \brief main file for NovatechChannelPair
 *  \section license License
 *
 *  Copyright (C) 2011 Susannah M Dickerson <sdickers@stanford.edu>\n
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
#include "NovatechChannelPair.h"

using namespace std;

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);    

	string ipAddress = "eplittletable.stanford.edu";

	NovatechChannelPair channelPair(orbManager, "Imaging Detuning", ipAddress, 0, 1, 2, "Big Table Novatech");

	if (channelPair.initialized) {
		orbManager->run();
	} else {
		std::cerr << "Error initializing NovatechChannelPair" << std::endl;
	}

	return 0;
}

