#pragma once

#include <Windows.h>
#include <list>
#include <string>
#include <utility>
#include <map>

using std::string;
using std::list;
using std::map;

namespace volmixer
{
	class VolMixerConfig
	{

	public:
		LPCWSTR port_name;
		int baudrate;
		int max_retries;
		string device_name;
		map<string, string> pin_map;
		map<string, list<long>> process_map;

		explicit VolMixerConfig(
			const LPCWSTR port_name, const int baudrate, const int max_retries, string device_name, 
			map<string, string> pin_map, map<string, list<long>> process_map)
			:
			port_name(port_name), baudrate(baudrate), max_retries(max_retries), device_name(std::move(device_name)),
			pin_map(std::move(pin_map)), process_map(std::move(process_map))
		{
		}
	};
}
