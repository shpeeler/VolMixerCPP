#pragma once
#include "volmixer_config.h"
#include "volmixer_helper.h"
#include "serialport.h"

namespace volmixer
{
	class VolMixer
	{
	private:
		VolMixerConfig vol_mixer_config_;
		SerialPort serial_port;
		
		bool TryGetInfoFromSerial(string value, string* application, list<long>* process_ids, float* volume);

	public:
		explicit VolMixer(VolMixerConfig vol_mixer_config);

		void Run();
	};
}
