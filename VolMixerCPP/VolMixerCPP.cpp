#include <iostream>
#include <mmdeviceapi.h>
#include <atlbase.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#include <Audioclient.h>
#include <audiopolicy.h>

int TryFindIMMDeviceByName(std::string aDeviceName, IMMDevice** ppDevice)
{
	HRESULT hr = S_OK;

	if (SUCCEEDED(hr))
	{
		CComPtr<IMMDeviceEnumerator> pEnumerator;
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&pEnumerator));

		if (SUCCEEDED(hr))
		{
			CComPtr<IMMDeviceCollection> pCollection;
			hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);

			if (SUCCEEDED(hr))
			{
				UINT deviceCount = 0;
				hr = pCollection->GetCount(&deviceCount);

				if (SUCCEEDED(hr))
				{
					for (UINT i = 0; i < deviceCount; ++i)
					{
						CComPtr<IMMDevice> pDevice;
						hr = pCollection->Item(i, &pDevice);

						if (SUCCEEDED(hr))
						{
							CComPtr<IPropertyStore> pPropertyStore;
							hr = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);

							if (SUCCEEDED(hr))
							{
								PROPVARIANT varName;
								PropVariantInit(&varName);

								hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &varName);

								if (SUCCEEDED(hr))
								{
									std::wstring ws(varName.pwszVal);
									std::string currentDeviceName(ws.begin(), ws.end());

									if ((currentDeviceName == aDeviceName) == true)
									{

										hr = pDevice.CopyTo(ppDevice);

										PropVariantClear(&varName);
										return true;
									}
								}

								PropVariantClear(&varName);
							}
						}
					}
				}
			}
		}
	}

	return false;
}

int TryGetISimpleAudioVolumeByProcessId(UINT aProcessId, std::string aDeviceName, ISimpleAudioVolume** ppSimpleAudioVolume)
{
	HRESULT hr = S_OK;

	CComPtr<IMMDevice> pDevice;
	int result = TryFindIMMDeviceByName(aDeviceName, &pDevice);

	CComPtr<IAudioSessionManager2> pIAudioSessionManager2;
	hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&pIAudioSessionManager2));
	if (SUCCEEDED(hr))
	{
		CComPtr<IAudioSessionEnumerator> pAudioSessionEnumerator;
		hr = pIAudioSessionManager2->GetSessionEnumerator(&pAudioSessionEnumerator);

		if (SUCCEEDED(hr))
		{
			int sessionCount;
			hr = pAudioSessionEnumerator->GetCount(&sessionCount);

			if (SUCCEEDED(hr))
			{
				for (int i = 0; i < sessionCount; ++i)
				{
					CComPtr<IAudioSessionControl> pAudioSessionControl;
					hr = pAudioSessionEnumerator->GetSession(i, &pAudioSessionControl);

					if (SUCCEEDED(hr))
					{
						CComPtr<IAudioSessionControl2> pAudioSessionControl2;
						hr = pAudioSessionControl.QueryInterface(&pAudioSessionControl2);

						if (SUCCEEDED(hr))
						{
							DWORD processId;
							hr = pAudioSessionControl2->GetProcessId(&processId);

							if (processId == aProcessId)
							{
								CComPtr<ISimpleAudioVolume> pSimpleAudioVolume;
								hr = pAudioSessionControl.QueryInterface(&pSimpleAudioVolume);
								
								if (SUCCEEDED(hr))
								{
									hr = pSimpleAudioVolume.CopyTo(ppSimpleAudioVolume);
									return SUCCEEDED(hr);
								}
							}
						}
					}
				}
			}
		}
	}

	return false;
}

int TrySetApplicationVolume(UINT aProcessId, std::string aDeviceName, float aVolumeLevel)
{
	CComPtr<ISimpleAudioVolume> pSimpleAudioVolume;
	int result = TryGetISimpleAudioVolumeByProcessId(aProcessId, aDeviceName, &pSimpleAudioVolume);
	if (result == 1)
	{
		return pSimpleAudioVolume->SetMasterVolume(aVolumeLevel / 100, nullptr);
	}
	
	return false;
}


int main()
{
	std::cout << "Hello World!\n";
	
	HRESULT hr = S_OK;
	hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	int result = TrySetApplicationVolume(12672, "SPDIF Interface (2- FiiO USB DAC-E10)", 90);

	CoUninitialize();
	return result;
}
