#include "mandbgevent.h"

BOOL InitDebugInfo() {
	WCHAR logpath[MAX_PATH] = { 0 };
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	_snwprintf_s(logpath, MAX_PATH, _TRUNCATE, L".\\log\\DBG_%04d%02d%02d_%02d%02d%02d.txt"
		, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	g_hLogFile = CreateFile(logpath, GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return (g_hLogFile != INVALID_HANDLE_VALUE);
}

VOID WriteDebugInfo(const PWCHAR foramt, ...) {
	if (g_hLogFile == INVALID_HANDLE_VALUE) return;

	WCHAR szbuf[0x1000] = { 0 };
	va_list args;

	va_start(args, foramt);
	int nLength = _vsnwprintf_s(szbuf, _countof(szbuf), _TRUNCATE, foramt, args);
	va_end(args);

	if (nLength > 0) {
		DWORD dwWrite = 0;
		WriteFile(g_hLogFile, szbuf, nLength * sizeof(WCHAR), &dwWrite, NULL);
	}
}

BOOL DbgEventCreateProcess(CREATE_PROCESS_DEBUG_INFO* info) {
	// Basic Information
	{
		if (!info) {
			wprintf(L"Invaild CREATEPROCESS Param \r\n");
			return FALSE;
		}

		printf("[CREATE_PROCESS_DEBUG_INFO] \r\n");
		printf("hProcess -> 0x%p \r\n", info->hProcess);
		printf("hThread -> 0x%p \r\n", info->hThread);
		printf("Image of BASETYPES -> 0x%p \r\n", info->lpBaseOfImage);
		printf("Debug Info File Offset -> 0x%x \r\n", info->dwDebugInfoFileOffset);
		printf("Debug Info Size -> %d \r\n", info->nDebugInfoSize);
		printf("Thread Local Base -> 0x%p \r\n", info->lpThreadLocalBase);

		if (info->lpImageName) {
			if (info->fUnicode) {
				wprintf(L"Image Name -> %ws \r\n", (wchar_t*)info->lpImageName);
			}
			else {
				printf("Image Name -> %s \r\n", (char*)info->lpImageName);
			}
		}
		else {
			WCHAR szFileName[MAX_PATH] = { 0 };
			if (GetModuleFileNameEx(info->hProcess, NULL, szFileName, MAX_PATH)) {
				wprintf(L"Image Name -> %ws \r\n", szFileName);
			}
			else {
				wprintf(L"GetModuleFileNameExW Failed -> %d \r\n", GetLastError());
			}
		}

		CONTEXT context = { 0 };
		context.ContextFlags = CONTEXT_ALL;
		if (GetThreadContext(info->hThread, &context)) {

#ifdef _WIN64
			wprintf(L"RIP -> 0x%p \r\n", (void*)context.Rip);
			wprintf(L"RSP -> 0x%p \r\n", (void*)context.Rsp);
			wprintf(L"RBP -> 0x%p \r\n", (void*)context.Rbp);
#else
			wprintf(L"EIP -> 0x%p \r\n", (void*)context.Eip);
			wprintf(L"ESP -> 0x%p \r\n", (void*)context.Esp);
			wprintf(L"EBP -> 0x%p \r\n", (void*)context.Ebp);
#endif
		}
		else {
			wprintf(L"GetThreadContext Failed -> %d \r\n", GetLastError());
		}
	}
	
	return TRUE;
}

BOOL DbgEventCreateThread(CREATE_THREAD_DEBUG_INFO* info) {
	printf("[CreateThread]\n");
	return TRUE;
}

BOOL DbgEventException(EXCEPTION_DEBUG_INFO* info) {

	return TRUE;
}

VOID GetProcessFullPath(HANDLE hProcess, PWCHAR szBuffer, DWORD dwBufferSize) {
	DWORD dwSize = dwBufferSize;
	if (!QueryFullProcessImageName(hProcess, 0, szBuffer, &dwSize)){
		wcscpy_s(szBuffer,dwBufferSize,L"Unkown Process");
	}
}

VOID GetProcessMemInfo(HANDLE hProcess, PPROCESS_EXIT_INFO pExitInfo) {
	PROCESS_MEMORY_COUNTERS_EX pmc = { 0 };
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);

	if (GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc))) {
		pExitInfo->PeakPagefileUsage = pmc.PeakPagefileUsage;
		pExitInfo->PeakWorkingSetSize = pmc.PeakWorkingSetSize;
		pExitInfo->WorkingSetSize = pmc.WorkingSetSize;
	}
}

VOID CreateProcessDump(HANDLE hProcess, DWORD dwProcessId) {
	WCHAR dumppath[MAX_PATH] = { 0 };
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	_snwprintf_s(dumppath, MAX_PATH, _TRUNCATE, L".\\dump\\DBG_%04d%02d%02d_%02d%02d%02d.txt"
		, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	HANDLE dumpFile = CreateFile(dumppath, GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
}

BOOL DbgEventExitProcess(EXIT_PROCESS_DEBUG_INFO* info) {
	if (!info) {
		printf("Wrong Param\n");
		return FALSE;
	}

	// Init Log File
	printf("Process Exit\n");
	// get exit info


	return FALSE;
}

BOOL DbgEventExitThread(EXIT_THREAD_DEBUG_INFO* info) {
	printf("[ExitThread] exit code->0x%04X\n",info->dwExitCode);
	return TRUE;
}

BOOL DbgEventLoadDll(LOAD_DLL_DEBUG_INFO* info) {
	printf("[LoadDll]\n");
	return TRUE;
}

BOOL DbgEventOutputDebugString(OUTPUT_DEBUG_STRING_INFO* info) {
	return TRUE;
}

BOOL DbgEventRipInfo(RIP_INFO* info) {
	return TRUE;
}

BOOL DbgEventUnloadDll(UNLOAD_DLL_DEBUG_INFO* info) {
	printf("[UnloadDll]");
	return TRUE;
}
