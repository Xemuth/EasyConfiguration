#ifndef _EasyConfiguration_EasyConfiguration_h_
#define _EasyConfiguration_EasyConfiguration_h_
#include <Core/Core.h>

using namespace Upp;

class EasyConfiguration{
	private:
		VectorMap<String,Value> ConfigurationType;
	public:
		EasyConfiguration();
		
		template <class T> T GetValue(String fieldName);
		template <class T> bool SetValue(String fieldName,const T &t);
};


#endif
