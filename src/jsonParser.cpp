#include "jsonParser.h"
#include <vector>

namespace jsonParser
{
	std::map<std::string, std::string> ParseJson(char* json_c)
	{

		//exmaple - {"access":false,"message":"Custom massage from admin"}

		std::string json(json_c);

		json = json.substr(1, json.length() - 2); //removing [{] [}]

		std::vector<std::string> vec;

		char sep = ','; //seperates blocks
		std::string temp = "";
		for (int i = 0; i < json.length(); i++)
		{
			if (json[i] != sep)
			{
				temp += json[i];
			}
			else
			{
				vec.push_back(temp);
				temp = "";
			}
		}
		vec.push_back(temp);


		std::map<std::string, std::string> to_return;

		std::vector<std::string> temp_vec;

		for (int j = 0; j < vec.size(); j++) //"access":false
		{
			temp_vec.clear();

			char sep = ':'; //seperates blocks
			temp = "";
			for (int i = 0; i < vec[j].length(); i++)
			{
				if (vec[j][i] != sep && vec[j][i] != '\"') //TODO: check " better
				{
					temp += vec[j][i];
				}
				else if (vec[j][i] == sep)
				{
					temp_vec.push_back(temp);
					temp = "";
				}
			}
			temp_vec.push_back(temp);

			to_return.insert(std::pair<std::string, std::string>(temp_vec[0], temp_vec[1]));
		}



		return to_return;


	}

	std::string CreateJson(std::unordered_map<std::string, std::string> map_json)
	{
		std::string to_return = "{";

		for (std::unordered_map<std::string, std::string>::const_iterator it = map_json.begin(); it != map_json.end(); ++it)
		{
			to_return += it->first;
			to_return += ":";
			to_return += it->second;
			to_return += ",";
		}
		to_return.erase(to_return.end() - 1);

		to_return += "}";

		return to_return;
	}
}