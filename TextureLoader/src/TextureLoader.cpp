#include <iostream>
#include <Windows.h>
#include "DsMap/DsMap.h"
#include "AppProcess/AppProcess.h"
#include <shellapi.h>
#include <filesystem>

#define GM_EXPORT extern "C" __declspec(dllexport)
#define GM_TRUE (1.0)
#define GM_FALSE (0.0)

namespace filesystem = std::filesystem;

static bool tl_initialized = false;
static AppProcess GameProcess(L"SMM_WE.exe");

char* GetEnv(const char* pPath)
{
	char* env;
	size_t len;
	errno_t err = _dupenv_s(&env, &len, pPath);

	return env;
}

GM_EXPORT double dllinit()
{
	if (!tl_initialized)
		tl_initialized = true;
	return GM_TRUE;
}

GM_EXPORT double dllexit()
{
	if (tl_initialized)
		tl_initialized = false;
	return GM_FALSE;
}

GM_EXPORT double RegisterCallbacks(char* arg1, char* arg2, char* arg3, char* arg4)
{
	DsMap::RegCallbacks(arg1, arg2, arg3, arg4);
	return GM_TRUE;
}

GM_EXPORT double tl_init()
{
	if (!tl_initialized) return GM_FALSE;

	GameProcess.AttachProcess();
	if (!GameProcess.isValid())
	{
		return GM_FALSE;
	}

	return GM_TRUE;
}

GM_EXPORT double tl_update()
{
	if (!tl_initialized) return GM_FALSE;
	return GM_TRUE;
}

GM_EXPORT double tl_injectdll()
{
	if (!tl_initialized) return GM_FALSE;
	DsMap Log;

	if (GameProcess.isValid())
	{
		if (GameProcess.isRunning())
		{
			if (!GameProcess.hasModule("libSMMWE.dll"))
			{
				GameProcess.InjectDLL("libSMMWE.dll");
				Log.AddString("event_type", "DllInjected").AddString("details", "Dll injected correctly").Send();
				return GM_TRUE;
			}
			Log.AddString("event_type", "DllExists").AddString("details", "Dll already injected in the current process").Send();
			return GM_FALSE;
		}
		Log.AddString("event_type", "ProcessClosed").AddString("details", "The actual process is closed").Send();
		return GM_FALSE;
	}
	Log.AddString("event_type", "InvalidProcess")
		.AddString("details", "Handle have tried to attach but the process isn't running or something went wrong")
		.AddString("solution", "Re-Attach the process")
		.Send();

	return GM_FALSE;
}

GM_EXPORT double tl_is_process_running()
{
	if (!tl_initialized) return GM_FALSE;
	if (!GameProcess.isValid()) return GM_FALSE;

	return GameProcess.isRunning();

}

GM_EXPORT double tl_has_module()
{
	if (!tl_initialized) return GM_FALSE;

	if (GameProcess.isValid())
	{
		if (GameProcess.isRunning())
		{
			return GameProcess.hasModule("libSMMWE.dll");
		}
	}

	return GM_FALSE;
}

GM_EXPORT double tl_open_folder()
{
	if (!tl_initialized) return GM_FALSE;

	filesystem::path LocalAppData = GetEnv("LOCALAPPDATA");
	filesystem::path oPath = LocalAppData / "SMM_WE\\Textures";

	ShellExecute(NULL, L"open", oPath.wstring().c_str(), NULL, NULL, SW_SHOWDEFAULT);

	return GM_TRUE;
}

GM_EXPORT double tl_open_url()
{
	if (!tl_initialized) return GM_FALSE;

	// Open my youtube channel! :D
	ShellExecute(0, 0, L"https://www.youtube.com/channel/UCNQ-5fc7v3FQNMHfrbKXWlQ", 0, 0, SW_SHOW);

	return GM_TRUE;
}

GM_EXPORT double tl_create_directories()
{
	if (!tl_initialized) return GM_FALSE;
	filesystem::path LocalAppData = GetEnv("LOCALAPPDATA");
	filesystem::path TexturesPath = LocalAppData / "SMM_WE\\Textures\\Default";
	filesystem::path SpritesPath = LocalAppData / "SMM_WE\\Textures\\Default\\Sprites";
	filesystem::path BGPath = LocalAppData / "SMM_WE\\Textures\\Default\\Backgrounds";

	filesystem::create_directories(TexturesPath);
	filesystem::create_directory(SpritesPath);
	filesystem::create_directory(BGPath);

	return GM_TRUE;
}