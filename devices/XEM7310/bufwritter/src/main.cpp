
#include <iostream>
#include <bitset>

#include "okFrontPanelDLL.h"
#include "frontpanelsupport.h"

UINT32 makeBitValue(const std::string& bits, bool enable = true)
{
	std::bitset<26> output(bits);
	output.set(24, enable);	//enable

	return output.to_ulong();
}

void printRegData(const std::vector<okTRegisterEntry>& regs)
{
	for (unsigned j = 0; j < regs.size(); ++j) {
		std::cout << "<"
			<< regs.at(j).address << ", "
			<< regs.at(j).data << ">" << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	// Initialize the FPGA with our configuration bitfile.
	OpalKelly::FrontPanelPtr dev = initializeFPGA("bufwriter.bit");
	if (!dev.get()) {
		printf("FPGA could not be initialized.\n");
		return -1;
	}

	okTRegisterEntry reg;

	std::vector<okTRegisterEntry> writeRegs;
	std::vector<okTRegisterEntry> readRegs;

	unsigned startaddr = 0;
	unsigned endaddr = 7;

	//setup and write
	for (unsigned i = startaddr; i <= endaddr; ++i) {
		reg.address = i;
		reg.data = 0;

		writeRegs.push_back(reg);
	}

	//Data buffer initialization
	writeRegs.at(0).data = makeBitValue("00000001");
	writeRegs.at(1).data = makeBitValue("00000010");
	writeRegs.at(2).data = makeBitValue("00000100");
	writeRegs.at(3).data = makeBitValue("00001000");
	writeRegs.at(4).data = makeBitValue("00010010");
	writeRegs.at(5).data = makeBitValue("00100000");
	writeRegs.at(6).data = makeBitValue("01000000");
	writeRegs.at(7).data = makeBitValue("10000000");

	int tmp;
	std::cout << "write reg? ";
	std::cin >> tmp;

	dev->WriteRegisters(writeRegs);

	bool readback = false;
	if (readback) {
		
		//setup and read
		for (unsigned i = startaddr; i <= endaddr; ++i) {
			reg.address = i;

			readRegs.push_back(reg);
		}

		dev->ReadRegisters(readRegs);
		std::cout << "Read back: " << std::endl;
		printRegData(readRegs);
	}


	std::cout << "reset? ";
	std::cin >> tmp;

	dev->ActivateTriggerIn(0x40, 1);	//reset

	int count = 0;

	
	UINT32 debugVal;

	dev->UpdateWireOuts();
	debugVal = dev->GetWireOutValue(0x21);
//	std::cout << "Debug: " << debugVal << std::endl;


	//This works to advance the state machine, but so far no data from the bus has been read.

	while (true) {
	//	std::cout << "Event: " << count << "    ";
		std::cin >> tmp;
		
		dev->ActivateTriggerIn(0x40, 0);	//trigger a single address advance

		dev->UpdateWireOuts();
		debugVal = dev->GetWireOutValue(0x21);
//		std::cout << "Debug: " << debugVal << std::endl;

		count++;
	}

	return 0;
}

