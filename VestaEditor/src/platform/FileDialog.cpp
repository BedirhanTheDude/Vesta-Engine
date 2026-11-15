#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shobjidl.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <platform/FileDialog.h>

bool FileDialog::pickFolder(GLFWwindow* owner, std::filesystem::path& outFolder) {
	HRESULT initResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(initResult) && initResult != RPC_E_CHANGED_MODE) return false;
	const bool uninit = SUCCEEDED(initResult);

	bool picked = false;

	IFileOpenDialog* dialog = nullptr;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
		DWORD options = 0;
		dialog->GetOptions(&options);
		dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);

		HWND ownerWindow = owner ? glfwGetWin32Window(owner) : nullptr;

		if (SUCCEEDED(dialog->Show(ownerWindow))) {
			IShellItem* item = nullptr;
			if (SUCCEEDED(dialog->GetResult(&item))) {
				PWSTR path = nullptr;
				if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
					outFolder = path;
					CoTaskMemFree(path);
					picked = true;
				}
				item->Release();
			}
		}
		dialog->Release();
	}

	if (uninit) CoUninitialize();
	return picked;
}