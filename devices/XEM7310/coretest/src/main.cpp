
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

class RegEvent
{
public:
	RegEvent(UINT32 addr, UINT32 time, const std::string& bits, char opcode) : addr(addr), time(time), opcode(opcode)
	{
		operand = makeBitValue(bits);
	}

	RegEvent(UINT32 time, const std::string& bits, char opcode) : RegEvent(0, time, bits, opcode) {}

	UINT32 getValue() const
	{
		return (opcode << 28) + operand;
	}

	UINT32 time;
	UINT32 operand;
	UINT32 addr;
	char opcode;
};

void addRegEvent(std::vector<okTRegisterEntry>& regs, const RegEvent& regEvent)
{
	okTRegisterEntry reg;


	reg.address = regEvent.addr;
	reg.data = regEvent.getValue();
	regs.push_back(reg);	//add value

	reg.address = regEvent.addr + 1;
	reg.data = regEvent.time;
	regs.push_back(reg);	//add time

}



void printRegData(const std::vector<okTRegisterEntry>& regs)
{
	for (unsigned j = 0; j < regs.size(); ++j) {
		std::cout << std::hex 
			<< "< x"
			<< regs.at(j).address << ", x"
			<< regs.at(j).data << " >" << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	// Initialize the FPGA with our configuration bitfile.
	OpalKelly::FrontPanelPtr dev = initializeFPGA("timing_mod_test_top.bit");
	if (!dev.get()) {
		printf("FPGA could not be initialized.\n");
		return -1;
	}

	std::vector<RegEvent> rawEvents;
	std::vector<okTRegisterEntry> writeRegs;
	
	//                     time, value, opcode
	rawEvents.emplace_back(10, "00000000", 1);	//wait for trigger event
	rawEvents.emplace_back(10, "00000000", 0); 
	rawEvents.emplace_back(0, "00000001", 0);
	rawEvents.emplace_back(0, "00000010", 0);
	rawEvents.emplace_back(0, "00000100", 0);
	rawEvents.emplace_back(0, "00000010", 0);
	rawEvents.emplace_back(0, "00000000", 0);
	rawEvents.emplace_back(0, "00000010", 0);
	rawEvents.emplace_back(0, "00000000", 0);

	rawEvents.emplace_back(1000, "00000001", 0);
	rawEvents.emplace_back(0, "00000000", 0);

	rawEvents.emplace_back(0, "00000000", 2);	//Stop

	unsigned initaddr = 0;

	//define addresses and convert to okRegisters
	for (unsigned i = initaddr; i < rawEvents.size(); ++i) {
		rawEvents.at(i).addr = 2 * i;

		addRegEvent(writeRegs, rawEvents.at(i));	//adds value and time on sequential addresses
	}

	printRegData(writeRegs);


	int tmp;
//	std::cout << "write reg? ";
//	std::cin >> tmp;

	dev->WriteRegisters(writeRegs);
	
//	std::cout << "reset? ";
//	std::cin >> tmp;

//	dev->ActivateTriggerIn(0x40, 1);	//reset
	
	//Set trigger mode on bit 0 ( 0=Software, 1=Hardware)
	UINT32 value = 0;		//set LSB
	dev->SetWireInValue(0x01, value);
	dev->UpdateWireIns();

	std::cout << "play? ";
	std::cin >> tmp;
	dev->ActivateTriggerIn(0x40, 0);	//play

	std::cout << "trigger? ";
	std::cin >> tmp;
	dev->ActivateTriggerIn(0x41, 0);	//trigger mod 0

	std::cout << "waiting for trigger... ";
	std::cin >> tmp;
	//return 0;

	while (true) {


		//dev->ActivateTriggerIn(0x41, 0);	//trigger mod 0
		dev->ActivateTriggerIn(0x40, 0);	//trigger the sequence. NOTE: the second argument ('bit') refers to which bit (of 32) is to be triggered on this call.

	}

	return 0;
}

