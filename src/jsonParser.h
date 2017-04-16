#pragma once
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>

namespace jsonParser
{
	std::map<std::string, std::string> ParseJson(char* json_c);

	std::string CreateJson(std::unordered_map<std::string, std::string> map_json);
}
