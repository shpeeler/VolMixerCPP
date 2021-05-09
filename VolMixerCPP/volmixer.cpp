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

	while (true)
	{
		char buffer[MAX_DATA_LENGTH];
		if (this->serial_port.TryReadSerialPort(buffer, MAX_DATA_LENGTH) == false)
		{
			return;
		}
		
		std::cout << buffer;
	}
}