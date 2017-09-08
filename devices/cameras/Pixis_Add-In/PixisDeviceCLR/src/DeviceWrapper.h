#pragma once

#include "DeviceHolder.h"

namespace STI {



public ref class DeviceWrapper
{
public:

	DeviceHolder* device;

	DeviceWrapper();
	~DeviceWrapper();

	void start();

};

};