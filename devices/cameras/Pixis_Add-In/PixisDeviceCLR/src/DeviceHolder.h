#ifndef DEVICE_H
#define DEVICE_H

#include <memory>

#include "callbacks.h"

class PixisAddinDevice;
class ORBManager;


class DeviceHolder
{
public:
	
	__declspec(dllexport) DeviceHolder();

	__declspec(dllexport) void startDevice();
	__declspec(dllexport) void stopWaiting();

	//Install callbacks
	__declspec(dllexport) void install_CB(Callback::Test cb) { install<Callback::Test>(cb); }
	__declspec(dllexport) void install_CB(Callback::Aquire cb) { install<Callback::Aquire>(cb); }
	__declspec(dllexport) void install_CB(Callback::Go cb) { install<Callback::Go>(cb); }
	__declspec(dllexport) void install_CB(Callback::Stop cb) { install<Callback::Stop>(cb); }

private:

	//Generic install
	template<typename CB> void install(CB cb);

	std::shared_ptr<PixisAddinDevice> pixisDevice;
	ORBManager* orb_manager;

};

#endif
