#ifndef CALLBACKS_H
#define CALLBACKS_H

//typedef int(__stdcall *TESTCB)(int);
//typedef void(__stdcall *AQUIRECB)(void);
//typedef void(__stdcall *STOPCB)(void);

#include <string>

namespace Callback
{

struct ExternalTriggerOn
{
	typedef bool(__stdcall *Func)(void);
	ExternalTriggerOn(Func cb) : cb(cb) {}
	Func cb;
};
struct IsReadyToAquire
{
	typedef bool(__stdcall *Func)(void);
	IsReadyToAquire(Func cb) : cb(cb) {}
	Func cb;
};
struct ClearImageCount
{
	typedef void(__stdcall *Func)(void);
	ClearImageCount(Func cb) : cb(cb) {}
	Func cb;
};
struct IncrementImageCount
{
	typedef void(__stdcall *Func)(void);
	IncrementImageCount(Func cb) : cb(cb) {}
	Func cb;
};

struct Test
{
	typedef int(__stdcall *Func)(int);
	Test(Func cb) : cb(cb) {}
	Func cb;
};

struct Aquire
{
	typedef void(__stdcall *Func)(int);
	Aquire(Func cb) : cb(cb) {}
	Func cb;
};

struct SetSaveDir
{
	typedef void(__stdcall *Func)(std::string);
	SetSaveDir(Func cb) : cb(cb) {}
	Func cb;
};

struct Print
{
	typedef void(__stdcall *Func)(std::string);
	Print(Func cb) : cb(cb) {}
	Func cb;
};

struct Go
{
	typedef void(__stdcall *Func)(void);
	Go(Func cb) : cb(cb) {}
	Func cb;
};

struct Stop
{
	typedef void(__stdcall *Func)(void);
	Stop(Func cb) : cb(cb) {}
	Func cb;
};

};

//struct GOCB
//{
//	typedef void(__stdcall *CB)(void);
//	GOCB(CB cb) : cb(cb) {}
//	CB cb;
//};

//template<typename CB>
//struct Callback
//{
//	using Callback<CB>::Callback;
//	Callback(CB cb) : cb(cb) {}
//	CB cb;
//};
//
//typedef void(__stdcall *GO2CB)(void);
//struct GO2 : public Callback<GO2CB> 
//{
////	using Callback<GO2CB>::Callback;
//};


#endif
