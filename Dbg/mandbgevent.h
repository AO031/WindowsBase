#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <dbghelp.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <minwinbase.h>
#include "common.h"


BOOL InitDebugInfo();
VOID WriteDebugInfo(CONST PWCHAR foramt, ...);

VOID GetProcessFullPath(HANDLE hProcess, PWCHAR szBuffer, DWORD dwBufferSize);
VOID GetProcessMemInfo(HANDLE hProcess, PPROCESS_EXIT_INFO pExitInfo);
BOOL InitBreakPointManager(PBREAKPOINT_MANAGER mamger, HANDLE hProcess);
BOOL SetBreakPoint(PBREAKPOINT_MANAGER manager, LPVOID address, BOOL temp);
BOOL InitBreakPointManager(PBREAKPOINT_MANAGER manager, HANDLE hProcess);
BOOL RemoveBreakPoint(PBREAKPOINT_MANAGER manager, LPVOID address);

BOOL DbgEventCreateProcess(CREATE_PROCESS_DEBUG_INFO* info);
BOOL DbgEventCreateThread(CREATE_THREAD_DEBUG_INFO* info);
BOOL DbgEventException(EXCEPTION_DEBUG_INFO* info);
BOOL DbgEventExitProcess(EXIT_PROCESS_DEBUG_INFO* info);
BOOL DbgEventExitThread(EXIT_THREAD_DEBUG_INFO* info);
BOOL DbgEventLoadDll(LOAD_DLL_DEBUG_INFO* info);
BOOL DbgEventOutputDebugString(OUTPUT_DEBUG_STRING_INFO* info);
BOOL DbgEventRipInfo(RIP_INFO* info);
BOOL DbgEventUnloadDll(UNLOAD_DLL_DEBUG_INFO* info);