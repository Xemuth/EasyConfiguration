#include <Core/Core.h>
#include "EasyConfiguration.h"
#include <cctype>
using namespace Upp;

EasyConfiguration::EasyConfiguration(){
}

EasyConfiguration::EasyConfiguration(Upp::String FilePath){
	LoadConfiguration(FilePath);
}

EasyConfiguration::EasyConfiguration(const EasyConfiguration &ec) {
	SaveInRelaxed =ec.SaveInRelaxed;
	rc4Key = ec.rc4Key;
	FileOpened = ec.GetFileOpened();
	auto e = Vector<String>();
	UltraUpdate(ec,e,true,true);
}

int EasyConfiguration::LoadConfiguration(String FilePath){ //Do not clear an old configuration
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

bool EasyConfiguration::setRC4Key(Upp::String _rc4Key){
	rc4Key =_rc4Key;
}

bool EasyConfiguration::ResolveAndAddLine(String line){
	try{
		if(line[0] != '#'){
			if(line.Find("->",1) != -1){ 
				if(line.Find("=",line.Find("->",1) +3)>(line.Find("->",1)+3)){ // Here I must be sure their is value between -> and =
					String type = ToLower( line.Left(line.Find("->",1)));
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
							Rc4.SetKey(rc4Key);
							Upp::String decoded = Rc4.Encode(value);
							if (decoded[decoded.GetCount()-1] == '@'){
								value = '@' + decoded.Left(decoded.GetCount()-1);
							}
							else{
								value = '@' + value;
							}
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
					type="rc4";
					SetValue<String>(name,value);
				}else if(value.GetCount() > 0 &&  value[0] != '@'){
					Rc4.SetKey(rc4Key);
					Upp::String decoded = Rc4.Encode(value);
					if (decoded[decoded.GetCount()-1] == '@'){
						type="string";	
						value = '@' + decoded.Left(decoded.GetCount()-1);
					}else{
						type="string";		
					}
					SetValue<String>(name,value);
				}else{
					type="string";	
					SetValue<String>(name,value);
				}	
				return true;
			}
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
		FileOut out(FileOpened);
		if(out){
			for(const String &e : ConfigurationType.GetKeys()){
				if(  ConfigurationType.Get(e).GetTypeName().IsEqual("String")){
					String val  = static_cast<String>(ConfigurationType.Get(e).Get<String>());
					if(val[0] == '@'){
						val = val.Mid(1,val.GetCount() -1);
						val +='@';
						Rc4.SetKey(rc4Key);
						val = Rc4.Encode(val);
						out << ((SaveInRelaxed)? "" : "rc4") << ((SaveInRelaxed)? "":"->") << e << "=" << val <<"\n";
					}else{
						out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<String>() <<"\n";
					}
				}else if( ConfigurationType.Get(e).GetTypeName().IsEqual("bool")){
					out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<bool>() <<"\n";
				}else if(  ConfigurationType.Get(e).GetTypeName().IsEqual("int")){
					out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<int>() <<"\n";
				}
			}
			out.Close();
			if(out.IsError()) { // check whether file was properly written
				LOG("Error");
				return false;
			}
			return true;
		}
	}
	return false;
}

bool EasyConfiguration::SaveConfiguration(String filePath,bool changePath){
	if(filePath.GetCount() != 0){
		FileOut out(filePath);
		if(out){
			for(const String &e : ConfigurationType.GetKeys()){
				if(  ConfigurationType.Get(e).GetTypeName().IsEqual("String")){
					String val  = static_cast<String>(ConfigurationType.Get(e).Get<String>());
					if(val[0] == '@'){
						val = val.Mid(1,val.GetCount() -1);
						val +='@';
						Rc4.SetKey(rc4Key);
						val = Rc4.Encode(val);
						out << ((SaveInRelaxed)? "" : "rc4") << ((SaveInRelaxed)? "":"->") << e << "=" << val <<"\n";
					}else{
						out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<String>() <<"\n";
					}
				}else if( ConfigurationType.Get(e).GetTypeName().IsEqual("bool")){
					out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<bool>() <<"\n";
				}else if(  ConfigurationType.Get(e).GetTypeName().IsEqual("int")){
					out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<int>() <<"\n";
				}
			}
			out.Close();
			if(out.IsError()) { // check whether file was properly written
				LOG("Error");
				return false;
			}
			if(changePath || FileOpened.GetCount()==0)FileOpened = filePath;
			return true;
		}
	}
	return false;
}

const String EasyConfiguration::GetFileOpened() const{
	return FileOpened;
}

bool EasyConfiguration::FindInVectorString(Vector<String> &vector,String value){
	for(String& val :vector){
		if(val.IsEqual(value))
			return true;	
	}
	return false;
}

bool EasyConfiguration::UltraUpdate(const EasyConfiguration& ec,Vector<String> &exception,bool update,bool merge,bool ApplyExceptionUpdate,bool ApplyExceptionMerge){
	bool trouver = false;
	auto &ecConfigurationType=ec.GetConfiguration();
	try{
		for(const String  &key : ecConfigurationType.GetKeys()){
			bool isException = FindInVectorString(exception,key);
			if((isException && !ApplyExceptionUpdate) || !isException){
				for(const String  &key2 : ConfigurationType.GetKeys() ){
					if(key2.IsEqual(key)){
						if(update){
							if(ecConfigurationType.Get(key).GetTypeName().IsEqual("String")){
								SetValue(key,ecConfigurationType.Get(key).Get<String>());
							}else if(ecConfigurationType.Get(key).GetTypeName().IsEqual("bool")){
								SetValue(key,ecConfigurationType.Get(key).Get<bool>());
							}else if(ecConfigurationType.Get(key).GetTypeName().IsEqual("int")){
								SetValue(key,ecConfigurationType.Get(key).Get<int>());
							}
						}
						trouver= true;
						break;
					}
				}
			}
			if((isException && !ApplyExceptionMerge) || !isException ){
				if(ecConfigurationType.Get(key).GetTypeName().IsEqual("String")){
					SetValue<String>(key,ecConfigurationType.Get(key).Get<String>());
				}else if(ecConfigurationType.Get(key).GetTypeName().IsEqual("bool")){
					SetValue(key,ecConfigurationType.Get(key).Get<bool>());
				}else if(ecConfigurationType.Get(key).GetTypeName().IsEqual("int")){
					SetValue(key,ecConfigurationType.Get(key).Get<int>());
				}
			}
			isException = false;
			trouver = false;
		}
		return true;
	}catch(...){
		return false;	
	}
	return false;
}
		
		
bool EasyConfiguration::NewConfiguration(const EasyConfiguration& ec, Vector<String> exception,bool ApplyExceptionUpdate,bool ApplyExceptionMerge){ //Used to copy configuration from ec to this
	ConfigurationType.Clear();
	return UltraUpdate(ec,exception,false,true,ApplyExceptionUpdate,ApplyExceptionMerge);
}

bool EasyConfiguration::UpdateConfigurationFromMaster(const EasyConfiguration& ec, Vector<String> exception,bool ApplyExceptionUpdate,bool ApplyExceptionMerge){ //Used to update all this value by ec value (do not add new value to this, just update existing value)
	return UltraUpdate(ec,exception,true,false,ApplyExceptionUpdate,ApplyExceptionMerge);
}
bool EasyConfiguration::MergeUpdateConfiguration(const EasyConfiguration& ec, Vector<String> exception,bool ApplyExceptionUpdate,bool ApplyExceptionMerge){ //Add and update every new configuration value
	return UltraUpdate(ec,exception,true,true,ApplyExceptionUpdate,ApplyExceptionMerge);
}
bool EasyConfiguration::MergeConfiguration(const EasyConfiguration& ec, Vector<String> exception,bool ApplyExceptionUpdate,bool ApplyExceptionMerge){ //Add every new tag to actual configuration
	return UltraUpdate(ec,exception,false,true,ApplyExceptionUpdate,ApplyExceptionMerge);
}

void EasyConfiguration::RelaxMode(bool b){
	SaveInRelaxed = b;
}
bool EasyConfiguration::isRelaxMode(){
	return SaveInRelaxed;	
}


