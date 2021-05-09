#include "volmixer.h"
#include "volmixer_config.h"
#include "serialport.h"
#include <utility>

using volmixer::VolMixer;

VolMixer::VolMixer(VolMixerConfig vol_mixer_config, VolMixerHelper vol_mixer_helper)
	:
	vol_mixer_config_(std::move(vol_mixer_config)),
	vol_mixer_helper_(std::move(vol_mixer_helper)),
	serial_port(SerialPort(vol_mixer_config.port_name, vol_mixer_config.baudrate))
{
}

void VolMixer::Run()
{
	if (this->serial_port.IsConnected() == false)
	{
		return;
	}

	while (true)
	{
		char buffer[MAX_DATA_LENGTH];
		if (this->serial_port.TryReadSerialPort(buffer, MAX_DATA_LENGTH) == false)
		{
			continue;
		}

		string result_string;
		if (TryParseBuffer(buffer, result_string) == false)
		{
			continue;
		}

		if (TryEnsureOrder(result_string) == false)
		{
			continue;
		}

		string application;
		list<long> process_ids;
		float volume;
		if (TryGetInfoFromSerial(result_string, application, process_ids, volume) == false)
		{
			continue;
		}

		if (process_ids.empty())
		{
			list<long> new_process_ids;
			if (SUCCEEDED(this->vol_mixer_helper_.TryGetProcessIdsByApplicationName(application, &new_process_ids)) == false) // TODO: return bool, not hresult from pub methods
			{
				// no proc ids found after reload
				continue;
			}
			process_ids = new_process_ids;
			this->vol_mixer_config_.process_map[application] = new_process_ids; // TODO: test
		}

		for (auto each_process_id : process_ids)
		{
			// TODO: check if process exists, if false reload
		}

		for (auto each_process_id : process_ids)
		{
			if (SUCCEEDED(this->vol_mixer_helper_.TrySetApplicationVolume(each_process_id, volume)) == false)
			{
				// log
			}
		}
	}
}

bool VolMixer::TryGetInfoFromSerial(const string value, string& application, list<long>& process_ids, float& volume) const
{
	string pin = value.substr(0, 2);
	string vol = value.substr(2, 2);

	auto result_pin_kvp = this->vol_mixer_config_.pin_map.find(pin);
	if (result_pin_kvp == vol_mixer_config_.pin_map.end())
	{
		return false;
	}
	
	string result_application = result_pin_kvp->second;
	if (result_application.empty() == true)
	{
		return false;
	}

	auto result_process_kvp = this->vol_mixer_config_.process_map.find(result_application);
	if (result_process_kvp == this->vol_mixer_config_.process_map.end())
	{
		return false;
	}
	list<long> result_process_ids = result_process_kvp->second;

	float result_vol;
	try
	{
		result_vol = std::stof(vol);
	}
	catch (...)
	{
		return false;
	}
	
	application = result_application;
	process_ids = result_process_ids;
	volume = result_vol;
	
	return true;
}

bool VolMixer::TryEnsureOrder(string& value)
{
	string first_two_digits = value.substr(0, 2);
	string last_two_digits = value.substr(2, 2);

	if (this->vol_mixer_config_.pin_map.find(first_two_digits) == this->vol_mixer_config_.pin_map.end() == false)
	{
		return true;
	}

	if (this->vol_mixer_config_.pin_map.find(last_two_digits) == this->vol_mixer_config_.pin_map.end() == false)
	{
		value = last_two_digits + first_two_digits;
		return true;
	}

	return false;
}

bool VolMixer::TryParseBuffer(char buffer[MAX_DATA_LENGTH], string& result_string)
{
	for (int i = 0; i < MAX_DATA_LENGTH; ++i)
	{
		char x = buffer[i];

		if (x == '\n' || x == '\r')
		{
			continue;
		}

		result_string += x;

		if (result_string.length() == 4)
		{
			break;
		}
	}
	
	return result_string.length() == 4;
}
