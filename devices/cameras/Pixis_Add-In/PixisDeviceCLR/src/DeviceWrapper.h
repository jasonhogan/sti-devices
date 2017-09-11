#pragma once

//C++/CLI adapter class.

#include "DeviceHolder.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace STI {

public delegate int TestDelegate(int);
public delegate void AquireDelegate(void);
public delegate void StopDelegate(void);
public delegate void GoDelegate(void);


public ref class DeviceWrapper
{
public:

	DeviceHolder* device;

	DeviceWrapper();
	~DeviceWrapper();

	void startDevice();

	//Install delegates
	void installDelegate(TestDelegate^ del)		{ _installDelegate<Callback::Test>(del); }
	void installDelegate(AquireDelegate^ del)	{ _installDelegate<Callback::Aquire>(del); }
	void installDelegate(GoDelegate^ del)		{ _installDelegate<Callback::Go>(del); }
	void installDelegate(StopDelegate^ del)		{ _installDelegate<Callback::Stop>(del); }


private:

	template<typename CB>
	void _installDelegate(Delegate^ del)
	{
		delegates.Add(del);
		GCHandle gch = GCHandle::Alloc(del);
		handles.Add(gch);

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(del);

		//Wrap pointer in callback wrapper class for type CB.  Then install_CB calls correct overload.
		device->install_CB(
			CB(
				static_cast<CB::Func>(ip.ToPointer())));
	}


	//Save references to prevent unwanted garbage collection
	List<Delegate^> delegates;
	List<GCHandle> handles;

};

};
