#include <ORBManager.h>
#include "STI_Device_Adapter.h"

class TestDevice : public STI_Device_Adapter
{
public:

	TestDevice(ORBManager* orb_manager, std::string configFilename)
		: STI_Device_Adapter(orb_manager, configFilename) {}

	void defineChannels() 
	{
		addInputChannel(0, TData::DataDouble, TValue::ValueNumber, "in");
		addOutputChannel(1, TValue::ValueNumber,"out");
	}
	bool readChannel(unsigned short channel, const MixedValue& valueIn, MixedData& dataOut) 
	{
		if (channel == 0) {
			dataOut.setValue(2 * valueIn.getNumber());
		}
		return true;
	}
	bool writeChannel(unsigned short channel, const MixedValue& value)
	{
		if (channel == 1) {
			std::cout << "write: " << value.getNumber() << std::endl;
		}
		return true; 
	}


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


