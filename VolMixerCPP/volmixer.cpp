#include "volmixer.h"
#include "volmixer_config.h"
#include "serialport.h"
#include <utility>

using volmixer::VolMixer;

/// <summary>
/// creates an instance of VolMixer
/// </summary>
/// <param name="logger">reference to spdlog::logger</param>
/// <param name="vol_mixer_config">instance of VolMixerConfig</param>
/// <param name="vol_mixer_helper">instance of VolMixerHelper</param>
VolMixer::VolMixer(shared_ptr<logger>& logger, VolMixerConfig vol_mixer_config, VolMixerHelper vol_mixer_helper)
	:
	logger_(logger),
	vol_mixer_config_(std::move(vol_mixer_config)),
	vol_mixer_helper_(std::move(vol_mixer_helper)),
	serial_port(SerialPort(logger, vol_mixer_config.port_name, vol_mixer_config.baudrate))
{
}

/// <summary>
/// runs the VolMixer
/// </summary>
void VolMixer::Run()
{
	if (this->serial_port.IsConnected() == false)
	{
		logger_->error("serial-port not connected");
		return;
	}

	while (true)
	{
		//
		// read serial input
		//

		char buffer[MAX_DATA_LENGTH];
		if (this->serial_port.TryReadSerialPort(buffer, MAX_DATA_LENGTH) == false)
		{
			logger_->error("unable to read from serial-port");
			continue;
		}

		//
		// parse serial input
		//

		string result_string;
		if (TryParseBuffer(buffer, result_string) == false)
		{
			logger_->error("unable to parse buffer for string: {0}", result_string);
			continue;
		}

		//
		// serial read can mix up the input order, therefore: ensure order is as expected
		//	

		if (TryEnsureOrder(result_string) == false)
		{
			logger_->error("unable to ensure order for string: {0}", result_string);
			continue;
		}

		//
		// get all necessary from serial input
		//

		string application;
		list<long> process_ids;
		float volume;
		if (TryGetInfoFromSerial(result_string, application, process_ids, volume) == false)
		{
			logger_->error("unable to get all necessary information from string: {0}", result_string);
			continue;
		}

		//
		// ensure all processes exist or reload if not
		//
		
		if(TryEnsureProcessIdsExist(process_ids, application) == false)
		{
			logger_->error("unable to ensure process ids for application: {0}", application);
			continue;
		}

		//
		// set audio-volume for each process
		//

		for (auto each_process_id : process_ids)
		{
			if (this->vol_mixer_helper_.TrySetApplicationVolume(each_process_id, volume) == false)
			{
				logger_->error("unable to set volume for application: {0} with id: {1}", application, each_process_id);
			}
		}
	}
}

/// <summary>
/// tries to gather all needed information from the serial input
/// 	d => data
///		p => pin
///		v => volume
///					->	dddd
///						ppvv	
/// </summary>
/// <param name="value">serial input</param>
/// <param name="application">target application name</param>
/// <param name="process_ids">target process-ids</param>
/// <param name="volume">target volume</param>
/// <returns>true if successful, false if not</returns>
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

/// <summary>
/// tries to ensure the correct data-order read from the byte-stream
/// </summary>
/// <param name="value">serial input</param>
/// <returns>true if successful, false if not</returns>
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

/// <summary>
/// tries to ensure that processes exist for given process-ids, if not they will be reloaded based on application name
/// </summary>
/// <param name="process_ids">reference to process-ids</param>
/// <param name="application"></param>
/// <returns>true if successful, false if not</returns>
bool VolMixer::TryEnsureProcessIdsExist(list<long>& process_ids, const string& application)
{
	// self-repair mechanism - if no process-ids are found, try to reload them
	if (process_ids.empty())
	{
		list<long> new_process_ids;
		if (this->vol_mixer_helper_.TryGetProcessIdsByApplicationName(application, &new_process_ids) == false)
		{
			return false;
		}
		process_ids = new_process_ids;
		this->vol_mixer_config_.process_map[application] = new_process_ids;
	}

	// ensure that all processes present in mapping exist
	for (auto each_process_id : process_ids)
	{
		HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, each_process_id);
		if (h_process != nullptr)
		{
			if (WaitForSingleObject(h_process, 0))
			{
				CloseHandle(h_process);
				continue;
			}
		}

		//
		// self-repair mechanism
		// if process does not exist, reload it
		//

		list<long> new_process_ids;
		if (this->vol_mixer_helper_.TryGetProcessIdsByApplicationName(application, &new_process_ids) == false)
		{
			// no proc ids found after reload
			return false;
		}
		process_ids = new_process_ids;
		this->vol_mixer_config_.process_map[application] = new_process_ids;

		break;
	}
	
	return true;
}

/// <summary>
/// tries to parse the serial buffer to a usable string
/// </summary>
/// <param name="buffer">serial input</param>
/// <param name="result_string">parsed string</param>
/// <returns>true if successful, false if not</returns>
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
