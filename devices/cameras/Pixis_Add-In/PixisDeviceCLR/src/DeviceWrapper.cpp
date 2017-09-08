
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
