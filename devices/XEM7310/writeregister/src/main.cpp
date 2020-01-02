
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

	bool promptAddr = true;

	bool multipleAddr = true;

	int trials = 10;
	
	while (trials > 0) {
		trials--;

		if (multipleAddr) {

			std::vector<okTRegisterEntry> writeRegs;
			std::vector<okTRegisterEntry> readRegs;

			unsigned startaddr = 0;
			unsigned endaddr = 10;

			//setup and write
			for (unsigned i = startaddr; i < endaddr; ++i) {
				reg.address = i;
				reg.data = i;

				writeRegs.push_back(reg);
			}

			dev->WriteRegisters(writeRegs);

			//setup and read
			for (unsigned i = startaddr; i < endaddr; ++i) {
				reg.address = i;

				readRegs.push_back(reg);
			}

			UINT32 tmp;
			//	dev->ReadRegister(readRegs.at(1).address, &tmp);
			//	dev->ReadRegister(readRegs.at(0).address, &tmp);
			//	dev->ReadRegister(readRegs.at(3).address, &tmp);
			//	std::cout << "tmp: " << tmp << std::endl;
			//	dev->ReadRegister(readRegs.at(3).address, &tmp);
			//	std::cout << "tmp: " << tmp << std::endl;
			//	dev->ReadRegister(readRegs.at(3).address, &tmp);
			//	std::cout << "tmp: " << tmp << std::endl;

			dev->ReadRegisters(readRegs);

			std::cout << "Read back: " << std::endl;
			for (unsigned j = 0; j < readRegs.size(); ++j) {
				std::cout << "<"
					<< readRegs.at(j).address << ", "
					<< readRegs.at(j).data << ">" << std::endl;
			}
			std::cout << std::endl;

		}
	}

	while (promptAddr) {


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
			//	dev->ReadRegister(reg.address, &reg.data);	// the first read always shows the LAST addressed value

				//Reading twice here is a hack.  The HDL currently does not use a read enable
				//line as it should.  The read enable wasn't working (always read zero) so it
				//has been removed temporarily.  Probably the FrontPanel API is latching the 
				//read value before the BRAM updates its output.

				//Fixed: This test case works perfectly if I clock the BRAM with sys_clk instead of okClk.
				//I also added a timing contraint between sys_clk and regAddress -- not sure it that matters or makes sense.
				//However: This is an unstable solution; when reading multiple register values, it's clear
				//that the okClk and sys_clk are not synchronous, so the reads addresses will slip non-deterministically.

			std::cout << "Read data: " << reg.data << std::endl;
		}

	}

	return 0;
}

