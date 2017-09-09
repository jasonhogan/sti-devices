
#include "DeviceWrapper.h"

using namespace STI;



DeviceWrapper::DeviceWrapper()
{
	device = new DeviceHolder();
}

DeviceWrapper::~DeviceWrapper()
{
	if (nullptr != device)
	{
		delete device;
		device = nullptr;
	}
}

void DeviceWrapper::start()
{
	if (nullptr != device)
	{
		device->startDevice();
	}
}

void DeviceWrapper::installDelegate(TestDelegate^ del)
{
	testDelegate = del;
//	GetTheAnswerDelegate^ fp = gcnew GetTheAnswerDelegate(GetNumber);
	
//	GCHandle gch = GCHandle::Alloc(testDelegate);
	testgch = GCHandle::Alloc(testDelegate);
	IntPtr ip = Marshal::GetFunctionPointerForDelegate(testDelegate);
	
	TESTCB cb = static_cast<TESTCB>(ip.ToPointer());

	device->installCB(cb);
}

void DeviceWrapper::installAquireDelegate(AquireDelegate^ del)
{
	aquireDelegate = del;
	aquire_gch = GCHandle::Alloc(aquireDelegate);
	
	IntPtr ip = Marshal::GetFunctionPointerForDelegate(aquireDelegate);

	AQUIRECB cb = static_cast<AQUIRECB>(ip.ToPointer());

	device->install_AQUIRECB(cb);
}


//void A::func()
//{
//}
//
//void DeviceWrapper::installA(A^ a)
//{
//	myA = a;
//	agch = GCHandle::Alloc(myA, GCHandleType::Normal);
//	IntPtr ip = GCHandle::ToIntPtr(agch);
//
//	TESTCB cb = static_cast<A*>(ip.ToPointer());
//
//
////	IntPtr ip = Marshal::GetFunctionPointerForDelegate(myA);
//}