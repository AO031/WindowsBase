#pragma once

#include <Windows.h>

#define DEBUG_STATUS_SUSPENDED 0x00000001
#define DEBUG_STATUS_ACTIVE    0x00000002
#define DEBUG_STATUS_NONE      0x00000003

/* 头文件只声明全局变量（不要带初始化） */
extern DWORD	g_status;
extern HANDLE	g_hProcess;
extern HANDLE	g_hThread;
extern DWORD	g_Process;
extern DWORD	g_Thread;
extern HANDLE	g_hLogFile;

/* 类型定义放在头文件里（保持单一来源），确保其它头不要重复定义这些类型 */
typedef struct _PROCESS_EXIT_INFO {
	size_t PeakPagefileUsage;
	size_t PeakWorkingSetSize;
	size_t WorkingSetSize;
} PROCESS_EXIT_INFO, *PPROCESS_EXIT_INFO;

typedef struct _BREAKPOINT {
	LPVOID address;
	BYTE originalByte;
	BOOL isEnabled;
	BOOL isTemped;
} BREAKPOINT, *PBREAKPOINT;

typedef struct _BP_NODE {
	BREAKPOINT bp;
	struct _BP_NODE* next;
} BP_NODE, *PBP_NODE;

typedef struct _BREAKPOINT_MANAGER {
	PBP_NODE head;
	HANDLE hProcess;
	DWORD count;
} BREAKPOINT_MANAGER, *PBREAKPOINT_MANAGER;

/* 仅声明，不定义 */
extern BREAKPOINT_MANAGER g_bpManager;