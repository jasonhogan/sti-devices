
//#include "smcs_cpp/CameraSDK.h"
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include <iostream>

#include <string>
#include <ORBManager.h>
//#include "BlackflyDevice.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "BlackflyCamera.ini"; //default

	//ConfigFile configFile(configFilename);

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
	CameraPtr pCam = nullptr;	// Shared pointer; must be released manually *in this context* 

	pCam = camList.GetByIndex(0);


//	BlackflyDevice blackflyCamera(orbManager, configFile, devices.at(0));
//	blackflyCamera.setSaveAttributesToFile(true);

//	orbManager->run();	//Blocks while device is alive


	// Clean up
	pCam = nullptr;		// Needed to avoid exception from system->ReleaseInstance();
	camList.Clear();	// Clear camera list before releasing system
	system->ReleaseInstance();	// Release system

	return 0;
}
