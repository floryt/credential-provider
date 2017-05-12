#include "config.h"

namespace config
{

	std::string get_val(std::string key)
	{
		SingletonConfig* myConfig = SingletonConfig::getInstance();

		if (myConfig)//prevent crash
		{
			return myConfig->GetConfigParser()->GetVal(key);
		}
		dbugLog::log_write("config::get_val", "couldn't create singletonConfig");
		return "";
	}



	void delete_LAST_USE()
	{
		SingletonConfig* myConfig = SingletonConfig::getInstance();
		delete myConfig;
	}
}