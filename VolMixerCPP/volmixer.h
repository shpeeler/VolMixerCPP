#pragma once
#include "volmixer_config.h"
#include "volmixer_helper.h"
#include "serialport.h"

#include "spdlog/sinks/basic_file_sink.h"
using std::shared_ptr;
using spdlog::logger;

namespace volmixer
{
	class VolMixer
	{
	private:

		shared_ptr<logger> logger_;
		VolMixerConfig vol_mixer_config_;
		VolMixerHelper vol_mixer_helper_;
		SerialPort serial_port;

		static bool TryParseBuffer(char buffer[MAX_DATA_LENGTH], string& result_string);
		bool TryEnsureOrder(string& value);
		bool TryEnsureProcessIdsExist(list<long>& process_ids, const string& application);

		
		bool TryGetInfoFromSerial(const string value, string& application, list<long>& process_ids, float& volume) const;

	public:
		explicit VolMixer(shared_ptr<logger>& logger, VolMixerConfig vol_mixer_config, VolMixerHelper vol_mixer_helper);

		void Run();
	};
}
