#pragma once

#include <atlbase.h>
#include <mmdeviceapi.h>
#include <propvarutil.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>
#include <processthreadsapi.h>
#include <Psapi.h>
#include <list>
#include <string>
#include <utility>
#include <map>

using std::string;
using std::wstring;
using std::list;
using std::map;
using std::pair;

namespace volmixer
{
	class VolMixerHelper
	{

	private:

		string device_name_;

		HRESULT TryGetIMMDevice(IMMDevice** pp_device) const;

		HRESULT TryGetISimpleAudioVolumeByProcessId(const UINT process_id, ISimpleAudioVolume** pp_simple_audio_volume) const;

		HRESULT TryGetProcessIds(list<long>* p_process_ids) const;

	public: // TODO:logging

		explicit VolMixerHelper(string device_name);
		
		HRESULT TrySetApplicationVolume(const UINT process_id, const float volume_level) const;

		HRESULT TryGetProcessIdsByApplicationName(const string& application_name, list<long>* p_process_ids) const;

		HRESULT TryCreateProcessMapping(const map<string, string>& pin_map, map<string, list<long>>* process_map) const;

	};
}




