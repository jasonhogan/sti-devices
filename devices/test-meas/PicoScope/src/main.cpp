
#include <ORBManager.h>

#include "PicoScope5000Device.h"

ORBManager* orbManager;

int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);
	std::string configFilename = "PicoScope5000.ini"; //default

	ConfigFile configFile(configFilename);

	PicoScope5000Device picoscope(orbManager, configFilename);
	picoscope.setSaveAttributesToFile(true);

	orbManager->run();	//Blocks while device is alive

	return 0;
}
