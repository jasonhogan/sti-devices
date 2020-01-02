
#include <iostream>

#include "okFrontPanelDLL.h"

#include "frontpanelsupport.h"

int main(int argc, char* argv[])
{

	// Initialize the FPGA with our configuration bitfile.
	OpalKelly::FrontPanelPtr dev = initializeFPGA("writereg_top.bit");
	if (!dev.get()) {
		printf("FPGA could not be initialized.\n");
		return -1;
	}

	UINT32 value;
	okTRegisterEntry reg;

	UINT32* outdata;

	reg.address = 0x01;
	reg.data = 0x0000;

	outdata = &(reg.data);

	bool fixedAddr = false;

	while (fixedAddr) {

		std::cout << "Input value: ";
		std::cin >> value;
		std::cout << std::endl;

		//dev->SetWireInValue(0x01, value);
		//dev->UpdateWireIns();
	
		dev->WriteRegister(reg.address, value);

		dev->ReadRegister(reg.address, &reg.data);

		std::cout << "Read data: " << reg.data << std::endl;
		//std::cout << "Read data: " << (*outdata) << std::endl;

	}


	int cmd;
	UINT32 addr;

	while (true) {


		std::cout << "enter cmd (0=write, 1=read): ";
		std::cin >> cmd;
		std::cout << "enter address: ";
		std::cin >> addr;

		reg.address = addr;

		if (cmd == 0) {
			//write
			std::cout << "Input value: ";
			std::cin >> value;

			dev->WriteRegister(reg.address, value);
		}
		else {
			//read
			dev->ReadRegister(reg.address, &reg.data);
			dev->ReadRegister(reg.address, &reg.data);	// the first read always shows the LAST addressed value

			//Reading twice here is a hack.  The HDL currently does not use a read enable
			//line as it should.  The read enable wasn't working (always read zero) so it
			//has been removed temporarily.  Probably the FrontPanel API is latching the 
			//read value before the BRAM updates its output.

			std::cout << "Read data: " << reg.data << std::endl;
		}

	}

	return 0;
}

