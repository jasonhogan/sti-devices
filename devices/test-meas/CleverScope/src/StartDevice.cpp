
#include "StartDevice.h"
#include "CleverScopeDevice.h"
#include <ORBManager.h>


void makeAndRunDeviceWithHandle(UnmanagedCallback callback, 
								UnmanagedCallback callbackA, UnmanagedCallback callbackB, UnmanagedCallback callbackTrigger,
								STIDeviceCLRBridgeWrapper* deviceWrapper)
{
	//Fake args for ORBManager
	int argc = 1;
	char* argv[] = {""};

	ORBManager* orbManager = new ORBManager(argc, argv);
	
	CleverScopeDevice cleverScope(orbManager, "Clever Scope", "eppit1.stanford.edu", 0, callback);
	cleverScope.setSaveAttributesToFile(true);

	cleverScope.addHandleChannelA(callbackA);
	cleverScope.addHandleChannelB(callbackB);
	cleverScope.addHandleTrigger(callbackTrigger);

	//Connect the supplied wrapper with the actual device bridge pointer.
	deviceWrapper->setDeviceBridge(&cleverScope);

	orbManager->run();
	
	//If device exits for some reason, clear the bridge pointer.
	deviceWrapper->setDeviceBridge(NULL);
}
