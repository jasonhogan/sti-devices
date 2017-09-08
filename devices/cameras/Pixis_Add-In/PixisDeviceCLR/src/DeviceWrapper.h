#pragma once

#include "DeviceHolder.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace STI {

public delegate int TestDelegate(int);

public ref class DeviceWrapper
{
public:

	DeviceHolder* device;

	DeviceWrapper();
	~DeviceWrapper();

	void start();

	void installDelegate(TestDelegate^ del);

	TestDelegate^ testDelegate;
	GCHandle testgch;
};

};