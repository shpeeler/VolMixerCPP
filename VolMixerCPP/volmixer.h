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
		VolMixerHelper vol_mixer_helper_;
		SerialPort serial_port;

		static bool TryParseBuffer(char buffer[MAX_DATA_LENGTH], string& result_string);
		bool TryEnsureOrder(string& value);
		bool TryGetInfoFromSerial(const string value, string& application, list<long>& process_ids, float& volume) const;

	public:
		explicit VolMixer(VolMixerConfig vol_mixer_config, VolMixerHelper vol_mixer_helper);

		void Run();
	};
}
