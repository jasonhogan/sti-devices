#ifndef LIGHTFIELDHANDLE_H
#define LIGHTFIELDHANDLE_H

#include "callbacks.h"

class LightFieldHandle
{
public:
	LightFieldHandle();

	void aquire(int index);
	void stop();
	void go();
	int test(int x);
	bool isReadyToAquire();
	void clearImageCount();
	void incrementImageCount();

	//Install callbacks
	void install_CB(Callback::Aquire cb) { aquireCB = cb.cb; }
	void install_CB(Callback::Go cb) { goCB = cb.cb; }
	void install_CB(Callback::Stop cb) { stopCB = cb.cb; }
	void install_CB(Callback::Test cb) { testCB = cb.cb; }
	void install_CB(Callback::IsReadyToAquire cb) { isReadyCB = cb.cb; }
	void install_CB(Callback::ClearImageCount cb) { clearCountCB = cb.cb; }
	void install_CB(Callback::IncrementImageCount cb) { incrementCountCB = cb.cb; }

private:

	//Callbacks
	Callback::Aquire::Func aquireCB;
	Callback::Go::Func goCB;
	Callback::Stop::Func stopCB;
	Callback::Test::Func testCB;

	Callback::IsReadyToAquire::Func isReadyCB;
	Callback::ClearImageCount::Func clearCountCB;
	Callback::IncrementImageCount::Func incrementCountCB;

};

#endif
