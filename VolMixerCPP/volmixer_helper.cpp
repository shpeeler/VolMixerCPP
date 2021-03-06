#include "volmixer_helper.h"

using volmixer::VolMixerHelper;

/// <summary>
/// creates an instance of VolMixerHelper
/// </summary>
/// <param name="device_name">the full audio-device name | Device-FriendlyName</param>
/// <param name="logger">reference to spdlog::logger</param>
VolMixerHelper::VolMixerHelper(shared_ptr<logger>& logger, string device_name) : logger_(logger), device_name_(std::move(device_name))
{
}

/// <summary>
/// tries to get an IMMDevice interface
/// </summary>
/// <param name="pp_device">pointer to an IMMDevice interface</param>
/// <returns>true if successful, false if not</returns>
HRESULT VolMixerHelper::TryGetIMMDevice(IMMDevice** pp_device) const
{
	HRESULT hr = S_OK;
	CComPtr<IMMDeviceEnumerator> p_enumerator;
	CComPtr<IMMDeviceCollection> p_collection;
	UINT device_count = 0;

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&p_enumerator));

	if (SUCCEEDED(hr))
	{
		hr = p_enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &p_collection);
	}
	if (SUCCEEDED(hr))
	{
		hr = p_collection->GetCount(&device_count);
	}
	if (SUCCEEDED(hr))
	{
		for (UINT i = 0; i < device_count; ++i)
		{
			CComPtr<IMMDevice> p_device;
			CComPtr<IPropertyStore> p_property_store;
			PROPVARIANT var_name;
			PropVariantInit(&var_name);

			hr = p_collection->Item(i, &p_device);

			if (SUCCEEDED(hr))
			{
				hr = p_device->OpenPropertyStore(STGM_READ, &p_property_store);
			}

			if (SUCCEEDED(hr))
			{
				hr = p_property_store->GetValue(PKEY_Device_FriendlyName, &var_name);
			}
			if (SUCCEEDED(hr))
			{
				std::wstring ws(var_name.pwszVal);
				std::string each_device_name(ws.begin(), ws.end());

				if ((each_device_name == device_name_) == true)
				{
					hr = p_device.CopyTo(pp_device);

					PropVariantClear(&var_name);
					return hr;
				}
			}

			PropVariantClear(&var_name);
		}
	}

	return E_POINTER;
}

/// <summary>
/// tries to get the ISimpleAudioVolume interface of an application by its process-id
/// </summary>
/// <param name="process_id">process-id</param>
/// <param name="pp_simple_audio_volume">pointer to an ISimpleAudioVolume interface</param>
/// <returns>true if successful, false if not</returns>
HRESULT VolMixerHelper::TryGetISimpleAudioVolumeByProcessId(const UINT process_id, ISimpleAudioVolume** pp_simple_audio_volume) const
{
	CComPtr<IMMDevice> p_device;
	HRESULT hr = TryGetIMMDevice(&p_device);

	CComPtr<IAudioSessionManager2> p_audio_session_manager2;
	CComPtr<IAudioSessionEnumerator> p_audio_session_enumerator;
	int p_session_count;

	if (SUCCEEDED(hr))
	{
		hr = p_device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&p_audio_session_manager2));
	}
	if (SUCCEEDED(hr))
	{
		hr = p_audio_session_manager2->GetSessionEnumerator(&p_audio_session_enumerator);
	}
	if (SUCCEEDED(hr))
	{
		hr = p_audio_session_enumerator->GetCount(&p_session_count);
	}
	if (SUCCEEDED(hr))
	{
		for (int i = 0; i < p_session_count; ++i)
		{
			CComPtr<IAudioSessionControl> p_audio_session_control;
			CComPtr<IAudioSessionControl2> p_audio_session_control2;
			DWORD each_process_id = NULL;
			CComPtr<ISimpleAudioVolume> p_simple_audio_volume;

			hr = p_audio_session_enumerator->GetSession(i, &p_audio_session_control);

			if (SUCCEEDED(hr))
			{
				hr = p_audio_session_control.QueryInterface(&p_audio_session_control2);
			}
			if (SUCCEEDED(hr))
			{
				hr = p_audio_session_control2->GetProcessId(&each_process_id);
			}
			if (SUCCEEDED(hr))
			{
				if (each_process_id != NULL && each_process_id == process_id)
				{
					hr = p_audio_session_control.QueryInterface(&p_simple_audio_volume);

					if (SUCCEEDED(hr))
					{
						return p_simple_audio_volume.CopyTo(pp_simple_audio_volume);
					}
				}
			}
		}
	}

	return E_POINTER;
}

/// <summary>
/// tries to get all ids of processes currently running in the audio-mixer-session
/// </summary>
/// <param name="p_process_ids">reference to process-ids</param>
/// <returns>true if successful, false if not</returns>
HRESULT VolMixerHelper::TryGetProcessIds(list<long>* p_process_ids) const
{
	CComPtr<IMMDevice> p_device;
	CComPtr<IAudioSessionManager2> p_audio_session_manager2;
	CComPtr<IAudioSessionEnumerator> p_audio_session_enumerator;
	int p_session_count = 0;

	HRESULT hr = TryGetIMMDevice(&p_device);

	if (SUCCEEDED(hr))
	{
		hr = p_device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&p_audio_session_manager2));
	}
	if (SUCCEEDED(hr))
	{
		hr = p_audio_session_manager2->GetSessionEnumerator(&p_audio_session_enumerator);
	}
	if (SUCCEEDED(hr))
	{
		hr = p_audio_session_enumerator->GetCount(&p_session_count);
	}
	if (SUCCEEDED(hr))
	{
		for (int i = 0; i < p_session_count; ++i)
		{
			CComPtr<IAudioSessionControl> p_audio_session_control;
			CComPtr<IAudioSessionControl2> p_audio_session_control2;
			DWORD each_process_id = NULL;

			hr = p_audio_session_enumerator->GetSession(i, &p_audio_session_control);

			if (SUCCEEDED(hr))
			{
				hr = p_audio_session_control.QueryInterface(&p_audio_session_control2);
			}
			if (SUCCEEDED(hr))
			{
				hr = p_audio_session_control2->GetProcessId(&each_process_id);
			}

			if (SUCCEEDED(hr) && each_process_id != NULL)
			{
				p_process_ids->push_front(each_process_id);
			}
		}
	}

	return hr;
}

/// <summary>
/// tries to set the volume-level for a application by it's process-id
/// </summary>
/// <param name="process_id">process-id</param>
/// <param name="volume_level">volume-level</param>
/// <returns>true if successful, false if not</returns>
bool VolMixerHelper::TrySetApplicationVolume(const UINT process_id, const float volume_level) const
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	CComPtr<ISimpleAudioVolume> p_simple_audio_volume;
	hr = TryGetISimpleAudioVolumeByProcessId(process_id, &p_simple_audio_volume);
	if (SUCCEEDED(hr))
	{
		return SUCCEEDED(p_simple_audio_volume->SetMasterVolume(volume_level / 100, nullptr));
	}

	CoUninitialize();
	return SUCCEEDED(hr);
}

/// <summary>
/// tries to get all process-ids for a application by it's name
/// </summary>
/// <param name="application_name">application name</param>
/// <param name="p_process_ids">process-ids</param>
/// <returns>true if successful, false if not</returns>
bool VolMixerHelper::TryGetProcessIdsByApplicationName(const string& application_name, list<long>* p_process_ids) const
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	list<long> process_ids = list<long>();

	if (SUCCEEDED(hr))
	{
		hr = TryGetProcessIds(&process_ids);
	}
	if (SUCCEEDED(hr))
	{
		for (auto const& i : process_ids)
		{
			HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, i);
			if (h_process != nullptr)
			{
				HMODULE h_module;
				DWORD cb_needed;

				if (EnumProcessModules(h_process, &h_module, sizeof(h_module), &cb_needed))
				{
					TCHAR process_name[MAX_PATH];

					if (GetModuleBaseName(h_process, h_module, process_name, sizeof(process_name) / sizeof(TCHAR)))
					{
						wstring ws(process_name);
						string each_process_name(ws.begin(), ws.end());

						if ((each_process_name == application_name) == true)
						{
							p_process_ids->push_front(i);
						}
					}
				}

				CloseHandle(h_process);
			}
		}
	}

	CoUninitialize();
	return SUCCEEDED(hr);
}

/// <summary>
/// tries to create a map
///		-> first: application name
///		-> process-ids
/// </summary>
/// <param name="pin_map">pin-mapping <pin-id, application name></param>
/// <param name="process_map">process-mapping <application name, process-ids></param>
/// <returns>true if successful, false if not</returns>
bool VolMixerHelper::TryCreateProcessMapping(const map<string, string>& pin_map, map<string, list<long>>* process_map) const
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	
	CComPtr<IMMDevice> p_device;
	if (SUCCEEDED(hr))
	{
		hr = TryGetIMMDevice(&p_device);
	}

	bool successful = false;
	for (auto const& each_mapping : pin_map)
	{
		string pin_name = each_mapping.first;
		string application_name = each_mapping.second;

		if (application_name.empty() == true)
		{
			continue;
		}

		list<long> result_process_ids = list<long>();
		successful = TryGetProcessIdsByApplicationName(application_name, &result_process_ids);
		if (SUCCEEDED(hr) && successful)
		{
			process_map->insert(pair<string, list<long>>(application_name, result_process_ids));
		}
	}

	CoUninitialize();
	return (SUCCEEDED(hr) && successful);
}
