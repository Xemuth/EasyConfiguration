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
	bool fileEncrypted =  false;
	if (FileExists(FilePath)){
		FileOpened = FilePath;
		FileIn in(FilePath);
		if (in){
			Upp::String buffer ="";
			Upp::String commentaireBuffer ="";
			String line = "";
			int cpt = 0;
			int ln = 0;
			if(!in.IsEof()){
				buffer = in.GetLine();
				if(buffer[0]== '@'){ //if file encrypted
					Rc4.SetKey(rc4Key); 
					fileEncrypted = true;
					buffer.Remove(0);
					//parfois l'algo de cryptage met des "\n", qu'on doit gérer car l'objet
					//"in" l'interprète comme un retour à la ligne au lieu de deux simples
					//caractères
					while(!in.IsEof()){
						buffer+= "\n"+in.GetLine();
					}	
					//Cout() << "buffer : " << ToCharset(CHARSET_DEFAULT,buffer,CHARSET_CP850) << EOL;
					buffer = Rc4.Encode(buffer);
					//Cout() << "buffer : " << ToCharset(CHARSET_DEFAULT,buffer,CHARSET_CP850) << EOL;
					while(buffer.Find("\n") != -1){
						line = buffer.Mid(0,buffer.Find("\n"));
						buffer = buffer.Mid(buffer.Find("\n")+1,buffer.GetCount());
					//	Cout() << "line load : " << line << EOL;
					//	Cout() << "buffer load : " << buffer << EOL;
						if( line[0] == '#'){
							commentaireBuffer+= line +"\n";
						}else{ 
							if(commentaireBuffer.GetCount() > 0){ 
								AddCommentaire(cpt,commentaireBuffer);
								commentaireBuffer="";
							}
							ResolveAndAddCryptedLine(line);
							cpt++;
						}
					}
				}else{
					while(!in.IsEof()){
						if(ln == 0){
							if(buffer.GetCount() > 0){
								if( buffer[0] == '#'){
									commentaireBuffer+= buffer +"\n";
								}else{ 
									if(commentaireBuffer.GetCount() > 0){ 
										AddCommentaire(cpt,commentaireBuffer);
										commentaireBuffer="";
									}
									ResolveAndAddLine(buffer);
									cpt++;
								}
								ln++;
								//Cout() << "buffer load : " << buffer << EOL;
							}
						}else{						
							buffer = in.GetLine();
							if(buffer.GetCount() > 0){
								if( buffer[0] == '#'){
									commentaireBuffer+= buffer +"\n";
								}else{ 
									if(commentaireBuffer.GetCount() > 0){ 
										AddCommentaire(cpt,commentaireBuffer);
										commentaireBuffer="";
									}
									ResolveAndAddLine(buffer);
									cpt++;
								}
								//Cout() << "buffer load : " << buffer << EOL;
							}
						}

						
					}
				}
			}
			
			
			
			if(commentaireBuffer.GetCount() > 0){ 
				AddCommentaire(cpt,commentaireBuffer);
				commentaireBuffer="";
			}
			in.Close();
			if(ConfigurationType.GetCount() == 0){ // si le fichier est vide
				return -1;
			}
			return ConfigurationType.GetCount();
		}
	}
	//si le fichier n'existe pas
	return 0;
}

const VectorMap<String,Upp::Value>& EasyConfiguration::GetConfiguration()const{
	return this->ConfigurationType;	
}

const VectorMap<int,String>& EasyConfiguration::GetCommentaireBuffer() const{
	return this->CommentaireBuffer;	
}

int EasyConfiguration::GetCount(){
	return ConfigurationType.GetCount();
}

bool EasyConfiguration::ReloadConfiguration(){
	if(FileOpened.GetCount() && FileExists(FileOpened)){
		ConfigurationType.Clear();
		CommentaireBuffer.Clear();
		LoadConfiguration(FileOpened);
		return true;
	}
	return false;
}

String EasyConfiguration::toString(){
	String value= "";
	for(const String &key : ConfigurationType.GetKeys()){
		if( ConfigurationType.Get(key).GetTypeName().IsEqual("String") ){
			value  << key <<" : " << ConfigurationType.Get(key).Get<String>() <<"\n";
		}else if(ConfigurationType.Get(key).GetTypeName().IsEqual("int") ){
			value  << key <<" : " << AsString( ConfigurationType.Get(key).Get<int>()) <<"\n";
		}else if(ConfigurationType.Get(key).GetTypeName().IsEqual("bool") ){
			value  << key <<" : " << ((ConfigurationType.Get(key).Get<bool>())? "true":"false") <<"\n";
		}
	}
	return value;
}

void EasyConfiguration::setRC4Key(Upp::String _rc4Key){
	rc4Key =_rc4Key;
}

bool EasyConfiguration::ResolveAndAddLine(String line){
	try{
		if(line[0] != '#'){
			if(line.Find("->",1) != -1){ 
				if(line.Find("=",line.Find("->",1) +3)>(line.Find("->",1)+3)){ // Here I must be sure their is value between -> and =
					String type = ToLower( line.Left(line.Find("->",1)));
					String name = ToLower(line.Mid(line.Find("->",1)+2, line.Find("=",line.Find("->",1)) - (line.Find("->",1)+2) ));
					String value = TrimBoth(line.Right(line.GetCount()-(line.Find("=")+1)));
					if(type.IsEqual("bool")){
					//	value.TrimEnd(" ");
					//	value.TrimStart( " " );
						if(value.Find("b") !=-1 && isStringANumber(value.Right(value.GetCount()-1)) ){
							value.Replace("b","");
							SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false));
						}else if(value.IsEqual("true") || value.IsEqual("false")){
							SetValue<bool>(name, ((value.IsEqual("true"))? true:false));
						}else if (isStringANumber(value)){
							SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false)   );
						}
					}else if(type.IsEqual("int")){
						if(value.GetCount() > 9){
							SetValue<double>(name,std::stoi(value.ToStd()));
						}else if(value.Find(",")!= -1 || value.Find(".") != -1){
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
					}else{
						SetValue<String>(name,value);
					}
					return true;
				}
			}
			else if(line.Find("=",1)>1){
				String name = ToLower(line.Left(line.Find("=")));
				String value = TrimBoth(line.Right(line.GetCount()-(line.Find("=")+1)));
				String type = "";
				if(value.GetCount()> 0 && isStringANumber(value)){
					if(value.GetCount() > 9){
						SetValue<double>(name,std::stod(value.ToStd()));
					}else if(value.Find(",") !=-1 || value.Find(".") !=-1){
						SetValue<float>(name,std::stoi(value.ToStd()));
					}else{
						type="int";
						SetValue<int>(name,std::stoi(value.ToStd()));
					}
				}else if(value.GetCount()> 0 && ((value[0] == 'b' && isStringANumber(value.Right(value.GetCount()-1))) || (value.IsEqual("true") || value.IsEqual("false")))  ){
					if(value.Find("b") !=-1 && isStringANumber(value.Right(value.GetCount()-1)) ){
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



bool EasyConfiguration::isStringANumber(Upp::String stringNumber){
	if (std::isdigit(stringNumber[0]) || (stringNumber.GetCount() > 1 && (stringNumber[0] == '+'))){
        for (int i = 1 ; i < stringNumber.GetCount(); ++i)
            if (!std::isdigit(stringNumber[i]))
                return false;
        return true;
    }
    return false;
}

Upp::String EasyConfiguration::washRC4(Upp::String source){ //it will only remove the @ at front 
	if(source[0] == '@') source.Remove(0);
	if(source.GetCount() > 0 && source[source.GetCount()-1] == '@') source.Remove(source.GetCount()-1);
	return source;
}
bool EasyConfiguration::SaveConfiguration(){
	return SaveConfiguration(FileOpened);
}

void EasyConfiguration::AddCommentaire(int iterator,String commentaire){
	if(CommentaireBuffer.Find(iterator) != -1){
		String buffer=CommentaireBuffer.Get(iterator);
		buffer << "\n" << commentaire;
		CommentaireBuffer.Add(iterator,buffer);
	}
	else{
		CommentaireBuffer.Add(iterator,commentaire);
	}
}

bool EasyConfiguration::SaveConfiguration(String filePath,bool changePath){
 	if(filePath.GetCount() != 0){
		FileOut out(filePath);
		if(out){
			int cpt = 0;
			for(const String &e : ConfigurationType.GetKeys()){
				if(CommentaireBuffer.Find(cpt) != -1){ 
					out << CommentaireBuffer.Get(cpt) << "\n";
				}
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
						Cout() << ConfigurationType.Get(e).Get<String>() <<"\n";
					}
				}else if( ConfigurationType.Get(e).GetTypeName().IsEqual("bool")){
					out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<bool>() <<"\n";
				}else if(  ConfigurationType.Get(e).GetTypeName().IsEqual("int")){
					out << ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName()) << ((SaveInRelaxed)? "":"->") << e << "=" << ConfigurationType.Get(e).Get<int>() <<"\n";
				}
				cpt++;
			}
			if(CommentaireBuffer.Find(cpt) != -1) out << CommentaireBuffer.Get(cpt) <<"\n";
			
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

bool EasyConfiguration::IsBoolean(String str){
	bool ret = false;
	if(str == "true" || str == "false"){
		ret = true;
	}
	return ret;
}


bool EasyConfiguration::IsInteger(String str){
	bool ret = true;
	if(str != ""){
		for(int i=0; i< str.GetLength() ;i++){
			if(!IsDigit(str[i])){
				ret = false;
			}
		}	
	}else{
		ret = false;
	}
	return ret;
}


const String EasyConfiguration::GetFileOpened() const{
	return FileOpened;
}

void EasyConfiguration::SetFileOpened(Upp::String file){
	FileOpened = file;
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
	CommentaireBuffer.Clear();
	for(const int i : ec.GetCommentaireBuffer().GetKeys()){
		AddCommentaire(i,  ec.GetCommentaireBuffer().Get(i) );
	}
	auto &ecConfigurationType=ec.GetConfiguration();
	try{
		for(const String  &key : ecConfigurationType.GetKeys()){
			Cout() << "Key : "<<key <<"\n";
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

void EasyConfiguration::DeleteField(String key){
	if(ConfigurationType.Find(key) >= 0 && ConfigurationType.Find(key) < ConfigurationType.GetCount()){
			Cout() << "Find key : " << ConfigurationType.Find(key) <<EOL;
			ConfigurationType.Remove(ConfigurationType.Find(key),1);
			Cout() << "key " << key <<" removed" <<EOL;
	}
}
void EasyConfiguration::clearConfType(){
	ConfigurationType.Clear();
}

void EasyConfiguration::clearComBuffer(){
	CommentaireBuffer.Clear();
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

void EasyConfiguration::CryptFile(String path){
	Rc4.SetKey(rc4Key);
	if(path.GetCount() != 0){
		FileOut out(path);
		if(out){
			int cpt = 0;
			out << '@';
			String file ="";
			for(const String &e : ConfigurationType.GetKeys()){
				String ligne = "";
				if(CommentaireBuffer.Find(cpt) != -1){ 
					//out << Rc4.Encode(CommentaireBuffer.Get(cpt)) <<"\n";
					file += CommentaireBuffer.Get(cpt)+ "\n";
				}
				if(ConfigurationType.Get(e).GetTypeName().IsEqual("String")){
						ligne += ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName());
						ligne += ((SaveInRelaxed)? "":"->");
						ligne += e+"="+ConfigurationType.Get(e).Get<String>();
				}else if(ConfigurationType.Get(e).GetTypeName().IsEqual("bool")){
					ligne += ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName());
					ligne += ((SaveInRelaxed)? "":"->");
					ligne += e+"="+ConfigurationType.Get(e).ToString();
				}else if(ConfigurationType.Get(e).GetTypeName().IsEqual("int")){
					ligne += ((SaveInRelaxed)? "" : ConfigurationType.Get(e).GetTypeName());
					ligne += ((SaveInRelaxed)? "":"->");
					ligne += e+"="+ConfigurationType.Get(e).ToString();	
				}
				file += ligne +"\n";
				Cout() << ToCharset(CHARSET_DEFAULT,ligne,CHARSET_CP850) << EOL;
				//out <<  Rc4.Encode(ligne) << "\n";
				//Cout() << "ligne crypt : " << ToCharset(CHARSET_DEFAULT,Rc4.Encode(ligne),CHARSET_CP850) << EOL;
				cpt++;
			}
			
			//if(CommentaireBuffer.Find(cpt) != -1) out << Rc4.Encode(CommentaireBuffer.Get(cpt)) << "\n";
			if(CommentaireBuffer.Find(cpt) != -1) file += CommentaireBuffer.Get(cpt)+ "\n";
			Cout() << file<< EOL;
			out << Rc4.Encode(file);
			out.Close();
			if(out.IsError()) { // check whether file was properly written
				LOG("Error");
			}
		}
	}
}

bool EasyConfiguration::ResolveAndAddCryptedLine(String line){
	try{
		//Rc4.SetKey(rc4Key); 
		//Cout() << "ligne crypt : " << ToCharset(CHARSET_DEFAULT,line,CHARSET_CP850) << EOL;
		//line.Trim(line.GetCount()-1);
		//line = Rc4.Encode(line);
		//Cout() << "ligne : " <<  ToCharset(CHARSET_DEFAULT,line,CHARSET_CP850) << EOL;
		//Cout() << " find \\n" << line.Find("\n") << EOL;
		if(line[0] != '#'){
			if(line.Find("->",1) != -1){ 
				if(line.Find("=",line.Find("->",1) +3)>(line.Find("->",1)+3)){ // Here I must be sure their is value between -> and =
					String type = ToLower( line.Left(line.Find("->",1)));
					String name = ToLower(line.Mid(line.Find("->",1)+2, line.Find("=",line.Find("->",1)) - (line.Find("->",1)+2) ));
					String value = TrimBoth(line.Right(line.GetCount()-(line.Find("=")+1)));
					
					if(type.IsEqual("bool")){
					//	value.TrimEnd(" ");
					//	value.TrimStart( " " );
						if(value.Find("b") !=-1 && isStringANumber(value.Right(value.GetCount()-1)) ){
							value.Replace("b","");
							SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false));
						}else if(value.IsEqual("true") || value.IsEqual("false")){
							SetValue<bool>(name, ((value.IsEqual("true"))? true:false));
						}else if (isStringANumber(value)){
							SetValue<bool>(name, ((std::stoi(value.ToStd())!=0)? true:false)   );
						}
					}else if(type.IsEqual("int")){
						if(value.GetCount() > 9){
							SetValue<double>(name,std::stoi(value.ToStd()));
						}else if(value.Find(",")!= -1 || value.Find(".") != -1){
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
					}else{
						SetValue<String>(name,value);
					}
					return true;
				}
			}
			else if(line.Find("=",1)>1){
				String name = ToLower(line.Left(line.Find("=")));
				String value = TrimBoth(line.Right(line.GetCount()-(line.Find("=")+1)));
				String type = "";
				if(value.GetCount()> 0 && isStringANumber(value)){
					if(value.GetCount() > 9){
						SetValue<double>(name,std::stod(value.ToStd()));
					}else if(value.Find(",") !=-1 || value.Find(".") !=-1){
						SetValue<float>(name,std::stoi(value.ToStd()));
					}else{
						type="int";
						SetValue<int>(name,std::stoi(value.ToStd()));
					}
				}else if(value.GetCount()> 0 && ((value[0] == 'b' && isStringANumber(value.Right(value.GetCount()-1))) || (value.IsEqual("true") || value.IsEqual("false")))  ){
					if(value.Find("b") !=-1 && isStringANumber(value.Right(value.GetCount()-1)) ){
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
		Cout()<<"Exception has occured in ResolveAndAddCryptedLine Methode	"<<"\n";
		return false;
	}
	return false;
}