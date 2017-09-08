#ifndef DEVICE_H
#define DEVICE_H

#include <memory>

class TestDevice;
class ORBManager;

class DeviceHolder
{
public:

	std::shared_ptr<TestDevice> testDevice;

//	std::shared_ptr<ORBManager> orb_manager;

	ORBManager* orb_manager;

	__declspec(dllexport) DeviceHolder();

	__declspec(dllexport) void startDevice();

	__declspec(dllexport) void stopWaiting();
};

#endif

