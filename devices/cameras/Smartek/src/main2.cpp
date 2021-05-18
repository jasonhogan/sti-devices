

#include <string>
#include <ORBManager.h>
//#include "SmartekDevice.h"


#include <STI_Device_Adapter.h>


class SmartekDevice : public STI_Device_Adapter
{
public:

	SmartekDevice(ORBManager* orb_manager, std::string configFilename) 
		: STI_Device_Adapter(orb_manager, configFilename) {}
};

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "SmartekCamera.ini"; //default
	
	SmartekDevice smartekCamera(orbManager, configFilename);


	orbManager->run();	//Blocks while device is alive
	
	return 0;
}