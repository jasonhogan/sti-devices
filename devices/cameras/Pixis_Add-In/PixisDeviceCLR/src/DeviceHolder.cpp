
#include "DeviceHolder.h"
#include <ORBManager.h>
#include "PixisAddinDevice.h"

DeviceHolder::DeviceHolder()
{
	orb_manager = 0;
	setupORB();
}

__declspec(dllexport) void DeviceHolder::setupORB()
{
	int argc = 1;
	char* argv[] = { "" };

	orb_manager = new ORBManager(argc, argv);

	ConfigFile configFile("sti_pixis_addin.ini");		//Default dir apparently is C:\Windows\System32 when loading from dll

	pixisDevice = std::make_shared<PixisAddinDevice>(orb_manager, configFile);
}

__declspec(dllexport) DeviceHolder::~DeviceHolder()
{
	if (pixisDevice != 0) {
		pixisDevice->deviceShutdown();
	}

	if (orb_manager != 0) {
		orb_manager->ORBshutdown();
		delete orb_manager;
		orb_manager = 0;
	}
}

__declspec(dllexport) void DeviceHolder::startDevice()
{
//	pixisDevice->lightfield.aquire(73);

	orb_manager->run();
//	pixisDevice->lightfield.go();
//	pixisDevice->lightfield.stop();
//	pixisDevice->lightfield.test(57);

}

__declspec(dllexport) void DeviceHolder::stopWaiting(int index)
{
	if (pixisDevice != 0) {
		pixisDevice->stopWaiting(index);
	}
}

__declspec(dllexport) void DeviceHolder::shutdown()
{
	if (pixisDevice != 0) {
		pixisDevice->deviceShutdown();
	}
	
	if (orb_manager != 0) {
		orb_manager->ORBshutdown();
		delete orb_manager;
		orb_manager = 0;
	}

	setupORB();	//prepare for next startDevice() call
}

__declspec(dllexport) void DeviceHolder::setSavedImageFilename(int index, const std::string& filename)
{
	if (pixisDevice != 0) {
		pixisDevice->setSavedImageFilename(index, filename);
	}
}

__declspec(dllexport) void DeviceHolder::setSavedSPEFilename(int index, const std::string& filename)
{
	if (pixisDevice != 0) {
		pixisDevice->setSavedSPEFilename(index, filename);
	}
}


template<typename CB> void DeviceHolder::install(CB cb)
{
	if (pixisDevice != 0) {
		pixisDevice->lightfield.install_CB(cb);
	}
}
