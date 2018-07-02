
#include "DeviceHolder.h"
#include <ORBManager.h>
#include "PixisAddinDevice.h"

DeviceHolder::DeviceHolder(int module) : module(module)
{
	orb_manager = 0;
	setupORB();	//DEBUG
}

__declspec(dllexport) void DeviceHolder::setupORB()
{

//	int argc = 1;
//	char* argv[] = { "" };

	argc = 1;
	argv = new char*[argc];
	argv[0] = new char[10];
	strcpy(argv[0], "");

	orb_manager = new ORBManager(argc, argv);

	//Not working anymore for some reason (JMH 7/1/2018)
	//ConfigFile configFile("sti_pixis_addin.ini");		//Default dir apparently is C:\Windows\System32 when loading from dll
//	configFile = std::make_shared<ConfigFile>("sti_pixis_addin.ini");
	
	std::string config_dir = "C:\\Program Files\\Princeton Instruments\\LightField\\AddIns\\STI_Pixis\\";
//	configFile = std::make_shared<ConfigFile>(config_dir + "sti_pixis_addin.ini");

	switch (module)
	{
	case 0:
		configFile = std::make_shared<ConfigFile>(config_dir + "sti_pixis_north.ini");
		break;
	case 1:
		configFile = std::make_shared<ConfigFile>(config_dir + "sti_pixis_east.ini");
		break;
	}

	pixisDevice = std::make_shared<PixisAddinDevice>(orb_manager, (*configFile) );
//	pixisDevice = std::make_shared<PixisAddinDevice>(orb_manager);	//for debugging w/o config file
}

__declspec(dllexport) DeviceHolder::~DeviceHolder()
{
//	if (pixisDevice != 0) {
//		pixisDevice->deviceShutdown();
//	}

	if (orb_manager != 0) {
		orb_manager->ORBshutdown();
		delete orb_manager;
		orb_manager = 0;
	}

	// cleanup
	for (int i = 0; i < argc; i++)
		delete[] argv[i];
	delete[] argv;
}

__declspec(dllexport) void DeviceHolder::startDevice()
{
//	pixisDevice->lightfield.aquire(73);

//	pixisDevice->print("Test print!\n");  //DEBUG
//	pixisDevice->print((configFile->isParsed() ? "isParsed()=1" : "isParsed()=0")); //DEBUG
//	pixisDevice->print("config: " + configFile->printParameters());  //DEBUG


	if (orb_manager != 0) {
		orb_manager->run();
	}


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
//	if (pixisDevice != 0) {
//		pixisDevice->deviceShutdown();
//	}
	
	//pixisDevice->lightfield.
	

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
