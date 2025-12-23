#include "common.h"

/* 在这里为全局变量做唯一定义和初始化 */
DWORD	g_status    = DEBUG_STATUS_NONE;
HANDLE	g_hProcess  = NULL;
HANDLE	g_hThread   = NULL;
DWORD	g_Process   = 0;
DWORD	g_Thread    = 0;
HANDLE	g_hLogFile  = INVALID_HANDLE_VALUE;

BREAKPOINT_MANAGER g_bpManager = { 0 };