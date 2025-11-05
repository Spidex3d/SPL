#pragma once
#include <shobjidl.h>  // for IFileDialog

namespace SpxGui {

	template<typename T>
	inline T Clamp(T v, T lo, T hi) {
		return (v < lo) ? lo : (v > hi) ? hi : v;
	}

	


	std::string OpenFolderDialog(HWND owner = nullptr) {
		std::string folderPath;

		IFileDialog* pfd = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pfd));

		if (SUCCEEDED(hr)) {
			DWORD dwOptions;
			pfd->GetOptions(&dwOptions);
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS); // <-- makes it pick folders

			hr = pfd->Show(owner);
			if (SUCCEEDED(hr)) {
				IShellItem* psi = nullptr;
				hr = pfd->GetResult(&psi);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath = nullptr;
					psi->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					if (pszFilePath) {
						char buffer[MAX_PATH];
						WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, buffer, MAX_PATH, nullptr, nullptr);
						folderPath = buffer;
						CoTaskMemFree(pszFilePath);
					}
					psi->Release();
				}
			}
			pfd->Release();
		}

		return folderPath;
	}

}
