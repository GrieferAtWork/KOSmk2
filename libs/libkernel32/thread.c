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
#ifndef GUARD_LIBS_LIBKERNEL32_THREAD_C
#define GUARD_LIBS_LIBKERNEL32_THREAD_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "k32.h"
#include "thread.h"
#include "file.h"

#include <assert.h>
#include <signal.h>
#include <hybrid/compiler.h>
#include <hybrid/arch/cpu.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <format-printer.h>
#include <process.h>
#include <unicode.h>
#include <malloc.h>
#include <sys/syscall.h>
#include <kos/environ.h>
#include <sys/wait.h>
#include <sched.h>
#include <termio.h>
#include <termios.h>
#include <pty.h>

DECL_BEGIN

__LIBC int LIBCCALL dos_chdir(char const *dos_path) ASMNAME("_chdir");
__LIBC int LIBCCALL dos_execv(char const *path, char *argv[]) ASMNAME("_execv");
__LIBC int LIBCCALL dos_execve(char const *path, char *argv[], char *envp[]) ASMNAME("_execve");
__LIBC int LIBCCALL dos_execvp(char const *file, char *argv[]) ASMNAME("_execvp");
__LIBC int LIBCCALL dos_execvpe(char const *file, char *argv[], char *envp[]) ASMNAME("_execvpe");


/* Thread/Process ID APIs. */
INTERN WINBOOL WINAPI
K32_GetProcessAffinityMask(HANDLE hProcess,
                           PDWORD_PTR lpProcessAffinityMask,
                           PDWORD_PTR lpSystemAffinityMask) {
 WINBOOL result;
 if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return FALSE; }
 result = !sched_getaffinity(HANDLE_TO_PID(hProcess),sizeof(DWORD),
                            (cpu_set_t *)lpProcessAffinityMask);
 if (result) *lpSystemAffinityMask = *lpProcessAffinityMask;
 return result;
}
INTERN WINBOOL WINAPI
K32_SetProcessAffinityMask(HANDLE hProcess,
                           DWORD_PTR dwProcessAffinityMask) {
 if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return FALSE; }
 return !sched_setaffinity(HANDLE_TO_PID(hProcess),sizeof(DWORD),
                          (cpu_set_t *)&dwProcessAffinityMask);
}
DEFINE_INTERN_ALIAS(K32_SetThreadAffinityMask,K32_SetProcessAffinityMask);
INTERN WINBOOL WINAPI K32_GetProcessHandleCount(HANDLE hProcess, PDWORD pdwHandleCount) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetProcessTimes(HANDLE hProcess, LPFILETIME lpCreationTime, LPFILETIME lpExitTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime) { NOT_IMPLEMENTED(); return FALSE; }
DEFINE_INTERN_ALIAS(K32_GetThreadTimes,K32_GetProcessTimes);
INTERN WINBOOL WINAPI K32_GetProcessIoCounters(HANDLE hProcess, PIO_COUNTERS lpIoCounters) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetProcessWorkingSetSize(HANDLE hProcess, PSIZE_T lpMinimumWorkingSetSize, PSIZE_T lpMaximumWorkingSetSize) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetProcessWorkingSetSizeEx(HANDLE hProcess, PSIZE_T lpMinimumWorkingSetSize, PSIZE_T lpMaximumWorkingSetSize, PDWORD Flags) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetProcessWorkingSetSize(HANDLE hProcess, SIZE_T dwMinimumWorkingSetSize, SIZE_T dwMaximumWorkingSetSize) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetProcessWorkingSetSizeEx(HANDLE hProcess, SIZE_T dwMinimumWorkingSetSize, SIZE_T dwMaximumWorkingSetSize, DWORD Flags) { NOT_IMPLEMENTED(); return FALSE; }
INTERN HANDLE WINAPI K32_OpenProcess(DWORD UNUSED(dwDesiredAccess), WINBOOL UNUSED(bInheritHandle), DWORD dwProcessId) { return PID_TO_HANDLE(dwProcessId); }
DEFINE_INTERN_ALIAS(K32_OpenThread,K32_OpenProcess);
INTERN HANDLE WINAPI K32_GetCurrentProcess(void) { return PID_TO_HANDLE(K32_GetCurrentProcessId()); }
INTERN DWORD WINAPI K32_GetCurrentProcessId(void) { return (DWORD)getpid(); }
INTERN ATTR_NORETURN void WINAPI K32_ExitProcess(UINT uExitCode) { _exit((int)uExitCode); }

DEFINE_INTERN_ALIAS(K32_TerminateThread,K32_TerminateProcess);
INTERN WINBOOL WINAPI K32_TerminateProcess(HANDLE hProcess, UINT UNUSED(uExitCode)) {
 if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return FALSE; }
 return !kill(HANDLE_TO_PID(hProcess),SIGKILL);
}
DEFINE_INTERN_ALIAS(K32_GetExitCodeThread,K32_GetExitCodeProcess);
INTERN WINBOOL WINAPI K32_GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode) {
 int info;
 if (!lpExitCode) { SET_ERRNO(EINVAL); return FALSE; }
 if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return FALSE; }
 /* Poll the exit status of the given process, but don't reap it. */
 if (waitpid(HANDLE_TO_PID(hProcess),&info,WNOHANG|WEXITED|WNOWAIT)) {
  if (__get_errno() != EAGAIN) return FALSE;
  /* The process isn't dead yet. */
  *lpExitCode = STILL_ACTIVE;
 } else if (WIFEXITED(info)) {
  *lpExitCode = WEXITSTATUS(info);
 } else if (WCOREDUMP(info)) {
  *lpExitCode = 1;
 } else {
  *lpExitCode = 0;
 }
 return TRUE;
}
DEFINE_INTERN_ALIAS(K32_FatalExit,K32_ExitProcess); /* No special debugging for now... */

INTERN LPVOID WINAPI K32_CreateFiber(SIZE_T dwStackSize, LPFIBER_START_ROUTINE lpStartAddress, LPVOID lpParameter) { NOT_IMPLEMENTED(); return NULL; }
INTERN LPVOID WINAPI K32_CreateFiberEx(SIZE_T dwStackCommitSize, SIZE_T dwStackReserveSize, DWORD dwFlags, LPFIBER_START_ROUTINE lpStartAddress, LPVOID lpParameter) { NOT_IMPLEMENTED(); return NULL; }
INTERN void WINAPI K32_DeleteFiber(LPVOID lpFiber) { NOT_IMPLEMENTED(); }
INTERN LPVOID WINAPI K32_ConvertThreadToFiber(LPVOID lpParameter) { NOT_IMPLEMENTED(); return NULL; }
INTERN LPVOID WINAPI K32_ConvertThreadToFiberEx(LPVOID lpParameter, DWORD dwFlags) { NOT_IMPLEMENTED(); return NULL; }
INTERN WINBOOL WINAPI K32_ConvertFiberToThread(void) { NOT_IMPLEMENTED(); return FALSE; }
INTERN void WINAPI K32_SwitchToFiber(LPVOID lpFiber) { NOT_IMPLEMENTED(); }
INTERN WINBOOL WINAPI K32_SwitchToThread(void) { return sched_yield(); }
INTERN HANDLE WINAPI
K32_CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,
                 SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
                 LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
 uintptr_t result;
 result = _beginthreadex(lpThreadAttributes,dwStackSize,
                        (unsigned int (ATTR_STDCALL *)(void *))lpStartAddress,
                         lpParameter,dwCreationFlags,(unsigned int *)lpThreadId);
 if (!result) return NULL;
 return PID_TO_HANDLE(result);
}
INTERN HANDLE WINAPI
K32_CreateRemoteThread(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes,
                       SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
                       LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
 if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return NULL; }
 if (HANDLE_TO_PID(hProcess) != (pid_t)K32_GetCurrentProcessId()) { SET_ERRNO(EACCES); return NULL; }
 return K32_CreateThread(lpThreadAttributes,dwStackSize,lpStartAddress,
                         lpParameter,dwCreationFlags,lpThreadId);
}
INTERN HANDLE WINAPI K32_GetCurrentThread(void) { return PID_TO_HANDLE(K32_GetCurrentThreadId()); }
INTERN DWORD WINAPI K32_GetCurrentThreadId(void) { return (DWORD)syscall(SYS_gettid); }
INTERN WINBOOL WINAPI K32_SetThreadStackGuarantee(PULONG StackSizeInBytes) { NOT_IMPLEMENTED(); return FALSE; }
INTERN DWORD WINAPI K32_GetProcessIdOfThread(HANDLE Thread) {
 if (!HANDLE_IS_PID(Thread)) { SET_ERRNO(EBADF); return 0; }
 return getpgid(HANDLE_TO_PID(Thread));
}
INTERN DWORD WINAPI K32_GetThreadId(HANDLE Thread) {
 if (!HANDLE_IS_PID(Thread)) { SET_ERRNO(EBADF); return 0; }
 return HANDLE_TO_PID(Thread);
}
DEFINE_INTERN_ALIAS(K32_GetProcessId,K32_GetThreadId);
INTERN DWORD WINAPI K32_GetCurrentProcessorNumber(void) { return (DWORD)sched_getcpu(); }
INTERN DWORD WINAPI K32_SetThreadIdealProcessor(HANDLE UNUSED(hThread), DWORD dwIdealProcessor) { return dwIdealProcessor; }
INTERN WINBOOL WINAPI K32_SetProcessPriorityBoost(HANDLE hProcess, WINBOOL bDisablePriorityBoost) { if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return FALSE; } return TRUE; }
INTERN WINBOOL WINAPI K32_GetProcessPriorityBoost(HANDLE hProcess, PBOOL pDisablePriorityBoost) { if (!HANDLE_IS_PID(hProcess)) { SET_ERRNO(EBADF); return FALSE; } if (pDisablePriorityBoost) *pDisablePriorityBoost = TRUE; return TRUE; }
DEFINE_INTERN_ALIAS(K32_SetThreadPriorityBoost,K32_SetProcessPriorityBoost);
DEFINE_INTERN_ALIAS(K32_GetThreadPriorityBoost,K32_GetProcessPriorityBoost);
INTERN WINBOOL WINAPI K32_RequestWakeupLatency(LATENCY_TIME UNUSED(latency)) { SET_ERRNO(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_IsSystemResumeAutomatic(void) { return FALSE; }
DEFINE_INTERN_ALIAS(K32_GetPriorityClass,K32_GetThreadPriority);
DEFINE_INTERN_ALIAS(K32_SetPriorityClass,K32_SetThreadPriority);
INTERN int WINAPI K32_GetThreadPriority(HANDLE hThread) {
 struct sched_param param;
 if (!HANDLE_IS_PID(hThread)) { SET_ERRNO(EBADF); return THREAD_PRIORITY_ERROR_RETURN; }
 if (sched_getparam(HANDLE_TO_PID(hThread),&param)) return THREAD_PRIORITY_ERROR_RETURN;
 return param.__sched_priority;
}
INTERN WINBOOL WINAPI K32_SetThreadPriority(HANDLE hThread, int nPriority) {
 struct sched_param param = { nPriority };
 if (!HANDLE_IS_PID(hThread)) { SET_ERRNO(EBADF); return FALSE; }
 return !sched_setparam(HANDLE_TO_PID(hThread),&param);
}
INTERN WINBOOL WINAPI K32_GetThreadIOPendingFlag(HANDLE hThread, PBOOL lpIOIsPending) {
 if (!HANDLE_IS_PID(hThread)) { SET_ERRNO(EBADF); return FALSE; }
 if (lpIOIsPending) *lpIOIsPending = FALSE;
 return TRUE;
}
INTERN ATTR_NORETURN void WINAPI
K32_ExitThread(DWORD dwExitCode) {
 syscall(SYS_exit,(int)dwExitCode);
 __builtin_unreachable();
}
INTERN WINBOOL WINAPI
K32_GetThreadSelectorEntry(HANDLE hThread, DWORD dwSelector,
                           LPLDT_ENTRY lpSelectorEntry) {
 if (!HANDLE_IS_PID(hThread)) { SET_ERRNO(EBADF); return FALSE; }
 NOT_IMPLEMENTED();
 return FALSE;
}
INTERN EXECUTION_STATE WINAPI
K32_SetThreadExecutionState(EXECUTION_STATE esFlags) {
 NOT_IMPLEMENTED();
 return esFlags;
}
INTERN WINBOOL WINAPI
K32_IsWow64Process(HANDLE hProcess, PBOOL Wow64Process) {
 if (Wow64Process) *Wow64Process = FALSE;
 return TRUE;
}

PRIVATE size_t KCALL
K32_FixArgument(char *__restrict opt, size_t optlen) {
 char *iter,*end;
 int in_quote = 0;
 /* Remove backslashes. */
 end = (iter = opt)+optlen;
 for (; iter != end;) {
  bool was_quote = false;
  if (*iter == '\\') {
   /* NOTE: If the next character is also a '\\', checking it is skipped. */
del_character:
   --end;
   --optlen;
   memmove(iter,iter+1,(size_t)(end-iter)*sizeof(char));
   if (iter == end) break;
  } else if (in_quote <= 1 && *iter == '"') {
   in_quote = !in_quote;
   was_quote = true;
   goto del_character;
  } else if (in_quote != 1 && *iter == '\'') {
   in_quote = in_quote == 2 ? 0 : 2;
   was_quote = true;
   goto del_character;
  }
  if (!was_quote) ++iter;
 }
 /* Zero-terminate the option. */
 opt[optlen] = '\0';
 return optlen;
}

PRIVATE char **WINAPI K32_SplitCommandLine(LPSTR lpCommandLine) {
 int in_quote = 0;
 char *iter = lpCommandLine;
 char *arg_start = lpCommandLine;
 char **new_argv,**argv = NULL;
 size_t argc = 0,arga = 0;
 if (!*iter) goto end;
 for (;;) {
  if (!*iter || (!in_quote && *iter == ' ' && iter[-1] != '\\')) {
   if (arg_start != iter) {
    size_t optlen = (size_t)(iter-arg_start);
    optlen = K32_FixArgument(arg_start,optlen);
    if (optlen) {
     if (argc == arga) {
      arga = arga ? arga*2 : 2;
      new_argv = (char **)realloc(argv,arga*sizeof(char *));
      if unlikely(!new_argv) goto fail;
      argv = new_argv;
     }
     //syslog(LOG_DEBUG,"K32: CreateProcess(): Option: %.?q\n",optlen,arg_start);
     argv[argc++] = arg_start;
    }
   }
   if (!*iter) break;
   arg_start = iter+1;
  } else if (in_quote <= 1 && *iter == '"') {
   in_quote = !in_quote;
  } else if (in_quote != 1 && *iter == '\'') {
   in_quote = in_quote == 2 ? 0 : 2;
  }
  ++iter;
 }
 assert((argc != 0) == (argv != NULL));
 if (argc == arga) {
  new_argv = (char **)realloc(argv,(argc+1)*sizeof(char *));
  if (!new_argv) goto fail;
 } else if (argc+1 != arga) {
  new_argv = (char **)realloc(argv,(argc+1)*sizeof(char *));
  if (new_argv) argv = new_argv;
 }
 argv[argc] = NULL; /* NULL-terminate the argument list. */
end:
 SET_ERRNO(EOK);
 return argv;
fail:
 free(argv);
 return NULL;
}

PRIVATE WINBOOL WINAPI
K32_DO_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine,
                      WINBOOL bInheritHandles, DWORD dwCreationFlags,
                      LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,
                      LPSTARTUPINFOA lpStartupInfo,
                      LPPROCESS_INFORMATION lpProcessInformation) {
 WINBOOL result = FALSE; pid_t child_pid;
 char *fixed_argv[2]; int confirm[2];
 char *appname = (char *)lpApplicationName;
 char **argv = NULL; errno_t status; ssize_t temp;
 char **envp = NULL;
 if (lpEnvironment) {
  /* TODO: Transform the given environment strings. */
  // CREATE_UNICODE_ENVIRONMENT; /* Contained strings are UTF-16 encoded. */
 }

 /* Load the given commandline. */
 if (lpCommandLine) {
  argv = K32_SplitCommandLine(lpCommandLine);
  if (!argv) {
   if (__get_errno() != EOK) goto end;
   argv = fixed_argv;
   fixed_argv[0] = NULL;
   fixed_argv[1] = NULL;
  }
  /* Use the first argument as appname. */
  if (!appname)
       appname = argv[0];
  /* make sure that we've got an appname now! */
  if (!appname) goto noapp;
 } else if (appname) {
  /* Use the given appname as a single argument. */
  argv = fixed_argv;
  fixed_argv[0] = appname;
  fixed_argv[1] = NULL;
 } else {
noapp:
  /* Neither appname, nor cmdline given. */
  SET_ERRNO(EINVAL);
  goto end;
 }

 /* Use pipes to confirm execution. */
 if (pipe2(confirm,O_CLOEXEC)) goto end;

 /* Time to get serious... */
 if ((child_pid = fork()) == 0) {
  bool keep_std_handles = false;
  close(confirm[0]); /* Close reader. */

  /* Switch to the given PWD before exec()-ing the new process. */
  if (lpCurrentDirectory &&
      dos_chdir(lpCurrentDirectory))
      goto err_child;

  /* Setup standard handles. */
  if (lpStartupInfo &&
     (lpStartupInfo->dwFlags&STARTF_USESTDHANDLES)) {
   int fd,i,temp_handles[3] = {-1,-1,-1};
   int std_handles[3] = {0,1,2};
   STATIC_ASSERT(STDIN_FILENO == 0);
   STATIC_ASSERT(STDOUT_FILENO == 1);
   STATIC_ASSERT(STDERR_FILENO == 2);
   /* Collect handles. */
   for (i = 0; i < 3; ++i) {
    if ((&lpStartupInfo->hStdInput)[i] != INVALID_HANDLE_VALUE) {
     if (!HANDLE_IS_FD((&lpStartupInfo->hStdInput)[i])) goto err_badf;
     fd = HANDLE_TO_FD((&lpStartupInfo->hStdInput)[i]);
     std_handles[i] = fd;
     if (fd != STDIN_FILENO && fd < 3 &&
        (std_handles[i] = temp_handles[i] = dup(fd)) < 0)
         goto err_child;
    }
   }

   /* Assign handles. */
   for (i = 0; i < 3; ++i) {
    if (std_handles[i] != i)
        dup2(std_handles[i],i);
   }

   /* Close temporary handles. */
   for (i = 0; i < 3; ++i)
       if (temp_handles[i] >= 0)
           close(temp_handles[i]);
   keep_std_handles = true;
  }

  if (!bInheritHandles) {
   /* TODO: KERNEL SUPPORT: closeall(minfd,maxfd);
    *       Close all handles except for 'confirm[1]'
    *      (And std handles if 'keep_std_handles' is true) */
   (void)keep_std_handles;
  }

  /* TODO: dwCreationFlags&CREATE_SUSPENDED
   * Requires kernel support: If exec() succeeds, the calling thread must be suspended. */

  /* NOTE: exec() will close the writer on success. */
  if (!envp) envp = environ;
  dos_execve(appname,argv,envp);
  dos_execvpe(appname,argv,envp);

  /* Try again after appending '.exe' to the appname. */
  /* XXX: DOS Only does this if there isn't already another extension... */
  { size_t applen = strlen(appname);
    char *appcopy = (char *)malloc((applen+5)*sizeof(char));
    if (appcopy) {
     memcpy(appcopy,appname,applen*sizeof(char));
     memcpy(appcopy+applen,".exe",5*sizeof(char));
     dos_execve(appcopy,argv,envp);
     dos_execvpe(appcopy,argv,envp);
     /* We'll just exit in second anyways. - No need for cleanup now... */
     //free(appcopy);
    }
  }

  /* If we can't start the process, tell our parent why. */
  status = __get_errno();
err_child:
  write(confirm[1],&status,sizeof(status));
  close(confirm[1]);
  _exit(127);
err_badf: status = EBADF; goto err_child;
 }
 close(confirm[1]); /* Close writer. */

 if (child_pid < 0) {
  /* Failed to fork() */
  close(confirm[0]); /* Close reader. */
  goto end;
 }

 /* Wait until until the pipe is closed, or an error is written. */
 do temp = read(confirm[0],&status,sizeof(status));
 while (temp < 0 && __get_errno() == EINTR);
 close(confirm[0]); /* Close reader. */

 if (!temp) {
  /* If the connection was interrupted, it's because exec() was successful. */
 } else {
  /* Otherwise, something went wrong. */
  kill(child_pid,SIGKILL); /* Just to be sure, also kill the child. */
  waitpid(child_pid,NULL,WEXITED); /* Now wait for the child. */
  goto end;
 }

 result = TRUE;

 /* Fill in process information. */
 if (lpProcessInformation) {
  lpProcessInformation->hProcess    = PID_TO_HANDLE(child_pid);
  lpProcessInformation->hThread     = PID_TO_HANDLE(child_pid);
  lpProcessInformation->dwProcessId = child_pid;
  lpProcessInformation->dwThreadId  = child_pid;
 }

end:
 /* Cleanup... */
 if (argv != fixed_argv)
     free(argv);
 return result;
}

INTERN WINBOOL WINAPI
K32_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine,
                   LPSECURITY_ATTRIBUTES UNUSED(lpProcessAttributes),
                   LPSECURITY_ATTRIBUTES UNUSED(lpThreadAttributes),
                   WINBOOL bInheritHandles, DWORD dwCreationFlags,
                   LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,
                   LPSTARTUPINFOA lpStartupInfo,
                   LPPROCESS_INFORMATION lpProcessInformation) {
 WINBOOL result; char *cmdline = NULL;
 if (lpCommandLine && (cmdline = strdup(lpCommandLine)) == NULL) return FALSE;
 result = K32_DO_CreateProcessA(lpApplicationName,cmdline,bInheritHandles,
                                dwCreationFlags,lpEnvironment,lpCurrentDirectory,
                                lpStartupInfo,lpProcessInformation);
 free(cmdline);
 return result;
}

INTERN WINBOOL WINAPI
K32_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                   LPSECURITY_ATTRIBUTES UNUSED(lpProcessAttributes),
                   LPSECURITY_ATTRIBUTES UNUSED(lpThreadAttributes),
                   WINBOOL bInheritHandles, DWORD dwCreationFlags,
                   LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                   LPSTARTUPINFOW lpStartupInfo,
                   LPPROCESS_INFORMATION lpProcessInformation) {
 WINBOOL result = FALSE;
 char *appname = NULL;
 char *cmdline = NULL;
 char *pwdpath = NULL;
 if (lpApplicationName && (appname = uni_utf16to8m(lpApplicationName)) == NULL) goto end;
 if (lpCommandLine && (cmdline = uni_utf16to8m(lpCommandLine)) == NULL) goto end;
 if (lpCurrentDirectory && (pwdpath = uni_utf16to8m(lpCurrentDirectory)) == NULL) goto end;
 result = K32_DO_CreateProcessA(appname,cmdline,bInheritHandles,
                                dwCreationFlags,lpEnvironment,pwdpath,
                               (LPSTARTUPINFOA)lpStartupInfo,
                                lpProcessInformation);
end:
 free(pwdpath);
 free(cmdline);
 free(appname);
 return result;
}
INTERN WINBOOL WINAPI
K32_CreateProcessAsUserA(HANDLE UNUSED(hToken), LPCSTR lpApplicationName, LPSTR lpCommandLine,
                         LPSECURITY_ATTRIBUTES lpProcessAttributes,
                         LPSECURITY_ATTRIBUTES lpThreadAttributes,
                         WINBOOL bInheritHandles, DWORD dwCreationFlags,
                         LPVOID lpEnvironment, LPCSTR lpCurrentDirectory,
                         LPSTARTUPINFOA lpStartupInfo,
                         LPPROCESS_INFORMATION lpProcessInformation) {
 return K32_CreateProcessA(lpApplicationName,lpCommandLine,lpProcessAttributes,
                           lpThreadAttributes,bInheritHandles,dwCreationFlags,
                           lpEnvironment,lpCurrentDirectory,lpStartupInfo,
                           lpProcessInformation);
}
INTERN WINBOOL WINAPI
K32_CreateProcessAsUserW(HANDLE UNUSED(hToken), LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                         LPSECURITY_ATTRIBUTES lpProcessAttributes,
                         LPSECURITY_ATTRIBUTES lpThreadAttributes,
                         WINBOOL bInheritHandles, DWORD dwCreationFlags,
                         LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
                         LPSTARTUPINFOW lpStartupInfo,
                         LPPROCESS_INFORMATION lpProcessInformation) {
 return K32_CreateProcessW(lpApplicationName,lpCommandLine,lpProcessAttributes,
                           lpThreadAttributes,bInheritHandles,dwCreationFlags,
                           lpEnvironment,lpCurrentDirectory,lpStartupInfo,
                           lpProcessInformation);
}





/* Critical section API. */
INTERN void WINAPI K32_InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {  NOT_IMPLEMENTED(); }
INTERN void WINAPI K32_EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {  NOT_IMPLEMENTED(); }
INTERN void WINAPI K32_LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {  NOT_IMPLEMENTED(); }
INTERN WINBOOL WINAPI K32_InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount) {  NOT_IMPLEMENTED(); return TRUE; }
INTERN DWORD WINAPI K32_SetCriticalSectionSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount) {  NOT_IMPLEMENTED(); return dwSpinCount; }
INTERN WINBOOL WINAPI K32_TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {  NOT_IMPLEMENTED(); return TRUE; }
INTERN void WINAPI K32_DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {  NOT_IMPLEMENTED(); }





/* Error code APIs. */
INTERN DWORD WINAPI K32_GetLastError(void) { return GET_NT_ERRNO(); }
INTERN void WINAPI K32_SetLastError(DWORD dwErrCode) { SET_NT_ERRNO(dwErrCode); }





/* Environment strings API. */
/* TODO: Collect all the necessary information from 'environ' */
INTERN LPCH WINAPI K32_GetEnvironmentStringsA(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN LPWCH WINAPI K32_GetEnvironmentStringsW(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN WINBOOL WINAPI K32_SetEnvironmentStringsA(LPCH NewEnvironment) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetEnvironmentStringsW(LPWCH NewEnvironment) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_FreeEnvironmentStringsA(LPCH lpEnv) { free(lpEnv); return lpEnv ? TRUE : FALSE; }
DEFINE_INTERN_ALIAS(K32_FreeEnvironmentStringsW,K32_FreeEnvironmentStringsA);
DEFINE_INTERN_ALIAS(K32_GetStartupInfoW,K32_GetStartupInfoA);
INTERN void WINAPI
K32_GetStartupInfoA(LPSTARTUPINFOA lpStartupInfo) {
 struct winsize size;
 memset(lpStartupInfo,0,sizeof(STARTUPINFOA));
 //lpStartupInfo->cb = 0; /* ??? */
 if (ioctl(STDIN_FILENO,TIOCGWINSZ,&size)) {
  lpStartupInfo->dwXSize       = size.ws_col*size.ws_xpixel;
  lpStartupInfo->dwYSize       = size.ws_row*size.ws_ypixel;
  lpStartupInfo->dwXCountChars = size.ws_col;
  lpStartupInfo->dwYCountChars = size.ws_row;
 }
 lpStartupInfo->dwFlags    = (STARTF_RUNFULLSCREEN|
                              STARTF_USESTDHANDLES);
 lpStartupInfo->hStdInput  = FD_TO_HANDLE(STDIN_FILENO);
 lpStartupInfo->hStdOutput = FD_TO_HANDLE(STDOUT_FILENO);
 lpStartupInfo->hStdError  = FD_TO_HANDLE(STDERR_FILENO);
}



/* Processor APIs. */
INTERN WINBOOL WINAPI
K32_IsProcessorFeaturePresent(DWORD ProcessorFeature) {
 (void)ProcessorFeature; /* TODO */
 return FALSE;
}
DEFINE_INTERN_ALIAS(K32_GetNativeSystemInfo,K32_GetSystemInfo);
INTERN void WINAPI
K32_GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
 if unlikely(!lpSystemInfo) return;
 lpSystemInfo->wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
 lpSystemInfo->dwPageSize = PAGESIZE;
 lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)0x00000000;
 lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)0xc0000000;
 lpSystemInfo->dwActiveProcessorMask = 1;
 lpSystemInfo->dwNumberOfProcessors = 1;
#ifdef __x86_64__
 lpSystemInfo->dwProcessorType = PROCESSOR_AMD_X8664;
#elif defined(__i386__)
 { PRIVATE DWORD processor_type = 0;
   if (!processor_type) {
    if (cpu_is_486()) {
#if 0 /* TODO: Check for 586 */
     processor_type = PROCESSOR_INTEL_PENTIUM;
#else
     processor_type = PROCESSOR_INTEL_486;
#endif
    } else {
     processor_type = PROCESSOR_INTEL_386;
    }
   }
   lpSystemInfo->dwProcessorType = processor_type;
 }
#else
#error FIXME
#endif
 lpSystemInfo->dwAllocationGranularity = PAGESIZE;
 lpSystemInfo->wProcessorLevel    = 0; /* ??? */
 lpSystemInfo->wProcessorRevision = 0; /* ??? */
}
INTERN WINBOOL WINAPI K32_SetSystemFileCacheSize(SIZE_T MinimumFileCacheSize, SIZE_T MaximumFileCacheSize, DWORD Flags) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetSystemFileCacheSize(PSIZE_T lpMinimumFileCacheSize, PSIZE_T lpMaximumFileCacheSize, PDWORD lpFlags) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetSystemRegistryQuota(PDWORD pdwQuotaAllowed, PDWORD pdwQuotaUsed) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize) {
 WINBOOL result = !gethostname(lpBuffer,*nSize);
 if (result) *nSize = strnlen(lpBuffer,MAX_COMPUTERNAME_LENGTH);
 return result;
}
INTERN WINBOOL WINAPI K32_SetComputerNameA(LPCSTR lpComputerName) {
 if (!lpComputerName) { SET_ERRNO(EINVAL); return FALSE; }
 return !sethostname(lpComputerName,strlen(lpComputerName));
}
INTERN WINBOOL WINAPI K32_GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize) {
 char buf[MAX_COMPUTERNAME_LENGTH+1];
 WINBOOL result; DWORD size = sizeof(buf);
 result = K32_GetComputerNameA(buf,&size);
 if (result) {
  mbstate_t state = MBSTATE_INIT;
  *nSize = (DWORD)uni_utf8to16(buf,size,lpBuffer,*nSize,&state,
                               UNICODE_F_ALWAYSZEROTERM|
                               UNICODE_F_STOPONNUL|UNICODE_F_NOFAIL);
 }
 return result;
}
INTERN WINBOOL WINAPI K32_SetComputerNameW(LPCWSTR lpComputerName) {
 char *name; WINBOOL result;
 if (!lpComputerName) { SET_ERRNO(EINVAL); return FALSE; }
 name = uni_utf16to8m(lpComputerName);
 if (!name) return FALSE;
 result = K32_SetComputerNameA(name);
 free(name);
 return result;
}



/* Pointer encode/decode. */
INTERN PVOID WINAPI K32_EncodePointer(PVOID p) { return p; }
DEFINE_INTERN_ALIAS(K32_DecodePointer,K32_EncodePointer);
DEFINE_INTERN_ALIAS(K32_EncodeSystemPointer,K32_EncodePointer);
DEFINE_INTERN_ALIAS(K32_DecodeSystemPointer,K32_EncodePointer);


/* Debug APIs. */
INTERN void    WINAPI K32_DebugBreak(void) { __asm__ __volatile__("int $3\n" : : : "memory"); }
INTERN WINBOOL WINAPI K32_IsDebuggerPresent(void) { return FALSE; }
INTERN WINBOOL WINAPI K32_DebugBreakProcess(HANDLE Process) {
 if unlikely(!HANDLE_IS_PID(Process)) { SET_ERRNO(EBADF); return FALSE; }
 /* You can't debug-break any other process than yourself. */
 if (HANDLE_TO_PID(Process) != (pid_t)K32_GetCurrentProcessId()) { SET_ERRNO(EACCES); return FALSE; }
 K32_DebugBreak();
 return TRUE;
}
INTERN void WINAPI
K32_OutputDebugStringA(LPCSTR lpOutputString) {
 if (lpOutputString) {
  /* Simply forward to the system log. */
  syslog_printer(lpOutputString,strlen(lpOutputString),
                 SYSLOG_PRINTER_CLOSURE(LOG_DEBUG));
 }
}
INTERN void WINAPI
K32_OutputDebugStringW(LPCWSTR lpOutputString) {
 if (lpOutputString) {
  mbstate_t state = MBSTATE_INIT;
  /* Simply pipe the given message through
   * a format printer and into the system log. */
  format_c16sntomb(&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_DEBUG),
                    lpOutputString,(size_t)-1,&state,UNICODE_F_NOFAIL);
 }
}



/* Thread/Process APIs. */
DEFINE_PUBLIC_ALIAS(GetProcessAffinityMask,K32_GetProcessAffinityMask);
DEFINE_PUBLIC_ALIAS(SetProcessAffinityMask,K32_SetProcessAffinityMask);
DEFINE_PUBLIC_ALIAS(GetProcessHandleCount,K32_GetProcessHandleCount);
DEFINE_PUBLIC_ALIAS(GetProcessTimes,K32_GetProcessTimes);
DEFINE_PUBLIC_ALIAS(GetProcessIoCounters,K32_GetProcessIoCounters);
DEFINE_PUBLIC_ALIAS(GetProcessWorkingSetSize,K32_GetProcessWorkingSetSize);
DEFINE_PUBLIC_ALIAS(GetProcessWorkingSetSizeEx,K32_GetProcessWorkingSetSizeEx);
DEFINE_PUBLIC_ALIAS(SetProcessWorkingSetSize,K32_SetProcessWorkingSetSize);
DEFINE_PUBLIC_ALIAS(SetProcessWorkingSetSizeEx,K32_SetProcessWorkingSetSizeEx);
DEFINE_PUBLIC_ALIAS(OpenProcess,K32_OpenProcess);
DEFINE_PUBLIC_ALIAS(GetCurrentProcess,K32_GetCurrentProcess);
DEFINE_PUBLIC_ALIAS(GetCurrentProcessId,K32_GetCurrentProcessId);
DEFINE_PUBLIC_ALIAS(ExitProcess,K32_ExitProcess);
DEFINE_PUBLIC_ALIAS(TerminateProcess,K32_TerminateProcess);
DEFINE_PUBLIC_ALIAS(GetExitCodeProcess,K32_GetExitCodeProcess);
DEFINE_PUBLIC_ALIAS(FatalExit,K32_FatalExit);
DEFINE_PUBLIC_ALIAS(GetCurrentThreadId,K32_GetCurrentThreadId);
DEFINE_PUBLIC_ALIAS(CreateFiber,K32_CreateFiber);
DEFINE_PUBLIC_ALIAS(CreateFiberEx,K32_CreateFiberEx);
DEFINE_PUBLIC_ALIAS(DeleteFiber,K32_DeleteFiber);
DEFINE_PUBLIC_ALIAS(ConvertThreadToFiber,K32_ConvertThreadToFiber);
DEFINE_PUBLIC_ALIAS(ConvertThreadToFiberEx,K32_ConvertThreadToFiberEx);
DEFINE_PUBLIC_ALIAS(ConvertFiberToThread,K32_ConvertFiberToThread);
DEFINE_PUBLIC_ALIAS(SwitchToFiber,K32_SwitchToFiber);
DEFINE_PUBLIC_ALIAS(SwitchToThread,K32_SwitchToThread);
DEFINE_PUBLIC_ALIAS(CreateThread,K32_CreateThread);
DEFINE_PUBLIC_ALIAS(CreateRemoteThread,K32_CreateRemoteThread);
DEFINE_PUBLIC_ALIAS(GetCurrentThread,K32_GetCurrentThread);
DEFINE_PUBLIC_ALIAS(GetCurrentThreadId,K32_GetCurrentThreadId);
DEFINE_PUBLIC_ALIAS(SetThreadStackGuarantee,K32_SetThreadStackGuarantee);
DEFINE_PUBLIC_ALIAS(GetProcessIdOfThread,K32_GetProcessIdOfThread);
DEFINE_PUBLIC_ALIAS(GetThreadId,K32_GetThreadId);
DEFINE_PUBLIC_ALIAS(GetProcessId,K32_GetProcessId);
DEFINE_PUBLIC_ALIAS(GetCurrentProcessorNumber,K32_GetCurrentProcessorNumber);
DEFINE_PUBLIC_ALIAS(SetThreadAffinityMask,K32_SetThreadAffinityMask);
DEFINE_PUBLIC_ALIAS(SetThreadIdealProcessor,K32_SetThreadIdealProcessor);
DEFINE_PUBLIC_ALIAS(SetProcessPriorityBoost,K32_SetProcessPriorityBoost);
DEFINE_PUBLIC_ALIAS(GetProcessPriorityBoost,K32_GetProcessPriorityBoost);
DEFINE_PUBLIC_ALIAS(RequestWakeupLatency,K32_RequestWakeupLatency);
DEFINE_PUBLIC_ALIAS(IsSystemResumeAutomatic,K32_IsSystemResumeAutomatic);
DEFINE_PUBLIC_ALIAS(OpenThread,K32_OpenThread);
DEFINE_PUBLIC_ALIAS(GetThreadPriority,K32_GetThreadPriority);
DEFINE_PUBLIC_ALIAS(SetThreadPriority,K32_SetThreadPriority);
DEFINE_PUBLIC_ALIAS(GetPriorityClass,K32_GetPriorityClass);
DEFINE_PUBLIC_ALIAS(SetPriorityClass,K32_SetPriorityClass);
DEFINE_PUBLIC_ALIAS(SetThreadPriorityBoost,K32_SetThreadPriorityBoost);
DEFINE_PUBLIC_ALIAS(GetThreadPriorityBoost,K32_GetThreadPriorityBoost);
DEFINE_PUBLIC_ALIAS(GetThreadTimes,K32_GetThreadTimes);
DEFINE_PUBLIC_ALIAS(GetThreadIOPendingFlag,K32_GetThreadIOPendingFlag);
DEFINE_PUBLIC_ALIAS(ExitThread,K32_ExitThread);
DEFINE_PUBLIC_ALIAS(TerminateThread,K32_TerminateThread);
DEFINE_PUBLIC_ALIAS(GetExitCodeThread,K32_GetExitCodeThread);
DEFINE_PUBLIC_ALIAS(GetThreadSelectorEntry,K32_GetThreadSelectorEntry);
DEFINE_PUBLIC_ALIAS(SetThreadExecutionState,K32_SetThreadExecutionState);
DEFINE_PUBLIC_ALIAS(IsWow64Process,K32_IsWow64Process);
DEFINE_PUBLIC_ALIAS(CreateProcessA,K32_CreateProcessA);
DEFINE_PUBLIC_ALIAS(CreateProcessW,K32_CreateProcessW);
DEFINE_PUBLIC_ALIAS(CreateProcessAsUserA,K32_CreateProcessAsUserA);
DEFINE_PUBLIC_ALIAS(CreateProcessAsUserW,K32_CreateProcessAsUserW);

/* Critical section API. */
DEFINE_PUBLIC_ALIAS(InitializeCriticalSection,K32_InitializeCriticalSection);
DEFINE_PUBLIC_ALIAS(EnterCriticalSection,K32_EnterCriticalSection);
DEFINE_PUBLIC_ALIAS(LeaveCriticalSection,K32_LeaveCriticalSection);
DEFINE_PUBLIC_ALIAS(InitializeCriticalSectionAndSpinCount,K32_InitializeCriticalSectionAndSpinCount);
DEFINE_PUBLIC_ALIAS(SetCriticalSectionSpinCount,K32_SetCriticalSectionSpinCount);
DEFINE_PUBLIC_ALIAS(TryEnterCriticalSection,K32_TryEnterCriticalSection);
DEFINE_PUBLIC_ALIAS(DeleteCriticalSection,K32_DeleteCriticalSection);

/* Error code APIs. */
DEFINE_PUBLIC_ALIAS(GetLastError,K32_GetLastError);
DEFINE_PUBLIC_ALIAS(SetLastError,K32_SetLastError);

/* Environment strings API. */
DEFINE_PUBLIC_ALIAS(GetEnvironmentStrings,K32_GetEnvironmentStringsA); /* Yes! This one is called 'GetEnvironmentStrings', not 'GetEnvironmentStringsA' */
DEFINE_PUBLIC_ALIAS(GetEnvironmentStringsW,K32_GetEnvironmentStringsW);
DEFINE_PUBLIC_ALIAS(SetEnvironmentStringsA,K32_SetEnvironmentStringsA);
DEFINE_PUBLIC_ALIAS(SetEnvironmentStringsW,K32_SetEnvironmentStringsW);
DEFINE_PUBLIC_ALIAS(FreeEnvironmentStringsA,K32_FreeEnvironmentStringsA);
DEFINE_PUBLIC_ALIAS(FreeEnvironmentStringsW,K32_FreeEnvironmentStringsW);
DEFINE_PUBLIC_ALIAS(GetStartupInfoA,K32_GetStartupInfoA);
DEFINE_PUBLIC_ALIAS(GetStartupInfoW,K32_GetStartupInfoW);

/* Processor APIs. */
DEFINE_PUBLIC_ALIAS(IsProcessorFeaturePresent,K32_IsProcessorFeaturePresent);
DEFINE_PUBLIC_ALIAS(GetSystemInfo,K32_GetSystemInfo);
DEFINE_PUBLIC_ALIAS(GetNativeSystemInfo,K32_GetNativeSystemInfo);
DEFINE_PUBLIC_ALIAS(SetSystemFileCacheSize,K32_SetSystemFileCacheSize);
DEFINE_PUBLIC_ALIAS(GetSystemFileCacheSize,K32_GetSystemFileCacheSize);
DEFINE_PUBLIC_ALIAS(GetSystemRegistryQuota,K32_GetSystemRegistryQuota);
DEFINE_PUBLIC_ALIAS(GetComputerNameA,K32_GetComputerNameA);
DEFINE_PUBLIC_ALIAS(GetComputerNameW,K32_GetComputerNameW);
DEFINE_PUBLIC_ALIAS(SetComputerNameA,K32_SetComputerNameA);
DEFINE_PUBLIC_ALIAS(SetComputerNameW,K32_SetComputerNameW);

/* Pointer encode/decode. */
DEFINE_PUBLIC_ALIAS(EncodePointer,K32_EncodePointer);
DEFINE_PUBLIC_ALIAS(DecodePointer,K32_DecodePointer);
DEFINE_PUBLIC_ALIAS(EncodeSystemPointer,K32_EncodeSystemPointer);
DEFINE_PUBLIC_ALIAS(DecodeSystemPointer,K32_DecodeSystemPointer);

/* Debug APIs. */
DEFINE_PUBLIC_ALIAS(DebugBreak,K32_DebugBreak);
DEFINE_PUBLIC_ALIAS(IsDebuggerPresent,K32_IsDebuggerPresent);
DEFINE_PUBLIC_ALIAS(DebugBreakProcess,K32_DebugBreakProcess);
DEFINE_PUBLIC_ALIAS(OutputDebugStringA,K32_OutputDebugStringA);
DEFINE_PUBLIC_ALIAS(OutputDebugStringW,K32_OutputDebugStringW);



DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_THREAD_C */
