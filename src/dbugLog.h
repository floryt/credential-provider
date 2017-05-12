#pragma once
#ifndef DBUGLOG
#define DBUGLOG
#include "SingletonLog.h"


namespace dbugLog
{
	
	void log_write(std::string foo, std::string messgae);

	void delete_LAST_USE();


}

#endif

