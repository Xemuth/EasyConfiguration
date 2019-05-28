#include <Core/Core.h>
#include "EasyConfiguration.h"
using namespace Upp;

CONSOLE_APP_MAIN
{
	EasyConfiguration ez;
	try{
		ez.SetValue<bool>("test",true); 
		ez.SetValue<String>("test2", "Hello world !");
		ez.SetValue<int>("test3",123);
		ez.SetValue<double>("test4",123.0);
	}catch(Upp::ValueTypeError &e){
		Cout() <<"Exception"<< e <<"\n";	
	}
	bool test1 = ez.GetValue<bool>("test");
	String test2 = ez.GetValue<String>("test2");
	int test3 = ez.GetValue<int>("test3");
	double test4 = ez.GetValue<double>("test4");
	int test5 = ez.GetValue<int>("test5");
	
	Cout() << "test1 : " <<test1 << "\n";
	Cout() << "test2 : " <<test2 << "\n";
	Cout() << "test3 : " <<test3 << "\n";
	Cout() << "test4 : " <<test4 << "\n";
	Cout() << "Test5 (Int non definie) : " << test5 <<"\n";
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
		Value v(t);
		if(ConfigurationType.Add(fieldName,v))
			return true;
	}catch(Upp::ValueTypeError &e){
		throw e;	
	}
}



EasyConfiguration::EasyConfiguration(){
}

