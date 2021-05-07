#pragma once
#include "volmixer_config.h"
#include "volmixer_helper.h"

namespace volmixer
{
	class VolMixer
	{
	private:
		VolMixerConfig vol_mixer_config_;

		HANDLE TryOpenPort() const
		{
			HANDLE serial_handle = CreateFile(vol_mixer_config_.port_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (serial_handle == INVALID_HANDLE_VALUE)
			{
				CloseHandle(serial_handle);
				return nullptr;
			}

			return serial_handle;
		}

		bool TryGetInfoFromSerial(string value, string* application, list<long>* process_ids, float* volume)
		{


			return true;
		}
	public:
		explicit VolMixer(VolMixerConfig vol_mixer_config) : vol_mixer_config_(std::move(vol_mixer_config))
		{
		}

		void Run()
		{
			HANDLE serial_handle = TryOpenPort();
			if (serial_handle == nullptr)
			{
				return;
			}

			DCB serial_params = { 0 };
			serial_params.DCBlength = sizeof(serial_params);

			if (GetCommState(serial_handle, &serial_params) == false)
			{
				CloseHandle(serial_handle);
				return;
			}
			
			serial_params.BaudRate = vol_mixer_config_.baudrate;

			if (SetCommState(serial_handle, &serial_params) == false)
			{
				CloseHandle(serial_handle);
				return;
			}
			
			CloseHandle(serial_handle);
		}
	};
}
