#include <ORBManager.h>
#include "STI_Device_Adapter.h"

class TestDevice : public STI_Device_Adapter
{
public:

	TestDevice(ORBManager* orb_manager, std::string configFilename)
		: STI_Device_Adapter(orb_manager, configFilename) {}

};

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "testDevice.ini";

	TestDevice test(orbManager, configFilename);

	orbManager->run();

	return 0;
}


