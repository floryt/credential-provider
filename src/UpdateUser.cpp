#include "UpdateUser.h"
#include <fstream>
#include "dbugLog.h"

void updateCurrentUser(std::string user_email)
{

	std::ofstream ofs("C:\\Users\\User\\Desktop\\current_user.txt", std::ofstream::in | std::ofstream::trunc);
	if (ofs.is_open())
	{
		ofs << user_email;
	}
	ofs.close();
	dbugLog::log_write("updateCurrentUser", "updated current user to " + user_email);
}