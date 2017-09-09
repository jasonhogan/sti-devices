#pragma once

#include "DeviceHolder.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace STI {

public delegate int TestDelegate(int);
public delegate void AquireDelegate(void);


//public ref class A
//{
//	void func();
//};

public ref class DeviceWrapper
{
public:

	DeviceHolder* device;

	DeviceWrapper();
	~DeviceWrapper();

	void start();

	void installDelegate(TestDelegate^ del);

	void installAquireDelegate(AquireDelegate^ del);

	TestDelegate^ testDelegate;
	GCHandle testgch;

	AquireDelegate^ aquireDelegate;
	GCHandle aquire_gch;


//	void installA(A^ a);
//	A^ myA;
//	GCHandle agch;
};

};