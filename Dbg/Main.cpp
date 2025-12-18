#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <Psapi.h>
#include "mandbgevent.h"


#define DEBUG_STATUS_SUSPENDED 0x00000001
#define DEBUG_STATUS_ACTIVE    0x00000002
#define DEBUG_STATUS_NONE 0x00000003

std::vector<std::wstring> Cmd;

DWORD g_status = DEBUG_STATUS_NONE;

HANDLE g_hprocess = NULL;
HANDLE g_hthread = NULL;
DWORD g_process = NULL;
DWORD g_thread = NULL;
HANDLE g_hLogFile = INVALID_HANDLE_VALUE;


void softinfo();
BOOL DispatchTask();
BOOL DispatchDebugEvent(DEBUG_EVENT* dbg_event);
BOOL CheckCmdSize(int num);
void CreateProcessDebug();
VOID DBGgo();
// c D:\code\CTF\re\PeCon\Debug\InstDrv.exe

int main() {
	softinfo();

	while (1) {
		//std::wstring userinfo = L"c D:\code\CTF\re\PeCon\Debug\InstDrv.exe";
		std::wstring userinfo;
		std::wcout << L"DBG> ";
		std::getline(std::wcin, userinfo);

		std::wstringstream ss(userinfo);
		std::wstring token;
		while (std::getline(ss, token, L' ')) {
			//std::wcout << token << std::endl;
			Cmd.push_back(token);
		}
		
		if (!DispatchTask()) break;

		Cmd.clear();
	}
	
	return 0;
}

void softinfo() {
	std::wcout << L"AO DBG Con" << std::endl << std::endl;
	std::wcout << L"\t - ? Soft Help" << std::endl;
	std::wcout << L"\t - c Create Process Debug" << std::endl;
	std::wcout << L"\t - q quit" << std::endl;
	std::wcout << L"\t - g run" << std::endl;
	std::wcout << std::endl;

}

BOOL DispatchTask() {
	if (Cmd.size() < 1) return TRUE;
	if (Cmd[0] == L"q") return FALSE;

	static struct {
		std::wstring cmd;
		VOID(*callback)();
	}cmdlist[] = {
		{L"c", CreateProcessDebug},
		{L"?", softinfo},
		{L"g", DBGgo},
	};

	for (size_t i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++) {
		if (Cmd[0] == cmdlist[i].cmd) {
			cmdlist[i].callback();
			return TRUE;
		}
	}

	std::wcout << L"Unknown Command"<< std::endl;

	return TRUE;
}

BOOL CheckCmdSize(int num) {
	if (Cmd.size() != num) {
		std::wcout << L"Usage: c <FullPath> [p]" << std::endl;
		return FALSE;
	}
	return TRUE;
}

VOID CreateProcessDebug() {
	if (!CheckCmdSize(2)){
		return;
	}

	if (g_status != DEBUG_STATUS_NONE) {
		std::wcout << L"Debugging Process is exist, Please Quit it First." << std::endl;
		return;
	}

	STARTUPINFO si = {0};
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi = { 0 };
	BOOL bCreate = CreateProcess(
		Cmd[1].c_str(),
		NULL,
		NULL,
		NULL,
		FALSE,
		DEBUG_ONLY_THIS_PROCESS|CREATE_NEW_CONSOLE|CREATE_SUSPENDED,
		NULL,
		NULL,
		&si,
		&pi
	);
	if (!bCreate) {
		std::wcout << L"CreateProcess failed (" << GetLastError() << L").\n";
		return;
	}
	g_status = DEBUG_STATUS_SUSPENDED;
	

	g_process = pi.dwProcessId;
	g_hprocess = pi.hProcess;
	g_thread = pi.dwThreadId;
	g_hthread = pi.hThread;
	
	std::wcout << L"CreateProcess success.\n";


	/*if (Cmd.size() >= 3 && Cmd[2] == L"p") {
		DebugActiveProcess(pi.dwProcessId);
		std::wcout << L"Attach Process success.\n";
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);*/
}

VOID DBGgo()
{
	DEBUG_EVENT dbgEvent = { 0 };

	if (CheckCmdSize(1) == FALSE) return;

	if (g_status == DEBUG_STATUS_NONE) {
		std::wcout << L"No Debugging Process, Please Create First." << std::endl;
		return;
	}

	if (g_status == DEBUG_STATUS_SUSPENDED) {
		ResumeThread(g_hthread);
		//g_status = DEBUG_STATUS_ACTIVE;
		std::wcout << L"Resume Thread success.\n";
	}

	while (WaitForDebugEvent(&dbgEvent, INFINITE)) {
		//std::wcout << L"Debug Event Code: " << dbgEvent.dwDebugEventCode << std::endl;
		if (DispatchDebugEvent(&dbgEvent)) {
			ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, DBG_CONTINUE);
		}
		else {
			break;
		}
	}
}

BOOL DispatchDebugEvent(DEBUG_EVENT* dbg_event)
{
	switch (dbg_event->dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT:
			return DbgEventCreateProcess(&dbg_event->u.CreateProcessInfo);
		case CREATE_THREAD_DEBUG_EVENT:
			return DbgEventCreateThread(&dbg_event->u.CreateThread);
		case EXCEPTION_DEBUG_EVENT:
			return DbgEventException(&dbg_event->u.Exception);
		case EXIT_PROCESS_DEBUG_EVENT:
			return DbgEventExitProcess(&dbg_event->u.ExitProcess);
		case EXIT_THREAD_DEBUG_EVENT:
			return DbgEventExitThread(&dbg_event->u.ExitThread);
		case LOAD_DLL_DEBUG_EVENT:
			return DbgEventLoadDll(&dbg_event->u.LoadDll);
		case OUTPUT_DEBUG_STRING_EVENT:
			return DbgEventOutputDebugString(&dbg_event->u.DebugString);
		case RIP_EVENT:
			return DbgEventRipInfo(&dbg_event->u.RipInfo);
		case UNLOAD_DLL_DEBUG_EVENT:
			return DbgEventUnloadDll(&dbg_event->u.UnloadDll);
	}
	return 0;
}