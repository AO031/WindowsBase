#include "mandbgevent.h"
#include "common.h"


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
/*====================================================================*/

typedef enum _EXCEPT_CMD {
	CONTINUE,
	STEP,
	REGISTER,
	MEMORY,
	HELP
}EXCEPT_CMD;

EXCEPT_CMD GetComand(const std::wstring& cmd) {
	if (cmd == L"c") return CONTINUE;
	if (cmd == L"ni") return STEP;
	if (cmd == L"reg") return REGISTER;
	if (cmd == L"tel") return MEMORY;
	if (cmd == L"help") return HELP;
}

PBP_NODE FindBreakPoint(PBREAKPOINT_MANAGER manager,LPVOID address) {
	if (!manager) {
		return NULL;
	}

	PBP_NODE cur = manager->head;

	while (cur) {
		if (cur->bp.address == address) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;
}

BOOL SetBreakPoint(PBREAKPOINT_MANAGER manager, LPVOID address, BOOL temp) {

	if (FindBreakPoint(manager,address)) {
		printf("already exit\n");
		return FALSE;
	}

	PBP_NODE newBp = (PBP_NODE)malloc(sizeof(BP_NODE));
	if (!newBp) {
		return FALSE;
	}

	BYTE int3 = 0xCC;
	BYTE originalByte = 0;
	SIZE_T byteRead, byteWrite;
	HANDLE hProcess = manager->hProcess;

	if (!ReadProcessMemory(hProcess, address, &originalByte, sizeof(BYTE), &byteRead)) {
		printf("ReadProcessMemory Failed\n");
		return FALSE;
	}

	if (!WriteProcessMemory(hProcess,address,&int3,sizeof(BYTE),&byteWrite)){
		printf("WriteProcessMemory Failed\n");
		return FALSE;
	}

	newBp->bp.address = address;
	newBp->bp.isEnabled = TRUE;
	newBp->bp.isTemped = temp;
	newBp->bp.originalByte = originalByte;

	if (manager->head) {
		newBp->next = manager->head;
		manager->head = newBp;
	}
	else {
		newBp->next = NULL;
		manager->head = newBp;
	}

	manager->count++;

	return TRUE;
}

BOOL InitBreakPointManager(PBREAKPOINT_MANAGER manager, HANDLE hProcess) {

	manager->head = NULL;
	manager->hProcess = hProcess;
	manager->count = 0;

	return TRUE;
}

BOOL RemoveBreakPoint(PBREAKPOINT_MANAGER manager, LPVOID address)
{
	if (!manager||!manager->head) {
		printf("There is no breakpoint\n");
		return FALSE;
	}

	PBP_NODE cur = manager->head;
	PBP_NODE prev = NULL;
	
	while (cur) {

		if (cur->bp.address == address) {
			if (!WriteProcessMemory(
				manager->hProcess,
				address,
				&cur->bp.originalByte,
				sizeof(BYTE),
				NULL
			)) {
				printf("WriteProcessMemory Failed\n");
				return FALSE;
			}

			if (prev) {
				prev->next = cur->next;
			}
			else {
				manager->head = cur->next;
			}
			manager->count--;
			free(cur);
			return TRUE;
		}
		prev = cur;
		cur = cur->next;
	}

	return FALSE;
}

VOID ShowMemory(HANDLE hProcess, LPVOID address) {
	BYTE code[80] = { 0 };

	if (ReadProcessMemory(hProcess, address, code, 80, NULL)) {
		printf("address -> %p\n", address);
		for (int i = 0; i < 80; i++) {
			if (i == 0) {
				printf("0x%02X ", code[i]);
				continue;
			}
			if (i % 8 == 0) printf("\n");
			printf("0x%02X ", code[i]);
		}
		printf("\n");
	}
}

VOID ShowStepInfo(HANDLE hProcess, LPVOID address, PBP_NODE pBpNode) {
	BYTE code[16] = {0};
	
	if (ReadProcessMemory(hProcess, address, code, 16, NULL)) {
		printf("EIP -> %p\n", address);
		for (int i = 0; i < 16; i++) {
			if (i == 0 && pBpNode) {
				printf("0x%02X ", pBpNode->bp.originalByte);
				continue;
			}
			if (i == 8) printf("\n");
			printf("0x%02X ", code[i]);
		}
		printf("\n");
	}
}

BOOL SetSingleStep(HANDLE hThread) {
	CONTEXT ctx = { 0 };
	ctx.ContextFlags = CONTEXT_ALL;

	if (!GetThreadContext(hThread, &ctx)) {
		printf("GetThreadContext Failed\n");
		return FALSE;
	}

	ctx.EFlags |= 0x100;

	if (!SetThreadContext(hThread, &ctx)) {
		printf("SetThreadContext Failed\n");
		return FALSE;
	}

	return TRUE;
}

VOID HandlerExceptionLoop() {
	while (g_status == DEBUG_STATUS_SUSPENDED) {
		std::wstring cmd;
		std::wcout << "\nDbg> ";
		std::getline(std::wcin, cmd);
		std::wstring addrstr;
		LPVOID address = 0;

		switch (GetComand(cmd)) {
		case CONTINUE:
			g_status = DEBUG_STATUS_ACTIVE;
			std::wcout << "Continue to run" << std::endl;
			break;
		case STEP:
			if (SetSingleStep(g_hThread)) {
				g_status = DEBUG_STATUS_ACTIVE;
				std::wcout << "Continue to Single run" << std::endl;
			}
			break;
		case REGISTER:
			break;
		case MEMORY:
			std::wcout << "address: ";
			std::getline(std::wcin, addrstr);
			address = (LPVOID)std::stoull(addrstr, nullptr, 16);
			ShowMemory(g_hProcess, address);
			break;
		case HELP:
			break;
		}

		if (g_status != DEBUG_STATUS_SUSPENDED) {
			break;
		}
	}
}

BOOL SingleStepHandler(PEXCEPTION_RECORD pExceptRecord) {

	ShowStepInfo(g_bpManager.hProcess, pExceptRecord->ExceptionAddress, NULL);

	g_status = DEBUG_STATUS_SUSPENDED;

	HandlerExceptionLoop();

	return TRUE;
}

BOOL BreakPointHandler(PEXCEPTION_RECORD pExceptionRecord) {
	PBP_NODE pBpNode = FindBreakPoint(&g_bpManager, pExceptionRecord->ExceptionAddress);
	if (!pBpNode) {
		printf("It's not our break point\n");
		return TRUE;
	}

	ShowStepInfo(g_bpManager.hProcess, pExceptionRecord->ExceptionAddress, pBpNode);

	if (pBpNode->bp.isEnabled) {
		RemoveBreakPoint(&g_bpManager, pExceptionRecord->ExceptionAddress);
	}

	g_status = DEBUG_STATUS_SUSPENDED;

	HandlerExceptionLoop();


	return TRUE;
}

BOOL DbgEventException(EXCEPTION_DEBUG_INFO* info) {
	PEXCEPTION_RECORD pExceptRecord = &info->ExceptionRecord;

	if (pExceptRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
		return SingleStepHandler(pExceptRecord);
	}

	if (pExceptRecord->ExceptionCode == EXCEPTION_BREAKPOINT) {
		return BreakPointHandler(pExceptRecord);
	}

	printf("Exception Code -> %p\n", (PVOID)pExceptRecord->ExceptionCode);

	HandlerExceptionLoop();
	return TRUE;
}

/*====================================================================*/
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
