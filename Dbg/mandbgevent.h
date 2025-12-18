#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <dbghelp.h>
#include <stdio.h>
#include <minwinbase.h>

extern HANDLE g_hLogFile;


typedef struct _PROCESS_EXIT_INFO{
	size_t PeakPagefileUsage;
	size_t PeakWorkingSetSize;
	size_t WorkingSetSize;
}PROCESS_EXIT_INFO, *PPROCESS_EXIT_INFO;

typedef struct _BREAKPOINT {
	LPVOID address;
	BYTE originalByte;
	BOOL isEnabled;
	BOOL isTemped;
}BREAKPOINT, * PBREAKPOINT;

typedef struct _BP_NODE {
	BREAKPOINT bp;
	struct _BP_NODE* next;
}BP_NODE, * PBP_NODE;

typedef struct _BREAKPOINT_MANAGER {
	PBP_NODE head;
	HANDLE hProcess;
	DWORD count;
}BREAKPOINT_MANAGER, * PBREAKPOINT_MANAGER;


BREAKPOINT_MANAGER g_bpManager = { 0 };

BOOL InitDebugInfo();
VOID WriteDebugInfo(CONST PWCHAR foramt, ...);

VOID GetProcessFullPath(HANDLE hProcess, PWCHAR szBuffer, DWORD dwBufferSize);
VOID GetProcessMemInfo(HANDLE hProcess, PPROCESS_EXIT_INFO pExitInfo);


BOOL DbgEventCreateProcess(CREATE_PROCESS_DEBUG_INFO* info);
BOOL DbgEventCreateThread(CREATE_THREAD_DEBUG_INFO* info);
BOOL DbgEventException(EXCEPTION_DEBUG_INFO* info);
BOOL DbgEventExitProcess(EXIT_PROCESS_DEBUG_INFO* info);
BOOL DbgEventExitThread(EXIT_THREAD_DEBUG_INFO* info);
BOOL DbgEventLoadDll(LOAD_DLL_DEBUG_INFO* info);
BOOL DbgEventOutputDebugString(OUTPUT_DEBUG_STRING_INFO* info);
BOOL DbgEventRipInfo(RIP_INFO* info);
BOOL DbgEventUnloadDll(UNLOAD_DLL_DEBUG_INFO* info);