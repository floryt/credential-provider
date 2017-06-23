#include "SingletonLog.h"

/* Null, because instance will be initialized on demand. */
SingletonLog* SingletonLog::instance = 0;

SingletonLog* SingletonLog::getInstance()
{
	//if null - returns the exists one. else - creates instance (prubbly because it's the first time we use it or a delete took place).
	if (instance == 0)
	{
		instance = new SingletonLog();
	}

	return instance;
}

SingletonLog::SingletonLog()
{
	_logger = new Logger("C:\\Program Files\\Floryt\\log.txt");
}

SingletonLog::~SingletonLog()
{
	delete _logger;
}

Logger* SingletonLog::GetLogger()
{
	return _logger;
}