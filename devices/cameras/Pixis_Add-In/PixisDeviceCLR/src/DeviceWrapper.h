#pragma once

//C++/CLI adapter class.

#include "DeviceHolder.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace STI {

public delegate int TestDelegate(int);
public delegate void AquireDelegate(int);
public delegate void StopDelegate(void);
public delegate void GoDelegate(void);

public delegate void SetSaveDirDelegate(String^ dir);
public delegate void SetSaveDirDelegate_Cpp(std::string dir);

public delegate void PrintDelegate(String^ message);
public delegate void PrintDelegate_Cpp(std::string message);

public delegate bool ExternalTriggerOn(void);
public delegate bool IsReadyToAquire(void);
public delegate void ClearImageCount(void);
public delegate void IncrementImageCount(void);


public ref class IMarshaledDelegate {};	//common interface
template<class D, class D_Cpp, class Ret, class ... Args>
public ref class MarshaledDelegate : public IMarshaledDelegate
{
public:
	MarshaledDelegate(D^ del) : del(del) {}

	Ret Func(Args ... args);

	D^ del;
};

//MarshaledDelegate<SetSaveDirDelegate, SetSaveDirDelegate_Cpp, void, std::string>

//template<>
//void MarshaledDelegate<SetSaveDirDelegate, SetSaveDirDelegate_Cpp, void, std::string, double>::Func(std::string dir, double d)
//{
//	String^ str = gcnew String(dir.c_str());
//	return del(str);
//}

//
//ref class SomeClass
//{
//public:
//	SomeClass(SetSaveDirDelegate^ del) : del(del) {}
//	
//	void Func(std::string dir)
//	{
//		String^ str = gcnew String(dir.c_str());
//		return del(str);
//	}
//	SetSaveDirDelegate^ del;
//};
//

public ref class DeviceWrapper
{
public:

	DeviceHolder* device;

	DeviceWrapper(int module);
	~DeviceWrapper();

	void startDevice();
	void shutdown();

	void stopWaiting(int index);
	void setSavedImageFilename(int index, String^ filename);
	void setSavedSPEFilename(int index, String^ filename);


	//Install delegates
	void installDelegate(TestDelegate^ del)		   { _installDelegate<Callback::Test>(del); }
	void installDelegate(AquireDelegate^ del)	   { _installDelegate<Callback::Aquire>(del); }
	void installDelegate(GoDelegate^ del)		   { _installDelegate<Callback::Go>(del); }
	void installDelegate(StopDelegate^ del)		   { _installDelegate<Callback::Stop>(del); }
	void installDelegate(IsReadyToAquire^ del)	   { _installDelegate<Callback::IsReadyToAquire>(del); }
	void installDelegate(ExternalTriggerOn^ del)   { _installDelegate<Callback::ExternalTriggerOn>(del); }
	void installDelegate(ClearImageCount^ del)	   { _installDelegate<Callback::ClearImageCount>(del); }
	void installDelegate(IncrementImageCount^ del) { _installDelegate<Callback::IncrementImageCount>(del); }
	void installDelegate(SetSaveDirDelegate^ del)  { _installMarshaledDelegate
		<Callback::SetSaveDir, SetSaveDirDelegate, SetSaveDirDelegate_Cpp, void, std::string>(del); }
	void installDelegate(PrintDelegate^ del)       { _installMarshaledDelegate
			<Callback::Print, PrintDelegate, PrintDelegate_Cpp, void, std::string>(del); }


//	void installDelegate(SetSaveDirDelegate^ del) { _installDelegate<Callback::SetSaveDir>(del); }
//	void installDelegate(SetSaveDirDelegate^ del) { installDelegate2(del); }


	//void installDelegate2(SetSaveDirDelegate^ del)
	//{
	//	SomeClass^ f = gcnew SomeClass(del);
	//	SetSaveDirDelegate_Cpp^ callback = gcnew SetSaveDirDelegate_Cpp(f, &SomeClass::Func);
	//	_installDelegate<Callback::SetSaveDir>(callback);
	//}
	
private:

	//Needed if the argument types used by the mananged function different from the unmanaged argument types.
	//This requires a custom specialization of MarshaledDelegate<...> for the types in question.
	template<typename CB, class D, class D_Cpp, class Ret, class ... Args>
	void _installMarshaledDelegate(D^ del)
	{
		MarshaledDelegate<D, D_Cpp, Ret, Args...>^ md = gcnew MarshaledDelegate<D, D_Cpp, Ret, Args...>(del);
		D_Cpp^ callback = gcnew D_Cpp(md, &MarshaledDelegate<D, D_Cpp, Ret, Args...>::Func);
		marshaledDelegates.Add(md);	//Save reference to marshaled delegate holder so GC doesn't delete
		_installDelegate<CB>(callback);		//pass along D_Cpp delegate (accepts cpp types)
	}

	template<typename CB>
	void _installDelegate(Delegate^ del)
	{
		delegates.Add(del);
		GCHandle gch = GCHandle::Alloc(del);
		handles.Add(gch);

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(del);

		//Wrap pointer in callback wrapper class for type CB.  Then install_CB calls correct overload.
		device->install_CB(
			CB(
				static_cast<CB::Func>(ip.ToPointer())));
	}


	//Save references to prevent unwanted garbage collection
	List<Delegate^> delegates;
	List<GCHandle> handles;
	List<IMarshaledDelegate^> marshaledDelegates;
};


template<>
void MarshaledDelegate<SetSaveDirDelegate, SetSaveDirDelegate_Cpp, void, std::string>::Func(std::string dir)
{
	String^ str = gcnew String(dir.c_str());
	return del(str);
}

template<>
void MarshaledDelegate<PrintDelegate, PrintDelegate_Cpp, void, std::string>::Func(std::string message)
{
	String^ str = gcnew String(message.c_str());
	return del(str);
}

};




//
//
//template<class R, typename A, typename B>
//ref class CC
//{
//public:
//	A f(B b);
//};
//
//template<>
//int CC<int, int, int>::f(int b)
//{
//	return b;
//}
//
//
////template<class R>
////ref class CC
////{
////public:
////	template<typename A, typename B>
////	A f(B b);
////};
////
////
////typedef CC<double> ccd;
////
////template<>
////int ccd::f<int,int>(int b)
////{
////	return b;
////}






//
//template<typename T>
//class Convert
//{
//
//};
//public ref class IMarshaller abstract
//{
//
//};
//
////template<typename T>
//public ref class Marshaller : public IMarshaller
//{
//public:
//	Marshaller(SetSaveDirDelegate^ del) : del(del) 
//	{
/////		cb = Marshaller::tmp;
//	}
//
////	Callback::SetSaveDir::Func cb;
//
//
//	void cb(std::string dir)
//	{
//		String^ str = gcnew String(dir.c_str());
//		del(str);
//	}
//
//	SetSaveDirDelegate^ del;
////	T^ del2;
//};
//