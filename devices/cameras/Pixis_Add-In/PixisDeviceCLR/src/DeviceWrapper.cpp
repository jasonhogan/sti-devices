
//#include <windows.h>
//#include <winnls.h>

#define WC_NO_BEST_FIT_CHARS      0x00000400  // do not use best fit chars
#include <msclr\marshal_cppstd.h>

#include "DeviceWrapper.h"



using namespace STI;


DeviceWrapper::DeviceWrapper()
{
	device = new DeviceHolder();
}

DeviceWrapper::~DeviceWrapper()
{
//	device->shutdown();

	if (nullptr != device)
	{
		delete device;
		device = nullptr;
	}
}

void DeviceWrapper::shutdown()
{
	if (nullptr != device)
	{
		device->shutdown();
	}
}


void DeviceWrapper::startDevice()
{
	if (nullptr != device)
	{
		device->startDevice();
	}
}
void DeviceWrapper::stopWaiting(int index)
{
	if (nullptr != device)
	{
		device->stopWaiting(index);
	}
}

void DeviceWrapper::setSavedImageFilename(int index, String^ filename)
{
	if (nullptr != device)
	{
		std::string unmanaged_filename = msclr::interop::marshal_as<std::string>(filename);
		device->setSavedImageFilename(index, unmanaged_filename);
	}
}
void DeviceWrapper::setSavedSPEFilename(int index, String^ filename)
{
	if (nullptr != device)
	{
		std::string unmanaged_filename = msclr::interop::marshal_as<std::string>(filename);
		device->setSavedSPEFilename(index, unmanaged_filename);
	}
}
