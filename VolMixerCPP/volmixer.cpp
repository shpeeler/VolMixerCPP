#include "volmixer.h"
#include "volmixer_config.h"
#include "serialport.h"
#include <utility>

using volmixer::VolMixer;

VolMixer::VolMixer(VolMixerConfig vol_mixer_config)
	:
	vol_mixer_config_(std::move(vol_mixer_config)),
	serial_port(SerialPort(vol_mixer_config.port_name, vol_mixer_config.baudrate))
{
}

void VolMixer::Run()
{
	if (this->serial_port.IsConnected() == false)
	{
		// log
		return;
	}

	char buffer[MAX_DATA_LENGTH];
	this->serial_port.ReadSerialPort(buffer, MAX_DATA_LENGTH);

	
	this->serial_port.CloseSerial();
}


/*
 * HANDLE serial_handle = CreateFile(vol_mixer_config_.port_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (serial_handle == INVALID_HANDLE_VALUE)
			{
				CloseHandle(serial_handle);
				return nullptr;
			}

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
 */
