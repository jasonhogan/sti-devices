
#include "DeviceHolder.h"

#include <ORBManager.h>

#include "PixisAddinDevice.h"

//#include <STI_Device_Adapter.h>
//class TestDevice : public STI_Device_Adapter
//{
//public:
//
//	TestDevice(ORBManager* orb_manager);
//
//	void stopWaiting();
//};
//
//
//
//TestDevice::TestDevice(ORBManager* orb_manager)
//	: STI_Device_Adapter(orb_manager, "TestDevice", "localhost", 0)
//{
//}
//
//void TestDevice::stopWaiting()
//{
//
//}


//////////////////////////////


DeviceHolder::DeviceHolder()
{
	int argc = 1;
	char* argv[] = { "" };

	orb_manager = new ORBManager(argc, argv);
	
//	orb_manager = std::make_shared<ORBManager>(argc, argv);

//	testDevice = std::make_shared<TestDevice>(orb_manager);
	pixisDevice = std::make_shared<PixisAddinDevice>(orb_manager);
}

__declspec(dllexport) void DeviceHolder::startDevice()
{
//	callBack(24);

//	orb_manager->run();
	pixisDevice->lightfield.go();
}

__declspec(dllexport) void DeviceHolder::stopWaiting()
{
	//if (testDevice != 0) {
	//	testDevice->stopWaiting();
	//}
}

__declspec(dllexport) void DeviceHolder::installCB(TESTCB cb)
{
	callBack = cb;
}

__declspec(dllexport) void DeviceHolder::install_AQUIRECB(AQUIRECB cb)
{
//	aquireCB = cb;

	if (pixisDevice != 0) {
//		pixisDevice->lightfield.install_AQUIRECB(cb);
		pixisDevice->lightfield.install_CB(cb);
	}
}

__declspec(dllexport) void DeviceHolder::install_STOPCB(STOPCB cb)
{
	//	aquireCB = cb;

	if (pixisDevice != 0) {
		//		pixisDevice->lightfield.install_AQUIRECB(cb);
		pixisDevice->lightfield.install_CB2(cb);
//		pixisDevice->lightfield.install_CB(GOCB(cb));

	}

//	GO2CB tmp;
//	GO2 go2(tmp);


}

//__declspec(dllexport) void DeviceHolder::install_CB(Callback::Go cb)
//{
//	pixisDevice->lightfield.install_CB(cb);
//}


template<typename CB> void DeviceHolder::install(CB cb)
{
	if (pixisDevice != 0) {
		pixisDevice->lightfield.install_CB(cb);
	}
}