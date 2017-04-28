#include "ConfigParser.h"
#include <Shlwapi.h>
#include <algorithm>

//TODO: get the path from registry
ConfigParser::ConfigParser(std::string path, Logger* log)
{
	/*TCHAR buffer[500];
	DWORD bufferSize = sizeof(buffer);*/
	_file.open(path.c_str());  //IMPORTANT TODO - check how to know if file filed
	/*std::cout << "opened file" << std::endl;*/
	_map = new map_config();
	_log = log;
}

ConfigParser::~ConfigParser()
{
	delete _map;
	_file.close();
}

bool ConfigParser::IsOpen()
{
	return _file.is_open();
}

void ConfigParser::Parse()
{
	std::string line;
	std::string key = "";
	std::string val = "";
	if (!_file.is_open())
	{
		_log->Write("ConfigParser::Parse", "no config file");
	}
	else
	{
		while (std::getline(_file, line))
		{
			key = "";
			val = "";

			//-----creating the key
			for (int i = 0; i < line.length() && line[i] != ' '; i++)
			{
				key += line[i];
			}
			//----creating the val
			for (int i = line.length() - 2; i >= 0 && (line[i] != '"'); i--)
			{
				val += line[i];
			}
			std::reverse(val.begin(), val.end());

			//std::cout << "key: " << key << " val: " << val << std::endl;

			//----inserting to map
			//std::string text = "\"" + val + '"';
			_map->insert(std::pair<std::string, std::string>(key, val));
		}
	}

}

std::string ConfigParser::GetVal(std::string key)
{
	std::string to_return = "none";
	if (_file.is_open())
	{
		map_config::iterator it;
		it = _map->find(key);
		if (it != _map->end())
		{
			to_return = it->second;
			_log->Write("ConfigParser::GetVal", "found value");
		}
	}
	_log->Write("ConfigParser::GetVal", "returned value " + to_return);
	return to_return;

}

