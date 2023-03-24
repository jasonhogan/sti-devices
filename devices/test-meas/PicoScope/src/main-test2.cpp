
#include "PicoScope5000.h"
#include <fstream>

#include "PicoScope5000Device.h"

int main(int argc, char* argv[])
{
	PicoScope5000 scope;

	scope.enableChannel(0, true);
	scope.enableChannel(1, true);
	scope.setupScopeBuffers(100, 100);
	scope.runBlock();

	scope.waitForRunComplete();

	std::ofstream out("output.txt");

	scope.writeDataToFile(out);
	out.close();

	return 0;
}
