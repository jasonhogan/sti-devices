
#include "DeviceHolder.h"
#include <ORBManager.h>
#include "PixisAddinDevice.h"

DeviceHolder::DeviceHolder()
{
	int argc = 1;
	char* argv[] = { "" };

	orb_manager = new ORBManager(argc, argv);
	
	pixisDevice = std::make_shared<PixisAddinDevice>(orb_manager);
}

__declspec(dllexport) void DeviceHolder::startDevice()
{

//	orb_manager->run();
	pixisDevice->lightfield.go();
	pixisDevice->lightfield.stop();
	pixisDevice->lightfield.aquire();
	pixisDevice->lightfield.test(57);

}

__declspec(dllexport) void DeviceHolder::stopWaiting()
{
	//if (testDevice != 0) {
	//	testDevice->stopWaiting();
	//}
}

template<typename CB> void DeviceHolder::install(CB cb)
{
	if (pixisDevice != 0) {
		pixisDevice->lightfield.install_CB(cb);
	}
}
