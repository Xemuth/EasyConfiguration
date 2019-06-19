#ifndef _EasyConfiguration_EasyConfiguration_h_
#define _EasyConfiguration_EasyConfiguration_h_
#include <Core/Core.h>

using namespace Upp;
/********************************************************************
EasyConfiguration package is intended to make configuration file easier to use
By reading and casting configuration value to proper valueMap.

Actually EasyConfiguration can bind 4 type of data
 1: String -> It's a basic string (default cast type)
 2: Rc4 String  -> It's basic string but starting with '@' 
 3: Boolean -> Bool can be store as int but with a 'b' forward Or true/False string
 4: Integer -> Basic integer 
     
The package can work with two type of configuration :

	Strict Mode where data type are defined.
	In this mode, Every binded data will be casting to valide type in code :
		string->type=global
		rc4->instance=@Here is RC4 String //Even if rc4 string data type is specified , the string must start with '@'
		string->usrOdigo=username
		string->pwdOdigo=password
		bool->version=1
		int->updateTime=3
		int->autoOdigo=2
		bool->autoLog=0
		bool->autoDelog=0
		string->block=
	
	
	the second configuration is
	Relaxed Mode where data type are no defined and resolved by programm :
		type=global             //Here programm resolve it as String
		instance=@string rc4    //SInce this data start by '@', programm resolve it as RC4 string
		windows=b1              //SInce this data start by 'b' and is pure int after the first charactere, programm resolve it as Boolean (if first char was not 'b' but int so the programm would resolve it as integer
		updateTime=3            //Since this data is pure integer, programm resolve it as integer
		block=                  //Here the data Value is empty so the default cast type is String

Project created 17/06/2019
By Clément Hamon 
This project have to be used with Ultimate++ FrameWork and required the Core Librairy from it
http://www.ultimatepp.org
Copyright © 1998, 2019 Ultimate++ team
All those sources are contained in "plugin" directory. Refer there for licenses, however all libraries have BSD-compatible license.
Ultimate++ has BSD license:
License : https://www.ultimatepp.org/app$ide$About$en-us.html
Thanks to UPP team !
**********************************************************************/

class EasyConfiguration{
	private:
		VectorMap<String,Upp::Value> ConfigurationType;
		
		bool SaveInRelaxed =true; //If set to false then the save mode will be "Strict Mode"
		
		String FileOpened=""; //Save when LoadConfiguration is used
		
		bool ResolveAndAddLine(String line);
		bool isStringisANumber(String stringNumber);
	protected:
		const String GetFileOpened();
		
		const VectorMap<String,Upp::Value>& GetConfiguration()const;
	public:
		EasyConfiguration();
		EasyConfiguration(Upp::String FilePath);
		EasyConfiguration(const EasyConfiguration& ec); //Copy constructor 
		
		bool NewConfiguration(const EasyConfiguration& ec); //Used to copy configuration from ec to this
		bool UpdateConfigurationFromMaster(const EasyConfiguration& ec); //Used to update all this value by ec value (do not add new value to this, just update existing value)
		bool MergeUpdateConfiguration(const EasyConfiguration& ec); //Add and update every new configuration value
		bool MergeConfiguration(const EasyConfiguration& ec); //Add every new tag to actual configuration
		
		void RelaxMode(bool b);
		bool isRelaxMode();
		int GetCount();
		
		bool SaveConfiguration();
		EasyConfiguration SaveConfiguration(String filePath);
		
		template <class T>  //Return value from ConfigrationType
		T GetValue(String fieldName){
			int index = ConfigurationType.Find(fieldName);
			if(index >= 0 && ConfigurationType[index].Is<T>()){
				return (T)ConfigurationType[index].Get<T>();
			}
			return T();
		}
		
		
		template <class T> 
		bool SetValue(String fieldName, const T &t){
			try{
				if(&ConfigurationType.Add(fieldName,Value(t)))
					return true;
			}catch(Upp::ValueTypeError &e){
				throw e;	
			}
			return false;
		} 
		
		int LoadConfiguration(String FilePath); //Return number of configuration loaded
		
		friend void PrepareDefaultConfiguration();
};
#endif
