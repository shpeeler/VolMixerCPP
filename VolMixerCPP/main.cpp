#include "volmixer_config.h"
#include "volmixer_helper.h"
#include "volmixer.h"

#include "spdlog/sinks/basic_file_sink.h"
using std::shared_ptr;
using spdlog::logger;

int main()
{
	try
	{
		shared_ptr<logger> logger = spdlog::basic_logger_mt("base_logger", "E:\\logs\\volmixercpp.log");
		logger->set_level(spdlog::level::warn);
		logger->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
		logger->info("logger initialized");
	}
	catch (...)
	{
		return 0;
	}

	auto logger = spdlog::get("base_logger");
	
	
	logger->info("initializing helper");
	auto volmixer_helper = volmixer::VolMixerHelper(logger, "SPDIF Interface (2- FiiO USB DAC-E10)");
	
	std::map<string, string> pin_map = std::map<string, string>();
	pin_map.insert(std::pair<string, string>("14", "Discord.exe"));
	pin_map.insert(std::pair<string, string>("15", "Spotify.exe"));
	pin_map.insert(std::pair<string, string>("16", "chrome.exe"));

	logger->info("initializing mapping");
	std::map<string, list<long>> process_map;
	bool successful = volmixer_helper.TryCreateProcessMapping(pin_map, &process_map);
	if (successful == false)
	{
		return 0;
	}


	logger->info("initializing volmixerconfig");
	auto vol_mixer_config = volmixer::VolMixerConfig("COM3", 9600, 50, "SPDIF Interface (2- FiiO USB DAC-E10)", pin_map, process_map);

	
	logger->info("initializing volmixer");
	auto vol_mixer = volmixer::VolMixer(logger, vol_mixer_config, volmixer_helper);

	logger->info("starting volmixer");
	vol_mixer.Run();

	return 1;
}