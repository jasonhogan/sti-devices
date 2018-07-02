#ifndef DEVICE_H
#define DEVICE_H

#include <memory>
#include <string>

#include "callbacks.h"

#include "ConfigFile.h"

class PixisAddinDevice;
class ORBManager;


class DeviceHolder
{
public:
	
	__declspec(dllexport) DeviceHolder(int module);
	__declspec(dllexport) ~DeviceHolder();

	__declspec(dllexport) void setupORB();

	__declspec(dllexport) void setSavedImageFilename(int index, const std::string& filename);
	__declspec(dllexport) void setSavedSPEFilename(int index, const std::string& filename);
	__declspec(dllexport) void stopWaiting(int index);

	__declspec(dllexport) void startDevice();

	__declspec(dllexport) void shutdown();


	//Install callbacks
	__declspec(dllexport) void install_CB(Callback::Test cb) { install<Callback::Test>(cb); }
	__declspec(dllexport) void install_CB(Callback::Aquire cb) { install<Callback::Aquire>(cb); }
	__declspec(dllexport) void install_CB(Callback::Go cb) { install<Callback::Go>(cb); }
	__declspec(dllexport) void install_CB(Callback::Stop cb) { install<Callback::Stop>(cb); }

	__declspec(dllexport) void install_CB(Callback::SetSaveDir cb) { install<Callback::SetSaveDir>(cb); }
	__declspec(dllexport) void install_CB(Callback::Print cb) { install<Callback::Print>(cb); }

	__declspec(dllexport) void install_CB(Callback::ExternalTriggerOn cb) { install<Callback::ExternalTriggerOn>(cb); }
	__declspec(dllexport) void install_CB(Callback::IsReadyToAquire cb) { install<Callback::IsReadyToAquire>(cb); }
	__declspec(dllexport) void install_CB(Callback::ClearImageCount cb) { install<Callback::ClearImageCount>(cb); }
	__declspec(dllexport) void install_CB(Callback::IncrementImageCount cb) { install<Callback::IncrementImageCount>(cb); }

private:

	//Generic install
	template<typename CB> void install(CB cb);

	int module;		//For distinguishing North vs East camera

	std::shared_ptr<ConfigFile> configFile;

	std::shared_ptr<PixisAddinDevice> pixisDevice;
	ORBManager* orb_manager;

	int argc;
	char** argv;

};

#endif
