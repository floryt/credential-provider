#pragma once
#include "ConfigParser.h"

//class for singleton log
class SingletonConfig
{
private:
	/* Here will be the instance stored. */
	static SingletonConfig* instance;

	ConfigParser* _config;

	/* Private constructor to prevent instancing. */
	SingletonConfig();


public:
	/* Static access method. */
	static SingletonConfig* getInstance();
	ConfigParser* GetConfigParser();
	~SingletonConfig();
};
#pragma once
