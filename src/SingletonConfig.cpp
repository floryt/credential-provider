#include "SingletonConfig.h"

/* Null, because instance will be initialized on demand. */
SingletonConfig* SingletonConfig::instance = 0;

SingletonConfig* SingletonConfig::getInstance()
{
	//if null - returns the exists one. else - creates instance (prubbly because it's the first time we use it or a delete took place).
	if (instance == 0)
	{
		instance = new SingletonConfig();
	}

	return instance;
}

SingletonConfig::SingletonConfig()
{
	_config = new ConfigParser("C:\\Program Files\\Floryt\\config.txt");
	_config->Parse();
}

SingletonConfig::~SingletonConfig()
{
	delete _config;
}

ConfigParser* SingletonConfig::GetConfigParser()
{
	return _config;
}