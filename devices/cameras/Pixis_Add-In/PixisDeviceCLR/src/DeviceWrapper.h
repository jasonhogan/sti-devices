#pragma once

#include "DeviceHolder.h"

using namespace System;
using namespace System::Runtime::InteropServices;

using namespace System::Collections::Generic;

namespace STI {

public delegate int TestDelegate(int);
public delegate void AquireDelegate(void);
public delegate void StopDelegate(void);
public delegate void GoDelegate(void);


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
	void installStopDelegate(StopDelegate^ del);

	void installGoDelegate(GoDelegate^ del);

	void installGoDelegate2(GoDelegate^ del) { installDelegate2<Callback::Go>(del); }

	IntPtr installDelegate(Delegate^ del);

	template<typename CB>
	void installDelegate2(Delegate^ del)
	{
		IntPtr ip = installDelegate(del);

		device->install_CB(
			CB(
				static_cast<CB::Func>(ip.ToPointer())));
	}


	TestDelegate^ testDelegate;
	GCHandle testgch;

	AquireDelegate^ aquireDelegate;
	GCHandle aquire_gch;

	StopDelegate^ stopDelegate;
	GCHandle stop_gch;

	List<Delegate^> delegates;
	List<GCHandle> handles;

//	void installA(A^ a);
//	A^ myA;
//	GCHandle agch;
};

};