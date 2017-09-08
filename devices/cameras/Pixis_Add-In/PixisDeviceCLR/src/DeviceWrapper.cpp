
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
