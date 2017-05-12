#pragma once
#include "Logger.h"

//class for singleton log
class SingletonLog
{
private:
	/* Here will be the instance stored. */
	static SingletonLog* instance;

	Logger* _logger;

	/* Private constructor to prevent instancing. */
	SingletonLog();


public:
	/* Static access method. */
	static SingletonLog* getInstance();
	Logger* GetLogger();
	~SingletonLog();
};
