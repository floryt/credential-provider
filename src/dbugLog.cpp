#include "dbugLog.h"

namespace dbugLog
{
	void log_write(std::string foo, std::string messgae)
	{
		SingletonLog* myLog = SingletonLog::getInstance();

		if (myLog)//prevent crash
		{
			myLog->GetLogger()->Write(foo, messgae);
		}

	}

	void delete_LAST_USE()
	{
		SingletonLog* myLog = SingletonLog::getInstance();
		delete myLog;
	}
}