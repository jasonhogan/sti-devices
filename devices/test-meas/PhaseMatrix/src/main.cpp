
#include <ORBManager.h>
#include "PhaseMatrixDevice.h"

ORBManager* orbManager;



int main(int argc, char* argv[])
{
	orbManager = new ORBManager(argc, argv);    

	int comPortNumber1 = 3;
	int comPortNumber2 = 4;

	PhaseMatrixDevice phaseMatrix1(orbManager, "PhaseMatrix", "EPMezzanine1.stanford.edu", 0, comPortNumber1);
	PhaseMatrixDevice phaseMatrix2(orbManager, "PhaseMatrix 2", "EPMezzanine1.stanford.edu", 1, comPortNumber2);

	phaseMatrix1.setSaveAttributesToFile(true);
	phaseMatrix2.setSaveAttributesToFile(true);

	orbManager->run();

	return 0;
}

