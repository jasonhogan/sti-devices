
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

	// initialize Camera API
	SystemPtr system = System::GetInstance();	// Retrieve singleton reference to system object

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

//	CameraPtr pCam = nullptr;	// Shared pointer; must be released manually *in this context* 
//	pCam = camList.GetByIndex(0);

	std::vector<CameraPtr> pCams;
	for (unsigned i = 0; i < numCameras; ++i) {
		pCams.push_back(nullptr);	// Shared pointer; must be released manually *in this context* 
		pCams.at(i) = camList.GetByIndex(i);
		//pCams.at(i) = camList.GetBySerial("19075973");
		std::string serialNum = pCams.at(i)->GetUniqueID();
		std::cout << "*** Found camera serial # " << serialNum << std::endl;
	}
	
	//pCam->Init();
	for (unsigned i = 0; i < numCameras; ++i) {
		if (pCams.at(i) != 0) {
			pCams.at(i)->Init();
		}
	}

//	CIntegerPtr ptrWidth = pCam->GetNodeMap().GetNode("Width");
	//Set width of 640 pixels
//	ptrWidth->SetValue(640);

	//Spinnaker::GenApi::NodeList_t nodelist;
	//INodeMap& appLayerNodeMap = pCam->GetNodeMap(); // .GetNodes(nodelist);

//	BlackflyDevice blackflyCamera(orbManager, blackflyConfigFile.getConfigFile("Blackfly 1"), pCam);
//	blackflyCamera.setSaveAttributesToFile(true);

	std::vector<std::unique_ptr<BlackflyDevice>> blackflyCameras;
	for (unsigned i = 0; i < numCameras; ++i) {
		
		std::string serialNum = pCams.at(i)->GetUniqueID();
		auto it = cameraSerialMap.find(serialNum);
		
		if (it != cameraSerialMap.end()) {
			//This camera serial number appears in the config file; create device
			auto bfdevice = std::make_unique<BlackflyDevice>(orbManager,
				blackflyConfigFile.getConfigFile(it->second),
				pCams.at(i));

			bfdevice->setSaveAttributesToFile(true);

			blackflyCameras.push_back(std::move(bfdevice));
		}

		//cameraSerialMap
//		std::stringstream configSectionName;
//		configSectionName << "Blackfly " << i;

	}

	orbManager->run();	//Blocks while device is alive


	// Clean up
//	pCam->DeInit();
//	pCam = nullptr;		// Needed to avoid exception from system->ReleaseInstance();

	// Clean up
	for (unsigned i = 0; i < numCameras; ++i) {
		pCams.at(i)->DeInit();
		pCams.at(i) = nullptr;		// Needed to avoid exception from system->ReleaseInstance();
	}

	camList.Clear();	// Clear camera list before releasing system
	system->ReleaseInstance();	// Release system

	return 0;
}
