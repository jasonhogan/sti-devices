#ifndef DEVICE_H
#define DEVICE_H

#include <memory>

#include "callbacks.h"

class PixisAddinDevice;
//class TestDevice;
class ORBManager;

//#include "PixisAddinDevice.h"

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
	__declspec(dllexport) void install_STOPCB(STOPCB cb);


	//	__declspec(dllexport) void install_CB(Callback::Go cb);
	__declspec(dllexport) void install_CB(Callback::Go cb) { install<Callback::Go>(cb); }


	template<typename CB> void install(CB cb);
	//{
	//	if (pixisDevice != 0) {
	//		pixisDevice->lightfield.install_CB(cb);
	//	}
	//}



	// void install_STOPCB(STOPCB cb);

	//template<typename T> void install_CB(T cb);


	//template<> __declspec(dllexport) void install_CB<STOPCB>(STOPCB cb)
	//{
	//	install_CB(cb);
	//}

	//Callbacks
	TESTCB callBack;
	//	AQUIRECB aquireCB;
};

#endif

