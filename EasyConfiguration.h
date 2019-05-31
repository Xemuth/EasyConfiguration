#ifndef _EasyConfiguration_EasyConfiguration_h_
#define _EasyConfiguration_EasyConfiguration_h_
#include <Core/Core.h>

using namespace Upp;
class EasyConfiguration{
	private:
		VectorMap<String,Upp::Value> ConfigurationType;
		String pathToLocalConfig="";
		String pathToGlobalConfig=""
	public:
		EasyConfiguration();
		
		template <class T> T GetValue(String fieldName);
		template <class T> bool SetValue(String fieldName, const T &t);
		
		friend void PrepareDefaultConfiguration();
};
#endif
