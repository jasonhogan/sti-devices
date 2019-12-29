
#include "smcs_cpp/CameraSDK.h"
#include <iostream>

#include <string>
#include <ORBManager.h>
#include "SmartekDevice.h"

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "SmartekCamera.ini"; //default

	ConfigFile configFile(configFilename);

	// initialize Camera API
	smcs::InitCameraAPI();
	smcs::ICameraAPI smcsApi = smcs::GetCameraAPI();

	if (!smcsApi->IsUsingKernelDriver()) {
		std::cout << "Warning: Smartek Filter Driver not loaded." << std::endl;
	}

	smcsApi->SetHeartbeatTime(3);

	// discover all devices on network
	smcsApi->FindAllDevices(3.0);
	const smcs::DevicesList devices = smcsApi->GetAllDevices();

	if (devices.size() == 0) {
		std::cout << "No cameras found." << std::endl;
		return 0;
	}

	smcs::IDevice device = devices.at(0);

	if (!device->IsConnected()) {	
		device->Connect();
	}

	SmartekDevice smartekCamera(orbManager, configFile, devices.at(0));
	smartekCamera.setSaveAttributesToFile(true);

	orbManager->run();	//Blocks while device is alive


	if (device->IsConnected()) {
		device->Disconnect();
	}
	smcs::ExitCameraAPI();

	return 0;
}
