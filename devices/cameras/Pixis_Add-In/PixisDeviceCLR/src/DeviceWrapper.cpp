
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

void DeviceWrapper::startDevice()
{
	if (nullptr != device)
	{
		device->startDevice();
	}
}

