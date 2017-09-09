#ifndef DEVICE_H
#define DEVICE_H

#include <memory>

#include "callbacks.h"

class PixisAddinDevice;
//class TestDevice;
class ORBManager;



class DeviceHolder
{
public:

	std::shared_ptr<PixisAddinDevice> pixisDevice;
//	std::shared_ptr<TestDevice> testDevice;

	ORBManager* orb_manager;

	__declspec(dllexport) DeviceHolder();

	__declspec(dllexport) void startDevice();

	__declspec(dllexport) void stopWaiting();

	//Install callbacks
	__declspec(dllexport) void installCB(TESTCB cb);
	__declspec(dllexport) void install_AQUIRECB(AQUIRECB cb);

	//Callbacks
	TESTCB callBack;
//	AQUIRECB aquireCB;
};

#endif

