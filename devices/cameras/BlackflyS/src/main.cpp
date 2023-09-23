
//#include "smcs_cpp/CameraSDK.h"
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include <iostream>

#include <string>
#include <memory>
#include <sstream>

#include <ORBManager.h>
#include "BlackflyDevice.h"
#include "BlackflyConfigFile.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "BlackflyCamera.ini"; //default

	//ConfigFile configFile(configFilename);
	BlackflyConfigFile blackflyConfigFile(configFilename);
	std::map<std::string, std::string> cameraSerialMap;		// map<"serial number" : "config section">

	auto cameraConfigNames = blackflyConfigFile.getSectionNames();
	for (auto& sectionName : cameraConfigNames) {
		std::string serialNumber;
		if (blackflyConfigFile.getParameter(sectionName, "Serial Number", serialNumber)) {
			//found camera serial number in config file
			cameraSerialMap[serialNumber] = sectionName;
		}
	}

	SystemPtr system;	//Spinnaker system object
	
	try {
		// initialize camera API (Spinnaker)
		system = System::GetInstance();	// Retrieve singleton reference to system object
	}
	catch (Exception& e) {
		std::cout << "Failed to retrieve Spinnaker system object." << std::endl;
		std::cout << "Spinnaker exception: " << e.GetErrorMessage() << std::endl;
		return -1;
	}
	catch (...) {
		std::cout << "Failed to retrieve Spinnaker system object." << std::endl;
		std::cout << "Unknown spinnaker exception!" << std::endl;
		return -1;
	}

	if (system == 0) {
		std::cout << "Error: Spinnaker system object is null." << std::endl;
		return -1;
	}

	CameraList camList = system->GetCameras();	// Retrieve list of cameras from the system
	const unsigned int numCameras = camList.GetSize();
	
	// Finish if there are no cameras
	if (numCameras == 0)
	{
		camList.Clear();	// Clear camera list before releasing system
		system->ReleaseInstance();	// Release system

		std::cout << "No cameras found!" << std::endl;
		return -1;
	}


//	TransportLayerDevice tdev();
//	tdev.DeviceSerialNumber;

	std::vector<CameraPtr> pCams;
	for (unsigned i = 0; i < numCameras; ++i) {
		pCams.push_back(nullptr);	// Shared pointer; must be released manually *in this context* 
		pCams.at(i) = camList.GetByIndex(i);
		//pCams.at(i) = camList.GetBySerial("19075973");

//		std::string serialNum = pCams.at(i)->GetUniqueID();
//		std::cout << "*** Found camera serial # " << serialNum << std::endl;
	}
	
	for (unsigned i = 0; i < numCameras; ++i) {
		if (pCams.at(i) != 0) {
			pCams.at(i)->Init();

//			auto ser = pCams.at(i)->DeviceSerialNumber.ToString();
//			std::cout << "DeviceSerialNumber: " << ser << std::endl;
			std::cout << "*** Found camera serial # " << pCams.at(i)->DeviceSerialNumber.ToString() << std::endl;
		}
	}

	// Create an STI device for each camera found
	std::vector<std::unique_ptr<BlackflyDevice>> blackflyCameras;
	for (unsigned i = 0; i < numCameras; ++i) {
		
		//std::string serialNum = pCams.at(i)->GetUniqueID();
		std::string serialNum = pCams.at(i)->DeviceSerialNumber.ToString();
		auto it = cameraSerialMap.find(serialNum);
		
		if (it != cameraSerialMap.end()) {
			//This camera serial number appears in the config file; create device
			auto bfdevice = std::make_unique<BlackflyDevice>(orbManager,
				blackflyConfigFile.getConfigFile(it->second),
				pCams.at(i));

			bfdevice->setSaveAttributesToFile(true);

			blackflyCameras.push_back(std::move(bfdevice));
		}
	}

	//blackflyCameras.at(0)->printAllCameraNodes();

	orbManager->run();	//Blocks while devices are alive

	// Clean up
	for (unsigned i = 0; i < numCameras; ++i) {
		pCams.at(i)->DeInit();
		pCams.at(i) = nullptr;		// Needed to avoid exception from system->ReleaseInstance();
	}

	camList.Clear();	// Clear camera list before releasing system
	system->ReleaseInstance();	// Release system

	return 0;
}
