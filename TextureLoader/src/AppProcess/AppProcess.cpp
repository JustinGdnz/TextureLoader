#include "AppProcess.h"
#include <TlHelp32.h>
#include <Psapi.h>
#include <filesystem>

int AppProcess::IGetProcessID()
{
	DWORD procID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);				// Create a snapshot with the current processes running

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		// Loop through all the processes until it finds the one that we require
		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (wcscmp(procEntry.szExeFile, m_ProcessName) == 0)
				{
					procID = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procID;
}

AppProcess::AppProcess(const wchar_t* name)
{
	m_ProcessName = name;
	m_ProcessID = 0;
	m_Process = INVALID_HANDLE_VALUE;
}

AppProcess::AppProcess(const wchar_t* name, int id)
{
	m_ProcessName = name;
	m_ProcessID = id;
	m_Process = INVALID_HANDLE_VALUE;
}

AppProcess::~AppProcess()
{
	CloseHandle(m_Process);														// Closes the handle of our app's process
}

void AppProcess::AttachProcess()
{
	m_ProcessID = IGetProcessID();												// Get the our app's procces ID
	m_Process = OpenProcess(PROCESS_ALL_ACCESS, 0, m_ProcessID);				// Create a handle for our app's process
}

bool AppProcess::isValid()
{
	if (m_Process && m_Process != INVALID_HANDLE_VALUE) return true;
	return false;
}

bool AppProcess::isRunning()
{
	DWORD exitCodeOut;

	// GetExitCodeProcess returns zero on failure
	if (GetExitCodeProcess(m_Process, &exitCodeOut) != 0)
	{
		// Return if the process is still active
		return exitCodeOut == STILL_ACTIVE;
	}
}

bool AppProcess::hasModule(const char* module_name)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;

	// Get a list of all the modules in this process
	if (EnumProcessModules(m_Process, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			wchar_t szModName[MAX_PATH];

			// Get the full path to the module's file.
			if (GetModuleFileNameEx(m_Process, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				std::filesystem::path modulePath = szModName;
				if (_stricmp(modulePath.filename().string().c_str(), module_name) == 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool AppProcess::InjectDLL(const char* dllName)
{

	// Some shit i copypasted from internet
	char dllPath[MAX_PATH] = { 0 };
	GetFullPathNameA(dllName, MAX_PATH, dllPath, NULL);

	if (isValid())
	{
		LPVOID loc = VirtualAllocEx(m_Process, 0, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		WriteProcessMemory(m_Process, loc, dllPath, strlen(dllPath) + 1, 0);
		HANDLE hTread = CreateRemoteThread(m_Process, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

		if (hTread)
		{
			CloseHandle(hTread);
		}
		return true;
	}

	return false;
}