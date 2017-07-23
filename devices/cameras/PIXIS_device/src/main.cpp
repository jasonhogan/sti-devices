/*! \file main.cpp
*  \author Jason Michael Hogan
*  \brief main() entry point for pixis device
*  \section license License
*
*  Copyright (C) 2017 Jason Hogan <hogan@stanford.edu>\n
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
#include <ORBManager.h>
#include "PixisDevice.h"

#include <Magick++.h>

ORBManager* orbManager;

void magickTest()
{
	std::list<Magick::Image> magickImages;

	Magick::Image image;
	std::vector<unsigned short> data = { 1, 2000, 15000, 4 };
	image.read(2, 2, "I", MagickCore::ShortPixel, data.data());	//is this a deep copy of the image data?  Could be slow.  JMH

	magickImages.push_back(image);
	//image.write("test.png");

	std::vector<unsigned short> data2 = { 2000, 1 , 4, 15000 };
	image.read(2, 2, "I", MagickCore::ShortPixel, data2.data());	//is this a deep copy of the image data?  Could be slow.  JMH

	magickImages.push_back(image);

	std::string filenametemp = "C:\\Users\\Jason\\Code\\dev\\epcamera\\pixis_device\\test.tif";
	Magick::writeImages(magickImages.begin(), magickImages.end(), filenametemp, true);

}

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);

	std::string configFilename = "pixisDevice.ini"; //default

	if (argc > 1)
	{
		configFilename = std::string(argv[1]);
	}

	//FPGA Trigger Device
	PixisDevice pixis(orbManager, configFilename);

//	pixis.camera.StartAcquisition();
//	pixis.camera.WaitForAcquisition();

	pixis.setSaveAttributesToFile(true);

	orbManager->run();

	return 0;
}

