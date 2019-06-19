#include <Core/Core.h>
#include "EasyConfiguration.h"
#include <cctype>
using namespace Upp;




/*
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
	Cout() << ez.LoadConfiguration(R"(\\qf53418\esp34\AGF_ASSURANCES\DIR_INDEMNISATION\AOO\localAOO.cfg)")<<"\n";
	ez.RelaxMode(false);
	ez.SaveConfiguration("local.cfg");
	
}*/
/*
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
	return false;
}*/

int EasyConfiguration::LoadConfiguration(String FilePath){
	if (FileExists(FilePath)){
		FileOpened = FilePath;
		FileIn in(FilePath);
		if (in){
			while(!in.IsEof()){
				ResolveAndAddLine(in.GetLine());
				
			}
			in.Close();
			return ConfigurationType.GetCount();
		}
	} 	
	return 0;
}

const VectorMap<String,Upp::Value>& EasyConfiguration::GetConfiguration()const{
	return this->ConfigurationType;	
}

int EasyConfiguration::GetCount(){
	return ConfigurationType.GetCount();
}

bool EasyConfiguration::ResolveAndAddLine(String line){
	try{
		if(line.Find("->",1) != -1){ 
			if(line.Find("=",line.Find("->",1) +3)>(line.Find("->",1)+3)){ // Here I must be sure their is value between -> and =
				String type = line.Left(line.Find("->",1));
				String name = line.Mid(line.Find("->",1)+2, line.Find("=",line.Find("->",1)) - (line.Find("->",1)+2) );
				String value = line.Right(line.GetCount()-(line.Find("=")+1));
				if(type.IsEqual("bool")){
					if(value.Find("b")>-1 && isStringisANumber(value.Right(value.GetCount()-1)) ){
						value.Replace("b","");
						SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false));
					}else if(value.IsEqual("true") || value.IsEqual("false")){
						SetValue<bool>(name, ((value.IsEqual("true"))? true:false));
					}else if (isStringisANumber(value)){
						SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false)   );
					}
				}else if(type.IsEqual("int")){
					if(value.GetCount() > 9){
						SetValue<double>(name,std::stoi(value.ToStd()));
					}else if(value.Find(",") || value.Find(".")){
						SetValue<float>(name,std::stoi(value.ToStd()));
					}else{
						SetValue<int>(name,std::stoi(value.ToStd()));
					}
			    }else if(type.IsEqual("double")){
			        SetValue<double>(name,std::stoi(value.ToStd()));
			    }else if(type.IsEqual("float")){
			    	SetValue<float>(name,std::stoi(value.ToStd()));
				}else if(type.IsEqual("rc4")){
					if (value.GetCount() > 0 &&  value[0] != '@'){
						value = '@' + value;
					}
					SetValue<String>(name,value);
				}else if(type.IsEqual("string")){
					SetValue<String>(name,value);
				}
				return true;
			}
		}
		else if(line.Find("=",1)>1){
			String name = line.Left(line.Find("="));
			String value = line.Right(line.GetCount()-(line.Find("=")+1));
			String type = "";
			if(value.GetCount()> 0 && isStringisANumber(value)){
				if(value.GetCount() > 9){
					SetValue<double>(name,std::stoi(value.ToStd()));
				}else if(value.Find(",") || value.Find(".")){
					SetValue<float>(name,std::stoi(value.ToStd()));
				}else{
					type="int";
					SetValue<int>(name,std::stoi(value.ToStd()));
				}
			}else if(value.GetCount()> 0 && ((value[0] == 'b' && isStringisANumber(value.Right(value.GetCount()-1))) || (value.IsEqual("true") || value.IsEqual("false")))  ){
				if(value.Find("b")>-1 && isStringisANumber(value.Right(value.GetCount()-1)) ){
					value.Replace("b","");
					SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false));
				}else if(value.IsEqual("true") || value.IsEqual("false")){
					SetValue<bool>(name, ((value.IsEqual("true"))? true:false));
				}
				type="bool";
			}else if(value.GetCount()> 0 && value[0] == '@'){
				value.Replace("@","");
				type="rc4";
				SetValue<String>(name,value);
			}else{
				type="string";	
				SetValue<String>(name,value);
			}	
			return true;
		}
	}catch(...){
		Cout()<<"Exception has occured in ResolveAndAddLine Methode	"<<"\n";
		return false;
	}
	return false;
}

bool EasyConfiguration::isStringisANumber(Upp::String stringNumber){
	if (std::isdigit(stringNumber[0]) || (stringNumber.GetCount() > 1 && (stringNumber[0] == '+'))){
        for (int i = 1 ; i < stringNumber.GetCount(); ++i)
            if (!std::isdigit(stringNumber[i]))
                return false;
        return true;
    }
    return false;
}

bool EasyConfiguration::SaveConfiguration(){
	if(FileOpened.GetCount() != 0){
		FileIn in(FileOpened);
		if(in){
			for(const String &e : ConfigurationType.GetKeys()){
				if(  ConfigurationType.Get(e).GetTypeName().IsEqual("String")){
					in << ((SaveInRelaxed)? ConfigurationType.Get(e).GetTypeName() : "") << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<String>() <<"\n";
				}else if( ConfigurationType.Get(e).GetTypeName().IsEqual("bool")){
					in << ((SaveInRelaxed)? ConfigurationType.Get(e).GetTypeName() : "") << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<bool>() <<"\n";
				}else if(  ConfigurationType.Get(e).GetTypeName().IsEqual("int")){
					in << ((SaveInRelaxed)? ConfigurationType.Get(e).GetTypeName() : "") << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<int>() <<"\n";
				}
			}
			in.Close();
		}
	}
}

EasyConfiguration EasyConfiguration::SaveConfiguration(String filePath){
	if(filePath.GetCount() != 0){
		FileOut  out(filePath);
		if(out){
			for(const String &e : ConfigurationType.GetKeys()){
				if(  ConfigurationType.Get(e).GetTypeName().IsEqual("String")){
					
					out << ((!SaveInRelaxed)? ConfigurationType.Get(e).GetTypeName() : "") << ((!SaveInRelaxed)? "->":"") << e << "=" << ConfigurationType.Get(e).Get<String>() <<"\n";
					
				}else if( ConfigurationType.Get(e).GetTypeName().IsEqual("bool")){
					out << ((!SaveInRelaxed)? ConfigurationType.Get(e).GetTypeName() : "") << ((!SaveInRelaxed)? "->":"") << e << "=" << ((ConfigurationType.Get(e).Get<int>()!=0)?"true":"false") <<"\n";
				}else if(  ConfigurationType.Get(e).GetTypeName().IsEqual("int")){
					out << ((!SaveInRelaxed)? ConfigurationType.Get(e).GetTypeName() : "") << ((!SaveInRelaxed)? "->":"") << e << "=" << ConfigurationType.Get(e).Get<int>() <<"\n";
				}
			}
			out.Close();
		}
	}
}

const String EasyConfiguration::GetFileOpened(){
	return FileOpened;
}

		
		
bool EasyConfiguration::NewConfiguration(const EasyConfiguration& ec){ //Used to copy configuration from ec to this
	ConfigurationType.Clear();
	for(const String &key : ec.GetConfiguration().GetKeys()){
		SetValue(key,ec.GetConfiguration().Get(key));
	}
}

bool EasyConfiguration::UpdateConfigurationFromMaster(const EasyConfiguration& ec){ //Used to update all this value by ec value (do not add new value to this, just update existing value)
	
}
bool EasyConfiguration::MergeUpdateConfiguration(const EasyConfiguration& ec){ //Add and update every new configuration value
	
}
bool EasyConfiguration::MergeConfiguration(const EasyConfiguration& ec){ //Add every new tag to actual configuration
	
}

void EasyConfiguration::RelaxMode(bool b){
	SaveInRelaxed = b;
}
bool EasyConfiguration::isRelaxMode(){
	return SaveInRelaxed;	
}

EasyConfiguration::EasyConfiguration(){
}
EasyConfiguration::EasyConfiguration(Upp::String FilePath){
	LoadConfiguration(FilePath);
}
EasyConfiguration::EasyConfiguration(const EasyConfiguration &ec) {
	SaveInRelaxed =  ec.SaveInRelaxed;
	for(const String &key : ec.GetConfiguration().GetKeys()){
		SetValue(key,ec.GetConfiguration().Get(key));
	}
}
