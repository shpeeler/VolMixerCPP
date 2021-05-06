#include "volmixer_helper.h"
#include <iostream>


int main()
{
	auto volmixer_helper = volmixer_helper::VolMixerHelper("SPDIF Interface (2- FiiO USB DAC-E10)");

	std::list<long> process_ids = std::list<long>();
	HRESULT hr = volmixer_helper.TryGetProcessIdsByApplicationName("Spotify.exe", &process_ids);

	return hr;
}
