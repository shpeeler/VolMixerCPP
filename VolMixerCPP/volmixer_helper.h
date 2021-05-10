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

#include "spdlog/sinks/basic_file_sink.h"
using std::shared_ptr;
using spdlog::logger;

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

		shared_ptr<logger> logger_;
		string device_name_;

		HRESULT TryGetIMMDevice(IMMDevice** pp_device) const;

		HRESULT TryGetISimpleAudioVolumeByProcessId(const UINT process_id, ISimpleAudioVolume** pp_simple_audio_volume) const;

		HRESULT TryGetProcessIds(list<long>* p_process_ids) const;

	public:

		explicit VolMixerHelper(shared_ptr<logger>& logger, string device_name);
		
		bool TrySetApplicationVolume(const UINT process_id, const float volume_level) const;
		
		bool TryGetProcessIdsByApplicationName(const string& application_name, list<long>* p_process_ids) const;

		bool TryCreateProcessMapping(const map<string, string>& pin_map, map<string, list<long>>* process_map) const;

	};
}




