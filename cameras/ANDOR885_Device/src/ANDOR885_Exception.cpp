/*! \file EventParsingException.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class EventParsingException
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

#include "ANDOR885_Exception.h"

ANDOR885_Exception::ANDOR885_Exception(std::string message) :
message_l(message)
{
}


ANDOR885_Exception::~ANDOR885_Exception() throw()
{
}


std::string ANDOR885_Exception::printMessage() const
{
	return message_l;
}

