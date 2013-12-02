#pragma once

#include <Windows.h>

typedef unsigned long long QWORD;

DWORD dwFindProcessId(LPTSTR lpProcessName);
HANDLE hProcOpenProcess(DWORD dwProcessId);
LPCVOID lpLocateModuleBase(DWORD dwProcessId, LPTSTR lpModuleName);

BOOL bReadObject(HANDLE hProc, LPCVOID lpAddress, SIZE_T nBytes, LPVOID lpOut);
BOOL bReadQword(HANDLE hProc, LPCVOID lpAddress, QWORD *qwOut);