/* Copyright (c) 2017 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBKERNEL32_THREAD_H
#define GUARD_LIBS_LIBKERNEL32_THREAD_H 1

#include "k32.h"
#include <hybrid/compiler.h>

DECL_BEGIN

/* Thread/Process APIs. */
INTDEF WINBOOL WINAPI K32_GetProcessAffinityMask(HANDLE hProcess, PDWORD_PTR lpProcessAffinityMask, PDWORD_PTR lpSystemAffinityMask);
INTDEF WINBOOL WINAPI K32_SetProcessAffinityMask(HANDLE hProcess, DWORD_PTR dwProcessAffinityMask);
INTDEF WINBOOL WINAPI K32_GetProcessHandleCount(HANDLE hProcess, PDWORD pdwHandleCount);
INTDEF WINBOOL WINAPI K32_GetProcessTimes(HANDLE hProcess, LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime);
INTDEF WINBOOL WINAPI K32_GetProcessIoCounters(HANDLE hProcess, PIO_COUNTERS lpIoCounters);
INTDEF WINBOOL WINAPI K32_GetProcessWorkingSetSize(HANDLE hProcess, PSIZE_T lpMinimumWorkingSetSize, PSIZE_T lpMaximumWorkingSetSize);
INTDEF WINBOOL WINAPI K32_GetProcessWorkingSetSizeEx(HANDLE hProcess, PSIZE_T lpMinimumWorkingSetSize, PSIZE_T lpMaximumWorkingSetSize, PDWORD Flags);
INTDEF WINBOOL WINAPI K32_SetProcessWorkingSetSize(HANDLE hProcess, SIZE_T dwMinimumWorkingSetSize, SIZE_T dwMaximumWorkingSetSize);
INTDEF WINBOOL WINAPI K32_SetProcessWorkingSetSizeEx(HANDLE hProcess, SIZE_T dwMinimumWorkingSetSize, SIZE_T dwMaximumWorkingSetSize, DWORD Flags);
INTDEF HANDLE  WINAPI K32_OpenProcess(DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwProcessId);
INTDEF HANDLE  WINAPI K32_GetCurrentProcess(void);
INTDEF DWORD   WINAPI K32_GetCurrentProcessId(void);
INTDEF ATTR_NORETURN void WINAPI K32_ExitProcess(UINT uExitCode);
INTDEF WINBOOL WINAPI K32_TerminateProcess(HANDLE hProcess, UINT uExitCode);
INTDEF WINBOOL WINAPI K32_GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);
INTDEF void    WINAPI K32_FatalExit(int ExitCode);
INTDEF LPVOID  WINAPI K32_CreateFiber(SIZE_T dwStackSize, LPFIBER_START_ROUTINE lpStartAddress, LPVOID lpParameter);
INTDEF LPVOID  WINAPI K32_CreateFiberEx(SIZE_T dwStackCommitSize, SIZE_T dwStackReserveSize, DWORD dwFlags, LPFIBER_START_ROUTINE lpStartAddress, LPVOID lpParameter);
INTDEF void    WINAPI K32_DeleteFiber(LPVOID lpFiber);
INTDEF LPVOID  WINAPI K32_ConvertThreadToFiber(LPVOID lpParameter);
INTDEF LPVOID  WINAPI K32_ConvertThreadToFiberEx(LPVOID lpParameter, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_ConvertFiberToThread(void);
INTDEF void    WINAPI K32_SwitchToFiber(LPVOID lpFiber);
INTDEF WINBOOL WINAPI K32_SwitchToThread(void);
INTDEF HANDLE  WINAPI K32_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
INTDEF HANDLE  WINAPI K32_CreateRemoteThread(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
INTDEF HANDLE  WINAPI K32_GetCurrentThread(void);
INTDEF DWORD   WINAPI K32_GetCurrentThreadId(void);
INTDEF WINBOOL WINAPI K32_SetThreadStackGuarantee (PULONG StackSizeInBytes);
INTDEF DWORD   WINAPI K32_GetProcessIdOfThread(HANDLE Thread);
INTDEF DWORD   WINAPI K32_GetThreadId(HANDLE Thread);
INTDEF DWORD   WINAPI K32_GetProcessId(HANDLE Process);
INTDEF DWORD   WINAPI K32_GetCurrentProcessorNumber(void);
INTDEF DWORD_PTR WINAPI K32_SetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask);
INTDEF DWORD   WINAPI K32_SetThreadIdealProcessor(HANDLE hThread, DWORD dwIdealProcessor);
INTDEF WINBOOL WINAPI K32_SetProcessPriorityBoost(HANDLE hProcess, WINBOOL bDisablePriorityBoost);
INTDEF WINBOOL WINAPI K32_GetProcessPriorityBoost(HANDLE hProcess, PBOOL pDisablePriorityBoost);
INTDEF WINBOOL WINAPI K32_RequestWakeupLatency(LATENCY_TIME latency);
INTDEF WINBOOL WINAPI K32_IsSystemResumeAutomatic(void);
INTDEF HANDLE  WINAPI K32_OpenThread(DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwThreadId);
INTDEF int     WINAPI K32_GetThreadPriority(HANDLE hThread);
INTDEF WINBOOL WINAPI K32_SetThreadPriority(HANDLE hThread, int nPriority);
INTDEF DWORD   WINAPI K32_GetPriorityClass(HANDLE hProcess);
INTDEF WINBOOL WINAPI K32_SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass);
INTDEF WINBOOL WINAPI K32_SetThreadPriorityBoost(HANDLE hThread, WINBOOL bDisablePriorityBoost);
INTDEF WINBOOL WINAPI K32_GetThreadPriorityBoost(HANDLE hThread, PBOOL pDisablePriorityBoost);
INTDEF WINBOOL WINAPI K32_GetThreadTimes(HANDLE hThread, LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime);
INTDEF WINBOOL WINAPI K32_GetThreadIOPendingFlag(HANDLE hThread, PBOOL lpIOIsPending);
INTDEF ATTR_NORETURN void WINAPI K32_ExitThread(DWORD dwExitCode);
INTDEF WINBOOL WINAPI K32_TerminateThread(HANDLE hThread, DWORD dwExitCode);
INTDEF WINBOOL WINAPI K32_GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode);
INTDEF WINBOOL WINAPI K32_GetThreadSelectorEntry(HANDLE hThread, DWORD dwSelector, LPLDT_ENTRY lpSelectorEntry);
INTDEF EXECUTION_STATE WINAPI K32_SetThreadExecutionState(EXECUTION_STATE esFlags);
INTDEF WINBOOL WINAPI K32_IsWow64Process(HANDLE hProcess, PBOOL Wow64Process);
INTDEF WINBOOL WINAPI K32_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
INTDEF WINBOOL WINAPI K32_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
INTDEF WINBOOL WINAPI K32_CreateProcessAsUserA(HANDLE hToken, LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
INTDEF WINBOOL WINAPI K32_CreateProcessAsUserW(HANDLE hToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

/* Critical section API. */
INTDEF void    WINAPI K32_InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
INTDEF void    WINAPI K32_EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
INTDEF void    WINAPI K32_LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
INTDEF WINBOOL WINAPI K32_InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);
INTDEF DWORD   WINAPI K32_SetCriticalSectionSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount);
INTDEF WINBOOL WINAPI K32_TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
INTDEF void    WINAPI K32_DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

/* Error code APIs. */
INTDEF DWORD WINAPI K32_GetLastError(void);
INTDEF void WINAPI K32_SetLastError(DWORD dwErrCode);

/* Environment strings/appenv API. */
INTDEF LPCH    WINAPI K32_GetEnvironmentStringsA(void);
INTDEF LPWCH   WINAPI K32_GetEnvironmentStringsW(void);
INTDEF WINBOOL WINAPI K32_SetEnvironmentStringsA(LPCH NewEnvironment);
INTDEF WINBOOL WINAPI K32_SetEnvironmentStringsW(LPWCH NewEnvironment);
INTDEF WINBOOL WINAPI K32_FreeEnvironmentStringsA(LPCH lpEnv);
INTDEF WINBOOL WINAPI K32_FreeEnvironmentStringsW(LPWCH lpEnv);
INTDEF void    WINAPI K32_GetStartupInfoA(LPSTARTUPINFOA lpStartupInfo);
INTDEF void    WINAPI K32_GetStartupInfoW(LPSTARTUPINFOW lpStartupInfo);

/* System/Processor APIs. */
INTDEF WINBOOL WINAPI K32_IsProcessorFeaturePresent(DWORD ProcessorFeature);
INTDEF void WINAPI K32_GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
INTDEF void WINAPI K32_GetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo);
INTDEF WINBOOL WINAPI K32_SetSystemFileCacheSize(SIZE_T MinimumFileCacheSize, SIZE_T MaximumFileCacheSize, DWORD Flags);
INTDEF WINBOOL WINAPI K32_GetSystemFileCacheSize(PSIZE_T lpMinimumFileCacheSize, PSIZE_T lpMaximumFileCacheSize, PDWORD lpFlags);
INTDEF WINBOOL WINAPI K32_GetSystemRegistryQuota(PDWORD pdwQuotaAllowed, PDWORD pdwQuotaUsed);
INTDEF WINBOOL WINAPI K32_GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize);
INTDEF WINBOOL WINAPI K32_GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize);
INTDEF WINBOOL WINAPI K32_SetComputerNameA(LPCSTR lpComputerName);
INTDEF WINBOOL WINAPI K32_SetComputerNameW(LPCWSTR lpComputerName);

/* Pointer encode/decode. */
INTDEF PVOID   WINAPI K32_EncodePointer(PVOID p);
INTDEF PVOID   WINAPI K32_DecodePointer(PVOID p);
INTDEF PVOID   WINAPI K32_EncodeSystemPointer(PVOID p);
INTDEF PVOID   WINAPI K32_DecodeSystemPointer(PVOID p);

/* Debug APIs. */
INTDEF void    WINAPI K32_DebugBreak(void);
INTDEF WINBOOL WINAPI K32_IsDebuggerPresent(void);
INTDEF WINBOOL WINAPI K32_DebugBreakProcess(HANDLE Process);
INTDEF void    WINAPI K32_OutputDebugStringA(LPCSTR lpOutputString);
INTDEF void    WINAPI K32_OutputDebugStringW(LPCWSTR lpOutputString);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_THREAD_H */
