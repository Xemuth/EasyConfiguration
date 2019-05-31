#include <Core/Core.h>
#include "EasyConfiguration.h"
using namespace Upp;

CONSOLE_APP_MAIN
{
	EasyConfiguration ez;
	try{
		ez.SetValue<bool>("test",true); 
		ez.SetValue<String>("test2", "Hello World");
		ez.SetValue<int>("test3",123);
		ez.SetValue<double>("test4",123.0);
	}catch(Upp::ValueTypeError &e){
		Cout() <<"Exception"<< e <<"\n";	
	}
	Cout() << "test1 : " << ez.GetValue<bool>("test") << "\n";
	Cout() << "test2 : " << ez.GetValue<String>("test2") << "\n";
	Cout() << "test3 : " << ez.GetValue<int>("test3") << "\n";
	Cout() << "test4 : " << ez.GetValue<double>("test4") << "\n";
	Cout() << "Test5 (Int non definie) : " << ez.GetValue<int>("test5") <<"\n";
}

template <class T>   
T EasyConfiguration::GetValue(String fieldName){
	int index = ConfigurationType.Find(fieldName);
	if(index >= 0 && ConfigurationType[index].Is<T>()){
		return (T)ConfigurationType[index].Get<T>();
	}
	return T();
}

template <class T>
bool EasyConfiguration::SetValue(String fieldName, const T &t){
	try{
		if(&ConfigurationType.Add(fieldName,Value(t)))
			return true;
	}catch(Upp::ValueTypeError &e){
		throw e;	
	}
}


EasyConfiguration::EasyConfiguration(){
}


