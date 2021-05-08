#include "volmixer_config.h"
#include "volmixer_helper.h"
#include "volmixer.h"

int main()
{
	auto volmixer_helper = volmixer::VolMixerHelper("SPDIF Interface (2- FiiO USB DAC-E10)");

	std::map<string, string> pin_map = std::map<string, string>();
	pin_map.insert(std::pair<string, string>("Pin_0", "Discord.exe"));
	pin_map.insert(std::pair<string, string>("Pin_1", "Spotify.exe"));
	pin_map.insert(std::pair<string, string>("Pin_2", "chrome.exe"));


	std::map<string, list<long>> process_map;
	HRESULT hr = volmixer_helper.TryCreateProcessMapping(pin_map, &process_map);
	auto vol_mixer_config = volmixer::VolMixerConfig(L"COM3", 9600, 50, "SPDIF Interface (2- FiiO USB DAC-E10)", pin_map, process_map);

	auto vol_mixer = volmixer::VolMixer(vol_mixer_config);

	vol_mixer.Run();

	return hr;
}