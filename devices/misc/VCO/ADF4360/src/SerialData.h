/*! \file SerialData.h
 *  \author Jason Michael Hogan
 *  \brief Include-file for the class Analog_Devices_VCO::SerialData
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

#ifndef ANALOG_DEVICES_VCO_SERIALDATA_H
#define ANALOG_DEVICES_VCO_SERIALDATA_H

namespace Analog_Devices_VCO {

class SerialData 
{
public:
	
	SerialData();
	SerialData(bool clock, bool data, bool le);
	~SerialData();

	unsigned int getParallelData() const;	//PC parallel port
	unsigned int getData(unsigned int vcoAddress) const;
	bool getPin(unsigned pin, unsigned int vcoAddress);


	bool CLOCK;
	bool DATA;
	bool LE;


};

}

#endif


