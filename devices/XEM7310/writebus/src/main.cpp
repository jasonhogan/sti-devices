
#include <iostream>

#include "okFrontPanelDLL.h"

#include "frontpanelsupport.h"

int main(int argc, char* argv[])
{

	// Initialize the FPGA with our configuration bitfile.
	OpalKelly::FrontPanelPtr dev = initializeFPGA("Counters.bit");
	if (!dev.get()) {
		printf("FPGA could not be initialized.\n");
		return -1;
	}

	return 0;
}

