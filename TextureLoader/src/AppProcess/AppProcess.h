#pragma once
#include <Windows.h>

// Class wrapper for handle the windows processes shit
class AppProcess
{
private:
	const wchar_t* m_ProcessName;												// Our app name
	int m_ProcessID;															// Our app's process ID
	HANDLE m_Process;															// Our app process
private:
	int IGetProcessID();
public:
	AppProcess(const wchar_t* name);
	AppProcess(const wchar_t* name, int id);
	~AppProcess();

	void AttachProcess();
	bool isValid();
	bool isRunning();
	bool hasModule(const char* module_name);
	bool InjectDLL(const char* dllName);
	//bool InjectHijackThreadDLL(const char dllName);
};