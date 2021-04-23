
#include <iostream>

#include "okFrontPanelDLL.h"

#include "frontpanelsupport.h"

int main(int argc, char* argv[])
{

	// Initialize the FPGA with our configuration bitfile.
	OpalKelly::FrontPanelPtr dev = initializeFPGA("stf_bus.bit");
	if (!dev.get()) {
		printf("FPGA could not be initialized.\n");
		return -1;
	}

	UINT32 value;

	while (true) {

		std::cout << "Input value: ";
		std::cin >> value;
		std::cout << std::endl;

		dev->SetWireInValue(0x01, value);
		dev->UpdateWireIns();
	
	}

	return 0;
}

