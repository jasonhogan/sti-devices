/*! \file PixisDevice.h
*  \author Jason Hogan
*  \brief header file for PixisDevice
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

#ifndef PIXISDEVICE_H
#define PIXISDEVICE_H


#include <STI_Device_Adapter.h>
#include "PixisCamera.h"
#include "Image.h"
#include "ImageWriter.h"

#include <memory>

struct PixisDeviceEventValue
{
	std::string baseFilename;
	std::string description;
	std::vector<int> cropVector;
};


class PixisDevice : public STI_Device_Adapter
{
public:

	PixisDevice(ORBManager* orb_manager, std::string configFilename);
	
	void defineAttributes();
	bool updateAttribute(std::string key, std::string value);
	void refreshAttributes();

	void defineChannels();

	void parseDeviceEvents(const RawEventMap& eventsIn, SynchronousEventVector& eventsOut) throw(std::exception);

	void definePartnerDevices();

	std::string getDeviceHelp();

	PixisCamera camera;

private:


	bool parseEventValue(const std::vector<MixedValue>& tuple, PixisDeviceEventValue& value, std::string& message);


//	ImageWriter imageWriter;

	class PixisEvent : public SynchronousEvent
	{
	public:

		PixisEvent(double time, PixisDevice* cameraDevice_, int index, const shared_ptr<Image>& image, const shared_ptr<ImageWriter>& imageWriter)
			: SynchronousEvent(time, cameraDevice_), cameraDevice(cameraDevice_), 
			imageIndex(index), image(image), imageWriter(imageWriter)
		{
			image->imageData.reserve(image->getImageSize());	//reserve memory for image data
		}

		void reset();	//override
		void stop();	//override

		void setupEvent() { }
		void loadEvent();
		void playEvent();
		void collectMeasurementData();

		void waitBeforeCollectData();

		PixisDevice* cameraDevice;

		shared_ptr<Image> image;
		shared_ptr<ImageWriter> imageWriter;

	private:
		int imageIndex;

	};


};

#endif
