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
#ifndef GUARD_LIBS_LIBKERNEL32_FILE_C
#define GUARD_LIBS_LIBKERNEL32_FILE_C 1
#define _LARGEFILE64_SOURCE 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1
#define _TIME64_SOURCE 1

#include "k32.h"
#include "file.h"
#include "time.h"

/* Since read()/write() used below can only pass 32-bit
 * buffer size arguments, link DOS's read()/write() functions
 * that also only take 32-bit arguments. */
#include <io.h>

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <unicode.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <kos/fcntl.h>
#include <bits/fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

DECL_BEGIN

/* DOS libc functions, addressed explicitly. */
__LIBC DIR *LIBCCALL dos_opendir(char const *dos_name) ASMNAME(".dos.opendir");
__LIBC DIR *LIBCCALL dos_opendirat(int dfd, char const *dos_name) ASMNAME(".dos.opendirat");
__LIBC int LIBCCALL dos_chdir(char const *dos_path) ASMNAME("_chdir");
__LIBC int LIBCCALL dos_rmdir(char const *dos_path) ASMNAME("_rmdir");
__LIBC int LIBCCALL dos_mkdir(char const *dos_path) ASMNAME("_mkdir");
__LIBC int LIBCCALL dos_mkdir2(char const *dos_path, mode_t mode) ASMNAME(".dos.mkdir");
__LIBC int LIBCCALL dos_stat(char const *dos_file, struct stat *buf) ASMNAME(".dos.kstat");
__LIBC int LIBCCALL dos_stat64(char const *dos_file, struct stat64 *buf) ASMNAME(".dos.kstat64");
__LIBC int LIBCCALL dos_fstatat(int fd, char const *dos_file, struct stat *buf, int flag) ASMNAME(".dos.kfstatat");
__LIBC int LIBCCALL dos_fstatat64(int fd, char const *dos_file, struct stat64 *buf, int flag) ASMNAME(".dos.kfstatat64");
__LIBC int LIBCCALL dos_lstat(char const *dos_file, struct stat *buf) ASMNAME(".dos.klstat");
__LIBC int LIBCCALL dos_lstat64(char const *dos_file, struct stat64 *buf) ASMNAME(".dos.klstat64");
__LIBC int ATTR_CDECL dos_open(char const *dos_file, int oflag, ...) ASMNAME("_open");
__LIBC int LIBCCALL dos_creat(char const *dos_file, mode_t mode) ASMNAME("_creat");
__LIBC int ATTR_CDECL dos_openat(int dfd, char const *dos_file, int oflag, ...) ASMNAME(".dos.openat");
__LIBC int LIBCCALL dos_chmod(char const *dos_file, int mode) ASMNAME("_chmod");
__LIBC int LIBCCALL dos_unlink(char const *dos_name) ASMNAME("_unlink");
__LIBC ssize_t LIBCCALL dos_readlink(char const *__restrict path, char *__restrict buf, size_t len) ASMNAME(".dos.readlink");
__LIBC ssize_t LIBCCALL dos_readlinkat(int fd, char const *__restrict path, char *__restrict buf, size_t len) ASMNAME(".dos.readlinkat");
__LIBC int LIBCCALL dos_rename(char const *__old, char const *__new) ASMNAME(".dos.rename");
__LIBC int LIBCCALL dos_unlinkat(int fd, char const *name, int flags) ASMNAME(".dos.unlinkat");


STATIC_ASSERT(OF_READ == O_RDONLY);
STATIC_ASSERT(OF_WRITE == O_WRONLY);
STATIC_ASSERT(OF_READWRITE == O_RDWR);
STATIC_ASSERT(HFILE_ERROR == -1); /* _hread() returns this on error, which should be the same as `read()' */


/* File-api Codepage API. */
PRIVATE ATTR_COLDBSS BOOL is_oem = FALSE;
INTERN ATTR_COLDTEXT void WINAPI K32_SetFileApisToOEM(void) { is_oem = TRUE; }
INTERN ATTR_COLDTEXT void WINAPI K32_SetFileApisToANSI(void) { is_oem = FALSE; }
INTERN ATTR_COLDTEXT WINBOOL WINAPI K32_AreFileApisANSI(void) { return !is_oem; }

INTERN HFILE WINAPI
K32_OpenFile(LPCSTR lpFileName, LPOFSTRUCT UNUSED(lpReOpenBuff), UINT uStyle) {
 oflag_t mode = uStyle&O_ACCMODE; /* Mostly guessed... */
 if (uStyle&OF_CREATE) mode |= O_CREAT|O_TRUNC;
 if (uStyle&OF_EXIST) mode |= O_EXCL;
 if (uStyle&OF_DELETE) mode |= O_TRUNC;
 return dos_open(lpFileName,mode);
}
INTERN HFILE WINAPI K32_lopen(LPCSTR lpPathName, int iReadWrite) { return dos_open(lpPathName,iReadWrite&O_ACCMODE); }
INTERN HFILE WINAPI K32_lcreat(LPCSTR lpPathName, int iAttribute) { return dos_creat(lpPathName,iAttribute == 1 ? 0444 : 0666); }
INTERN UINT  WINAPI K32_lread(HFILE hFile, LPVOID lpBuffer, UINT uBytes) { s32 res = read(hFile,lpBuffer,uBytes); return res < 0 ? 0 : (u32)res; }
INTERN UINT  WINAPI K32_lwrite(HFILE hFile, LPCCH lpBuffer, UINT uBytes) { s32 res = write(hFile,lpBuffer,uBytes); return res < 0 ? 0 : (u32)res; }
INTERN LONG  WINAPI K32_hread(HFILE hFile, LPVOID lpBuffer, LONG lBytes) { return (LONG)read(hFile,lpBuffer,(u32)lBytes); }
INTERN LONG  WINAPI K32_hwrite(HFILE hFile, LPCCH lpBuffer, LONG lBytes) { return (LONG)write(hFile,lpBuffer,(u32)lBytes); }
INTERN HFILE WINAPI K32_lclose(HFILE hFile) { close(hFile); return hFile; }
INTERN LONG  WINAPI K32_llseek(HFILE hFile, LONG lOffset, int iOrigin) { return (LONG)lseek(hFile,(off_t)lOffset,iOrigin); }



INTERN WINBOOL WINAPI K32_CloseHandle(HANDLE hObject) {
 switch (HANDLE_TYPE(hObject)) {
 case HANDLE_TYPE_FD:
  return !close(HANDLE_TO_FD(hObject));
 case HANDLE_TYPE_PID:
  /* Reap dead child processes.
   * >> Where functions like 'WaitForSingleObject()' will
   *    not cause process handles to be reaped, allowing for
   *    later calls to something like `GetExitCodeProcess()',
   *    closing such a handle must actually get rid of the
   *    child process.
   * >> With that in mind, simply reap the given process
   *    handle if the associated PID has exited.
   *    NOTE: 'WNOHANG' will prevent this function from hanging
   *           if the associated process hasn't died, such as
   *           when this function is called in the context of
   *           a child process being detached. */
  if (waitpid(HANDLE_TO_PID(hObject),NULL,WNOHANG|WEXITED) &&
      __get_errno() == ESRCH) return FALSE;
  return TRUE;
 default:
  SET_ERRNO(EBADF);
  break;
 }
 return FALSE;
}

INTERN WINBOOL WINAPI
K32_DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle,
                    HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle,
                    DWORD UNUSED(dwDesiredAccess),
                    WINBOOL bInheritHandle, DWORD UNUSED(dwOptions)) {
 int result;
 if (!lpTargetHandle) { SET_ERRNO(EINVAL); return FALSE; }
 if (!HANDLE_IS_PID(hSourceProcessHandle) ||
      hSourceProcessHandle != hTargetProcessHandle) {
  SET_ERRNO(EACCES);
  return FALSE;
 }

 switch (HANDLE_TYPE(hSourceHandle)) {
 case HANDLE_TYPE_FD:
  if (bInheritHandle) {
   result = dup(HANDLE_TO_FD(hSourceHandle));
  } else {
   result = fcntl(HANDLE_TO_FD(hSourceHandle),F_DUPFD_CLOEXEC);
  }
  if (result < 0) return FALSE;
  *lpTargetHandle = FD_TO_HANDLE(result);
  break;
 case HANDLE_TYPE_PID:
  *lpTargetHandle = hSourceHandle;
  break;

 default:
  SET_ERRNO(EPERM);
  return FALSE;
 }
 return TRUE;
}


INTERN WINBOOL WINAPI
K32_GetHandleInformation(HANDLE hObject, LPDWORD lpdwFlags) {
 /* TODO? */
 return FALSE;
}
INTERN WINBOOL WINAPI
K32_SetHandleInformation(HANDLE hObject, DWORD dwMask, DWORD dwFlags) {
 /* TODO? */
 return FALSE;
}

INTERN WINBOOL WINAPI
K32_GetFileTime(HANDLE hFile,
                LPFILETIME lpCreationTime,
                LPFILETIME lpLastAccessTime,
                LPFILETIME lpLastWriteTime) {
 struct stat64 val;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (fstat64(HANDLE_TO_FD(hFile),&val)) return FALSE;
 if (lpCreationTime)   *lpCreationTime   = K32_TimespecToFiletime(val.st_ctim64);
 if (lpLastAccessTime) *lpLastAccessTime = K32_TimespecToFiletime(val.st_atim64);
 if (lpLastWriteTime)  *lpLastWriteTime  = K32_TimespecToFiletime(val.st_mtim64);
 return TRUE;
}
INTERN WINBOOL WINAPI
K32_SetFileTime(HANDLE hFile,
                CONST FILETIME *UNUSED(lpCreationTime), /* Not allowed to be changed. */
                CONST FILETIME *lpLastAccessTime,
                CONST FILETIME *lpLastWriteTime) {
 struct timespec64 tms[2];
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (lpLastAccessTime)
      tms[0] = K32_FiletimeToTimespec(*lpLastAccessTime);
 else tms[0].tv_nsec = UTIME_OMIT;
 if (lpLastWriteTime)
      tms[1] = K32_FiletimeToTimespec(*lpLastWriteTime);
 else tms[1].tv_nsec = UTIME_OMIT;
 return !futimens64(HANDLE_TO_FD(hFile),tms);
}

INTERN DWORD WINAPI
K32_SetFilePointer(HANDLE hFile,
                   LONG lDistanceToMove,
                   PLONG lpDistanceToMoveHigh,
                   DWORD dwMoveMethod) {
 off64_t result;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return INVALID_SET_FILE_POINTER; }
 result = lDistanceToMove;
 if (lpDistanceToMoveHigh) result |= (off64_t)*lpDistanceToMoveHigh << 32;
 result = lseek64(HANDLE_TO_FD(hFile),result,(int)dwMoveMethod);
 if (result < 0) return INVALID_SET_FILE_POINTER;
 if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = (u32)((u64)result >> 32);
 return (u32)result;
}

INTERN WINBOOL WINAPI
K32_SetFilePointerEx(HANDLE hFile,
                     LARGE_INTEGER liDistanceToMove,
                     PLARGE_INTEGER lpNewFilePointer,
                     DWORD dwMoveMethod) {
 off64_t result;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 result = lseek64(HANDLE_TO_FD(hFile),liDistanceToMove.QuadPart,(int)dwMoveMethod);
 if (result < 0) return FALSE;
 if (lpNewFilePointer) lpNewFilePointer->QuadPart = result;
 return TRUE;
}

INTERN WINBOOL WINAPI K32_SetEndOfFile(HANDLE hFile) {
 off64_t file_length;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 file_length = lseek64(HANDLE_TO_FD(hFile),0,SEEK_CUR);
 if (file_length < 0) return FALSE;
 return !ftruncate64(HANDLE_TO_FD(hFile),file_length);
}

INTERN WINBOOL WINAPI K32_FlushFileBuffers(HANDLE hFile) {
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 return !fdatasync(HANDLE_TO_FD(hFile));
}

INTERN HANDLE WINAPI K32_GetStdHandle(DWORD nStdHandle) {
 switch (nStdHandle) {
 case STD_INPUT_HANDLE:  return FD_TO_HANDLE(STDIN_FILENO);
 case STD_OUTPUT_HANDLE: return FD_TO_HANDLE(STDOUT_FILENO);
 case STD_ERROR_HANDLE:  return FD_TO_HANDLE(STDERR_FILENO);
 default: break;
 }
 SET_ERRNO(EINVAL);
 return INVALID_HANDLE_VALUE;
}

INTERN WINBOOL WINAPI
K32_SetStdHandle(DWORD nStdHandle, HANDLE hHandle) {
 HANDLE handno = K32_GetStdHandle(nStdHandle);
 if (!HANDLE_IS_FD(handno)) return FALSE;
 if (!HANDLE_IS_FD(hHandle)) { SET_ERRNO(EBADF); return FALSE; }
 return !dup2(HANDLE_TO_FD(handno),HANDLE_TO_FD(hHandle));
}

INTERN WINBOOL WINAPI
K32_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
              LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
 ssize_t error;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (lpOverlapped) {
  off64_t write_pos; /* pwrite64() */
  write_pos  = (off64_t)lpOverlapped->Offset;
  write_pos |= (off64_t)lpOverlapped->OffsetHigh << 32;
  error = pwrite64(HANDLE_TO_FD(hFile),lpBuffer,nNumberOfBytesToWrite,write_pos);
 } else {
  error = write(HANDLE_TO_FD(hFile),lpBuffer,nNumberOfBytesToWrite);
 }
 if (error < 0) return FALSE;
 if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)error;
 return TRUE;
}
INTERN WINBOOL WINAPI
K32_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
             LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
 ssize_t error;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (lpOverlapped) {
  off64_t write_pos; /* pread64() */
  write_pos  = (off64_t)lpOverlapped->Offset;
  write_pos |= (off64_t)lpOverlapped->OffsetHigh << 32;
  error = pread64(HANDLE_TO_FD(hFile),lpBuffer,nNumberOfBytesToRead,write_pos);
 } else {
  error = read(HANDLE_TO_FD(hFile),lpBuffer,nNumberOfBytesToRead);
 }
 if (error < 0) return FALSE;
 if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)error;
 return TRUE;
}
INTERN DWORD WINAPI
K32_GetFileAttributesFromUnixMode(mode_t val) {
 DWORD result = 0;
 if (!(val&0222))  result |= FILE_ATTRIBUTE_READONLY;
 if (S_ISDIR(val)) result |= FILE_ATTRIBUTE_DIRECTORY;
 if (S_ISDEV(val)) result |= FILE_ATTRIBUTE_DEVICE;
 if (S_ISREG(val)) result |= FILE_ATTRIBUTE_NORMAL;
 if (S_ISLNK(val)) result |= FILE_ATTRIBUTE_REPARSE_POINT;
 return result;
}

INTERN WINBOOL WINAPI
K32_GetFileInformationByHandle(HANDLE hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation) {
 struct stat64 info;
 if (!lpFileInformation) { SET_ERRNO(EINVAL); return FALSE; }
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (fstat64(HANDLE_TO_FD(hFile),&info)) return FALSE;
 /* Translate information to DOS format. */
 //memset(lpFileInformation,0,sizeof(BY_HANDLE_FILE_INFORMATION));
 lpFileInformation->dwFileAttributes     = K32_GetFileAttributesFromUnixMode(info.st_mode);
 lpFileInformation->ftCreationTime       = K32_TimespecToFiletime(info.st_ctim64);
 lpFileInformation->ftLastAccessTime     = K32_TimespecToFiletime(info.st_atim64);
 lpFileInformation->ftLastWriteTime      = K32_TimespecToFiletime(info.st_mtim64);
 lpFileInformation->dwVolumeSerialNumber = info.st_dev; /* Why not? */
 lpFileInformation->nFileSizeHigh        = (u32)((u64)info.st_size64 >> 32);
 lpFileInformation->nFileSizeLow         = info.st_size32;
 lpFileInformation->nNumberOfLinks       = info.st_nlink;
 lpFileInformation->nFileIndexHigh       = (u32)((u64)info.st_ino64 >> 32);
 lpFileInformation->nFileIndexLow        = info.st_ino32;
 return TRUE;
}

INTERN DWORD WINAPI K32_GetFileType(HANDLE hFile) {
 struct stat64 info;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FILE_TYPE_UNKNOWN; }
 if (fstat64(HANDLE_TO_FD(hFile),&info)) return FILE_TYPE_UNKNOWN;
 if (S_ISCHR(info.st_mode)) return FILE_TYPE_CHAR;
 if (S_ISFIFO(info.st_mode)) return FILE_TYPE_PIPE;
 if (S_ISSOCK(info.st_mode)) return FILE_TYPE_REMOTE;
 return FILE_TYPE_DISK;
}

INTERN WINBOOL WINAPI
K32_GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
 struct stat64 info;
 if (!lpFileSize) { SET_ERRNO(EINVAL); return FALSE; }
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (fstat64(HANDLE_TO_FD(hFile),&info)) return FALSE;
 lpFileSize->QuadPart = info.st_size64;
 return TRUE;
}

INTERN DWORD WINAPI
K32_GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
 LARGE_INTEGER result;
 if (!K32_GetFileSizeEx(hFile,&result)) return INVALID_FILE_SIZE;
 if (lpFileSizeHigh) *lpFileSizeHigh = result.u.HighPart;
 return result.u.LowPart;
}
INTERN WINBOOL WINAPI
K32_LockFileEx(HANDLE hFile, DWORD dwFlags, DWORD UNUSED(dwReserved),
               DWORD nNumberOfBytesToLockLow,
               DWORD nNumberOfBytesToLockHigh,
               LPOVERLAPPED lpOverlapped) {
 struct flock64 lock;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 if (!lpOverlapped) { SET_ERRNO(EINVAL); return FALSE; }
 lock.l_start  = *(u64 *)&lpOverlapped->Offset;
 lock.l_len    = (u64)nNumberOfBytesToLockHigh << 32 | nNumberOfBytesToLockLow;
 lock.l_type   = dwFlags&LOCKFILE_EXCLUSIVE_LOCK ? F_WRLCK : F_RDLCK;
 if (dwFlags&LOCKFILE_FAIL_IMMEDIATELY) lock.l_type |= LOCK_NB;
 lock.l_whence = SEEK_SET;
 lock.l_pid    = getpid(); /* XXX: Will this be required? */
 return !fcntl(HANDLE_TO_FD(hFile),F_SETLK64,&lock);
}
INTERN WINBOOL WINAPI
K32_UnlockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh,
               DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh) {
 struct flock64 lock;
 if (!HANDLE_IS_FD(hFile)) { SET_ERRNO(EBADF); return FALSE; }
 lock.l_start  = (u64)dwFileOffsetHigh << 32 | dwFileOffsetLow;
 lock.l_len    = (u64)nNumberOfBytesToUnlockHigh << 32 | nNumberOfBytesToUnlockLow;
 lock.l_type   = F_UNLCK;
 lock.l_whence = SEEK_SET;
 lock.l_pid    = getpid(); /* XXX: Will this be required? */
 return !fcntl(HANDLE_TO_FD(hFile),F_SETLK64,&lock);
}
INTERN WINBOOL WINAPI
K32_LockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh,
             DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh) {
 OVERLAPPED start = { .Offset = nNumberOfBytesToLockLow,
                      .OffsetHigh = nNumberOfBytesToLockHigh, };
 return K32_LockFileEx(hFile,LOCKFILE_EXCLUSIVE_LOCK,0,
                       nNumberOfBytesToLockLow,
                       nNumberOfBytesToLockHigh,&start);
}






/* Extended File APIs. */
INTERN WINBOOL WINAPI
K32_ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
               LPOVERLAPPED lpOverlapped,
               LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
 DWORD read_ok = 0,did_read; OVERLAPPED my_overlap;
 if (lpOverlapped) my_overlap = *lpOverlapped;
 while (nNumberOfBytesToRead) {
  if (!K32_ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,&did_read,
                    lpOverlapped ? &my_overlap : NULL)) {
call_error:
   if (lpCompletionRoutine)
     (*lpCompletionRoutine)(GET_NT_ERRNO(),read_ok,lpOverlapped);
   return FALSE;
  }
  assert(did_read <= nNumberOfBytesToRead);
  if (!did_read) { SET_NT_ERRNO(ERROR_BROKEN_PIPE); goto call_error; }
  if (lpOverlapped) *(u64 *)&my_overlap.Offset += did_read;
  *(uintptr_t *)&lpBuffer += did_read;
  nNumberOfBytesToRead -= did_read;
  read_ok += did_read;
 }
 return TRUE;
}
INTERN WINBOOL WINAPI
K32_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
               LPOVERLAPPED lpOverlapped,
               LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
 DWORD write_ok = 0,did_write; OVERLAPPED my_overlap;
 if (lpOverlapped) my_overlap = *lpOverlapped;
 while (nNumberOfBytesToWrite) {
  if (!K32_WriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,&did_write,
                     lpOverlapped ? &my_overlap : NULL)) {
call_error:
   if (lpCompletionRoutine)
     (*lpCompletionRoutine)(GET_NT_ERRNO(),write_ok,lpOverlapped);
   return FALSE;
  }
  assert(did_write <= nNumberOfBytesToWrite);
  if (!did_write) { SET_NT_ERRNO(ERROR_BROKEN_PIPE); goto call_error; }
  if (lpOverlapped) *(u64 *)&my_overlap.Offset += did_write;
  *(uintptr_t *)&lpBuffer += did_write;
  nNumberOfBytesToWrite -= did_write;
  write_ok += did_write;
 }
 return TRUE;
}

INTERN WINBOOL WINAPI
K32_BackupRead(HANDLE hFile, LPBYTE lpBuffer, DWORD nNumberOfBytesToRead,
               LPDWORD lpNumberOfBytesRead, WINBOOL UNUSED(bAbort),
               WINBOOL UNUSED(bProcessSecurity), LPVOID *UNUSED(lpContext)) {
 return K32_ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,NULL);
}
INTERN WINBOOL WINAPI
K32_BackupSeek(HANDLE hFile, DWORD dwLowBytesToSeek, DWORD dwHighBytesToSeek,
               LPDWORD lpdwLowByteSeeked, LPDWORD lpdwHighByteSeeked, LPVOID *UNUSED(lpContext)) {
 DWORD error = K32_SetFilePointer(hFile,dwLowBytesToSeek,(PLONG)&dwHighBytesToSeek,SEEK_SET);
 if (error == INVALID_SET_FILE_POINTER && GET_ERRNO() != EOK)
     return FALSE;
 if (lpdwLowByteSeeked) *lpdwLowByteSeeked = error;
 if (lpdwHighByteSeeked) *lpdwHighByteSeeked = dwHighBytesToSeek;
 return TRUE;
}
INTERN WINBOOL WINAPI
K32_BackupWrite(HANDLE hFile, LPBYTE lpBuffer, DWORD nNumberOfBytesToWrite,
                LPDWORD lpNumberOfBytesWritten, WINBOOL UNUSED(bAbort),
                WINBOOL UNUSED(bProcessSecurity), LPVOID *UNUSED(lpContext)) {
 return K32_WriteFile(hFile,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,NULL);
}
INTERN WINBOOL WINAPI
K32_ReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[],
                    DWORD nNumberOfBytesToRead, LPDWORD UNUSED(lpReserved),
                    LPOVERLAPPED lpOverlapped) {
 OVERLAPPED my_overlap;
 if (lpOverlapped) my_overlap = *lpOverlapped;
 while (nNumberOfBytesToRead) {
  DWORD partsize = MIN(PAGESIZE,nNumberOfBytesToRead);
  if (!K32_ReadFileEx(hFile,(LPVOID)aSegmentArray[0].Buffer,partsize,
                      lpOverlapped ? &my_overlap : NULL,NULL))
       return FALSE;
  if (lpOverlapped) *(u64 *)&my_overlap.Offset += partsize;
  nNumberOfBytesToRead -= partsize;
  ++aSegmentArray;
 }
 return TRUE;
}
INTERN WINBOOL WINAPI
K32_WriteFileGather(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[],
                    DWORD nNumberOfBytesToWrite, LPDWORD lpReserved,
                    LPOVERLAPPED lpOverlapped) {
 OVERLAPPED my_overlap;
 if (lpOverlapped) my_overlap = *lpOverlapped;
 while (nNumberOfBytesToWrite) {
  DWORD partsize = MIN(PAGESIZE,nNumberOfBytesToWrite);
  if (!K32_WriteFileEx(hFile,(LPVOID)aSegmentArray[0].Buffer,partsize,
                       lpOverlapped ? &my_overlap : NULL,NULL))
       return FALSE;
  if (lpOverlapped) *(u64 *)&my_overlap.Offset += partsize;
  nNumberOfBytesToWrite -= partsize;
  ++aSegmentArray;
 }
 return TRUE;
}








struct find_query {
 DIR              *fq_stream; /*< [1..1][owned] Underlying libc-style directory stream. */
 char             *fq_query;  /*< [1..1][owned] Wildcard-style string matching */
 FINDEX_SEARCH_OPS fq_mode;   /*< [const] Search mode. */
};

#define FINDHANDLE_ISVALID(p) \
  ((p) && (p) != INVALID_HANDLE_VALUE)

INTERN HANDLE WINAPI
K32_FindFirstFileExA(LPCSTR lpFileName,
                     FINDEX_INFO_LEVELS fInfoLevelId,
                     LPVOID lpFindFileData,
                     FINDEX_SEARCH_OPS fSearchOp,
                     LPVOID UNUSED(lpSearchFilter),
                     DWORD UNUSED(dwAdditionalFlags)) {
 char const *query_start,*temp; DIR *stream;
 struct find_query *result;
 if (fInfoLevelId != FindExInfoStandard ||
     (unsigned int)fSearchOp >= FindExSearchMaxSearchOp) {
  SET_ERRNO(EINVAL);
  return INVALID_HANDLE_VALUE;
 }

 result = omalloc(struct find_query);
 if unlikely(!result) return INVALID_HANDLE_VALUE;

 /* Pull apart the query pattern and folder to search. */
 query_start = strrchr(lpFileName,'\\');
 temp = strrchr(lpFileName,'/');
 if (!query_start || temp < query_start) query_start = temp;
 if (!query_start) query_start = lpFileName;
 else ++query_start;

 /* Copy the query pattern portion. */
 result->fq_query = strdup(query_start);
 if unlikely(!result->fq_query) goto err;

 if (query_start == lpFileName)
     stream = dos_opendir(".");
 else {
  size_t folder_size = (query_start-lpFileName);
  char *folder = (char *)malloc(folder_size*sizeof(char));
  if unlikely(!folder) goto err2;
  memcpy(folder,lpFileName,(folder_size-1)*sizeof(char));
  folder[folder_size] = '\0';
  stream = dos_opendir(folder);
  free(folder);
 }
 if unlikely(!stream) goto err2;

 /* All right! We've got a stream opened up! */
 result->fq_stream = stream;
 result->fq_mode   = fSearchOp;

 /* Yield the first entry. */
 if (!K32_FindNextFileA(result,(LPWIN32_FIND_DATAA)lpFindFileData))
      goto err3;

 return result;
err3: closedir(result->fq_stream);
err2: free(result->fq_query);
err:  free(result);
 return INVALID_HANDLE_VALUE;
}
INTERN WINBOOL WINAPI
K32_FindNextFileA(HANDLE hFindFile,
                  LPWIN32_FIND_DATAA lpFindFileData) {
 struct find_query *query;
 struct dirent64 *ent;
 struct stat64 info;
 if (!FINDHANDLE_ISVALID(hFindFile)) { SET_ERRNO(EINVAL); return FALSE; }
 query = (struct find_query *)hFindFile;
 for (;;) {
  ent = readdir64(query->fq_stream);
  if (!ent) return FALSE;
  /* Do a wildcard match against the stored query string. */
  if (wildstrcasecmp(ent->d_name,query->fq_query) != 0)
      continue;
  /* Got a match! */
  switch (query->fq_mode) {
  case FindExSearchLimitToDirectories:
   if (ent->d_type != DT_DIR) continue;
   break;
  case FindExSearchLimitToDevices:
   if (ent->d_type != DT_CHR &&
       ent->d_type != DT_BLK) continue;
   break;
  default: break;
  }
  break;
 }
 /* Load file status information for this entry. */
 if (fstatat64(dirfd(query->fq_stream),ent->d_name,&info,AT_SYMLINK_NOFOLLOW))
     memset(&info,0,sizeof(struct stat64));
 /* Fill in information about this entry. */
 lpFindFileData->dwFileAttributes = K32_GetFileAttributesFromUnixMode(info.st_mode);
 lpFindFileData->ftCreationTime   = K32_TimespecToFiletime(info.st_ctim64);
 lpFindFileData->ftLastAccessTime = K32_TimespecToFiletime(info.st_atim64);
 lpFindFileData->ftLastWriteTime  = K32_TimespecToFiletime(info.st_mtim64);
 lpFindFileData->nFileSizeHigh    = (u32)(info.st_size64 >> 32);
 lpFindFileData->nFileSizeLow     = (u32)info.st_size32;
 lpFindFileData->dwReserved0      = 0;
 lpFindFileData->dwReserved1      = 0;
 memcpy(lpFindFileData->cFileName,ent->d_name,
        MIN(_D_EXACT_NAMLEN(ent)*sizeof(char),
            sizeof(lpFindFileData->cFileName)));
 /* Just re-use the first 13 characters as 'AlternateFileName'... */
 memcpy(lpFindFileData->cAlternateFileName,
        lpFindFileData->cFileName,13*sizeof(char));
 COMPILER_ENDOF(lpFindFileData->cFileName)[-1] = '\0';
 COMPILER_ENDOF(lpFindFileData->cAlternateFileName)[-1] = '\0';
 return TRUE;
}

INTERN WINBOOL WINAPI K32_FindClose(HANDLE hFindFile) {
 struct find_query *query;
 if (!FINDHANDLE_ISVALID(hFindFile)) { SET_ERRNO(EINVAL); return FALSE; }
 query = (struct find_query *)hFindFile;
 closedir(query->fq_stream);
 free(query->fq_query);
 free(query);
 return TRUE;
}



PRIVATE void
K32_FinddataAToFinddataW(LPWIN32_FIND_DATAW dst,
                         LPWIN32_FIND_DATAA src) {
 mbstate_t state;
 memcpy(dst,src,offsetof(WIN32_FIND_DATAA,cFileName));
 mbstate_reset(&state);
 uni_utf8to16(src->cFileName,COMPILER_LENOF(src->cFileName),
              dst->cFileName,COMPILER_LENOF(src->cFileName),
             &state,UNICODE_F_STOPONNUL|UNICODE_F_ALWAYSZEROTERM);
 mbstate_reset(&state);
 uni_utf8to16(src->cAlternateFileName,COMPILER_LENOF(src->cAlternateFileName),
              dst->cAlternateFileName,COMPILER_LENOF(src->cAlternateFileName),
             &state,UNICODE_F_STOPONNUL|UNICODE_F_ALWAYSZEROTERM);
}
INTERN HANDLE WINAPI
K32_FindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId,
                     LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp,
                     LPVOID lpSearchFilter, DWORD dwAdditionalFlags) {
 WIN32_FIND_DATAA temp; HANDLE result; char *query;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return INVALID_HANDLE_VALUE; }
 query = uni_utf16to8m(lpFileName);
 if unlikely(!query) return INVALID_HANDLE_VALUE;
 result = K32_FindFirstFileExA(query,fInfoLevelId,&temp,fSearchOp,
                               lpSearchFilter,dwAdditionalFlags);
 free(query);
 if (result != INVALID_HANDLE_VALUE)
     K32_FinddataAToFinddataW((LPWIN32_FIND_DATAW)lpFindFileData,&temp);
 return result;
}
INTERN WINBOOL WINAPI
K32_FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData) {
 WIN32_FIND_DATAA temp; WINBOOL result;
 result = K32_FindNextFileA(hFindFile,&temp);
 if (result) K32_FinddataAToFinddataW(lpFindFileData,&temp);
 return result;
}
INTERN HANDLE WINAPI
K32_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
 return K32_FindFirstFileExA(lpFileName,FindExInfoStandard,lpFindFileData,
                             FindExSearchNameMatch,NULL,0);
}
INTERN HANDLE WINAPI
K32_FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData) {
 return K32_FindFirstFileExW(lpFileName,FindExInfoStandard,lpFindFileData,
                             FindExSearchNameMatch,NULL,0);
}






/* Unimplemented... */
INTERN WINBOOL WINAPI
K32_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode,
                    LPVOID lpInBuffer, DWORD nInBufferSize,
                    LPVOID lpOutBuffer, DWORD nOutBufferSize,
                    LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped) {
 SET_ERRNO(ENOSYS);
 return FALSE;
}
INTERN WINBOOL WINAPI K32_RequestDeviceWakeup(HANDLE hDevice) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_CancelDeviceWakeupRequest(HANDLE hDevice) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_GetDevicePowerState(HANDLE hDevice, WINBOOL *pfOn) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetMessageWaitingIndicator(HANDLE hMsgIndicator, ULONG ulMsgCount) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetFileValidData(HANDLE hFile, LONGLONG ValidDataLength) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetFileShortNameA(HANDLE hFile, LPCSTR lpShortName) { NOT_IMPLEMENTED(); return FALSE; }
INTERN WINBOOL WINAPI K32_SetFileShortNameW(HANDLE hFile, LPCWSTR lpShortName) { NOT_IMPLEMENTED(); return FALSE; }



/* PWD access. */
INTERN WINBOOL WINAPI K32_SetCurrentDirectoryA(LPCSTR lpPathName) { return !dos_chdir(lpPathName); }
INTERN WINBOOL WINAPI K32_SetCurrentDirectoryW(LPCWSTR lpPathName) {
 char *path; WINBOOL result;
 if unlikely(!lpPathName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpPathName);
 if unlikely(!path) return FALSE;
 result = K32_SetCurrentDirectoryA(path);
 free(path);
 return result;
}
INTERN DWORD WINAPI K32_GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) {
 ssize_t result = xfdname2(AT_FDCWD,FDNAME_PATH,lpBuffer,nBufferLength);
 if (result < 1) result = 1;
 return (DWORD)(result-1); /* Don't include the terminating \0-character */
}
INTERN DWORD WINAPI K32_GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer) {
 size_t result; mbstate_t state = MBSTATE_INIT;
 char *pwd = getcwd(NULL,0); if (!pwd) return 0;
 result = uni_utf8to16(pwd,(size_t)-1,lpBuffer,nBufferLength,&state,
                       UNICODE_F_STOPONNUL|UNICODE_F_ALWAYSZEROTERM);
 if (result) --result; /* Don't include the terminating \0-character */
 free(pwd);
 return result;
}


/* Disk-free access. */
INTERN WINBOOL WINAPI
K32_GetDiskFreeSpaceExA(LPCSTR lpDirectoryName,
                        PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                        PULARGE_INTEGER lpTotalNumberOfBytes,
                        PULARGE_INTEGER lpTotalNumberOfFreeBytes) {
 if unlikely(!lpDirectoryName) { SET_ERRNO(EINVAL); return FALSE; }
 NOT_IMPLEMENTED();
 return FALSE;
}
INTERN WINBOOL WINAPI
K32_GetDiskFreeSpaceA(LPCSTR lpRootPathName,
                      LPDWORD lpSectorsPerCluster,
                      LPDWORD lpBytesPerSector,
                      LPDWORD lpNumberOfFreeClusters,
                      LPDWORD lpTotalNumberOfClusters) {
 if unlikely(!lpRootPathName) { SET_ERRNO(EINVAL); return FALSE; }
 NOT_IMPLEMENTED();
 return FALSE;
}

INTERN WINBOOL WINAPI
K32_GetDiskFreeSpaceExW(LPCWSTR lpDirectoryName,
                        PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                        PULARGE_INTEGER lpTotalNumberOfBytes,
                        PULARGE_INTEGER lpTotalNumberOfFreeBytes) {
 WINBOOL result; char *path;
 if unlikely(!lpDirectoryName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpDirectoryName);
 if unlikely(!path) return FALSE;
 result = K32_GetDiskFreeSpaceExA(path,
                                  lpFreeBytesAvailableToCaller,
                                  lpTotalNumberOfBytes,
                                  lpTotalNumberOfFreeBytes);
 free(path);
 return result;
}
INTERN WINBOOL WINAPI
K32_GetDiskFreeSpaceW(LPCWSTR lpRootPathName,
                      LPDWORD lpSectorsPerCluster,
                      LPDWORD lpBytesPerSector,
                      LPDWORD lpNumberOfFreeClusters,
                      LPDWORD lpTotalNumberOfClusters) {
 WINBOOL result; char *path;
 if unlikely(!lpRootPathName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpRootPathName);
 if unlikely(!path) return FALSE;
 result = K32_GetDiskFreeSpaceA(path,
                                lpSectorsPerCluster,
                                lpBytesPerSector,
                                lpNumberOfFreeClusters,
                                lpTotalNumberOfClusters);
 free(path);
 return result;
}


/* Directory create/delete API. */
INTERN WINBOOL WINAPI
K32_CreateDirectoryA(LPCSTR lpPathName,
                     LPSECURITY_ATTRIBUTES UNUSED(lpSecurityAttributes)) {
 return !dos_mkdir(lpPathName);
}

INTERN WINBOOL WINAPI K32_RemoveDirectoryA(LPCSTR lpPathName) { return !dos_rmdir(lpPathName); }
INTERN WINBOOL WINAPI
K32_CreateDirectoryW(LPCWSTR lpPathName,
                     LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
 char *path; WINBOOL result;
 if unlikely(!lpPathName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpPathName);
 if unlikely(!path) return FALSE;
 result = K32_CreateDirectoryA(path,lpSecurityAttributes);
 free(path);
 return result;
}
INTERN WINBOOL WINAPI
K32_RemoveDirectoryW(LPCWSTR lpPathName) {
 char *path; WINBOOL result;
 if unlikely(!lpPathName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpPathName);
 if unlikely(!path) return FALSE;
 result = K32_RemoveDirectoryA(path);
 free(path);
 return result;
}
INTERN WINBOOL WINAPI
K32_CreateDirectoryExA(LPCSTR UNUSED(lpTemplateDirectory),
                       LPCSTR lpNewDirectory,
                       LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
 return K32_CreateDirectoryA(lpNewDirectory,lpSecurityAttributes);
}
INTERN WINBOOL WINAPI
K32_CreateDirectoryExW(LPCWSTR UNUSED(lpTemplateDirectory),
                       LPCWSTR lpNewDirectory,
                       LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
 return K32_CreateDirectoryW(lpNewDirectory,lpSecurityAttributes);
}


PRIVATE int const creation_disk_flags[] = {
    [CREATE_NEW       ] = O_CREAT|O_TRUNC,
    [CREATE_ALWAYS    ] = O_CREAT|O_EXCL,
    [OPEN_EXISTING    ] = 0,
    [OPEN_ALWAYS      ] = O_CREAT,
    [TRUNCATE_EXISTING] = O_TRUNC,
};

/* File creation/attribute API. */
INTERN oflag_t WINAPI
K32_DesiredAccessToOflags(DWORD dwDesiredAccess) {
 oflag_t result = O_RDONLY;
 if (dwDesiredAccess&GENERIC_ALL)
     dwDesiredAccess |= (GENERIC_READ|GENERIC_WRITE);
 if (dwDesiredAccess&(FILE_WRITE_DATA|FILE_APPEND_DATA|
                      FILE_WRITE_EA|FILE_DELETE_CHILD))
     dwDesiredAccess |= GENERIC_WRITE;
 if (dwDesiredAccess&(FILE_READ_DATA|FILE_READ_EA))
     dwDesiredAccess |= GENERIC_READ;
 if (dwDesiredAccess&GENERIC_WRITE)
     result = dwDesiredAccess&GENERIC_READ ? O_RDWR : O_WRONLY;
 return result;
}

INTERN HANDLE WINAPI
K32_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD UNUSED(dwShareMode),
                LPSECURITY_ATTRIBUTES UNUSED(lpSecurityAttributes), DWORD dwCreationDisposition,
                DWORD dwFlagsAndAttributes, HANDLE UNUSED(hTemplateFile)) {
 int result; oflag_t oflags; mode_t mode = 0666;
 if (dwCreationDisposition >= COMPILER_LENOF(creation_disk_flags)) {
  SET_ERRNO(EINVAL);
  return INVALID_HANDLE_VALUE;
 }
 /* Apply creation disposition flags. */
 oflags  = K32_DesiredAccessToOflags(dwDesiredAccess);
 oflags |= creation_disk_flags[dwCreationDisposition];
 if (dwFlagsAndAttributes&FILE_FLAG_WRITE_THROUGH)      oflags |= O_DIRECT;
 if (dwFlagsAndAttributes&FILE_FLAG_DELETE_ON_CLOSE)    oflags |= O_TMPFILE;
 if (dwFlagsAndAttributes&FILE_FLAG_OPEN_REPARSE_POINT) oflags |= O_SYMLINK;
 if (dwFlagsAndAttributes&FILE_ATTRIBUTE_READONLY)      mode &= ~0222; /* Disable write-permissions. */

 result = dos_open(lpFileName,oflags,mode);
 if (result < 0) return INVALID_HANDLE_VALUE;

 if (!(dwFlagsAndAttributes&FILE_FLAG_BACKUP_SEMANTICS)) {
  struct stat64 info; /* Make sure this isn't a directly. */
  if (!fstat64(result,&info) && 
      S_ISDIR(info.st_mode)) {
   close(result);
   SET_ERRNO(EISDIR);
   return INVALID_HANDLE_VALUE;
  }
 }

 return FD_TO_HANDLE(result);
}
INTERN HANDLE WINAPI
K32_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
                DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
 HANDLE result; char *path;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return INVALID_HANDLE_VALUE; }
 path = uni_utf16to8m(lpFileName);
 if unlikely(!path) return INVALID_HANDLE_VALUE;
 result = K32_CreateFileA(path,dwDesiredAccess,dwShareMode,lpSecurityAttributes,
                          dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
 free(path);
 return result;
}

INTERN HANDLE WINAPI
K32_ReOpenFile(HANDLE hOriginalFile, DWORD dwDesiredAccess,
               DWORD UNUSED(dwShareMode), DWORD UNUSED(dwFlagsAndAttributes)) {
 int result; oflag_t oflags = K32_DesiredAccessToOflags(dwDesiredAccess);
 if (!HANDLE_IS_FD(hOriginalFile)) { SET_ERRNO(EBADF); return INVALID_HANDLE_VALUE; }
 result = dup(HANDLE_TO_FD(hOriginalFile));
 if (fcntl(result,F_SETFL,oflags) < 0) { close(result); return INVALID_HANDLE_VALUE; }
 return FD_TO_HANDLE(result);
}


INTERN WINBOOL WINAPI
K32_SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes) {
 struct stat64 info; mode_t new_mode;
 if (dos_stat64(lpFileName,&info)) return FALSE;
 new_mode = info.st_mode;
 if (dwFileAttributes&FILE_ATTRIBUTE_READONLY)
      new_mode &= ~0222; /* Disable write access */
 else new_mode |=  0222; /* Enable write access */
 if (new_mode == info.st_mode) return TRUE;
 return !dos_chmod(lpFileName,new_mode);
}

INTERN DWORD WINAPI
K32_GetFileAttributesA(LPCSTR lpFileName) {
 struct stat64 info;
 if (dos_stat64(lpFileName,&info))
     return INVALID_FILE_ATTRIBUTES;
 return K32_GetFileAttributesFromUnixMode(info.st_mode);
}
INTERN WINBOOL WINAPI
K32_SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes) {
 WINBOOL result; char *path;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpFileName);
 if unlikely(!path) return FALSE;
 result = K32_SetFileAttributesA(path,dwFileAttributes);
 free(path);
 return result;
}
INTERN DWORD WINAPI
K32_GetFileAttributesW(LPCWSTR lpFileName) {
 DWORD result; char *path;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return INVALID_FILE_ATTRIBUTES; }
 path = uni_utf16to8m(lpFileName);
 if unlikely(!path) return INVALID_FILE_ATTRIBUTES;
 result = K32_GetFileAttributesA(path);
 free(path);
 return result;
}
INTERN WINBOOL WINAPI K32_DeleteFileA(LPCSTR lpFileName) { return !dos_unlink(lpFileName); }
INTERN WINBOOL WINAPI K32_DeleteFileW(LPCWSTR lpFileName) {
 WINBOOL result; char *path;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpFileName);
 if unlikely(!path) return FALSE;
 result = K32_DeleteFileA(path);
 free(path);
 return result;
}

INTERN WINBOOL WINAPI
K32_GetFileAttributesExA(LPCSTR lpFileName,
                         GET_FILEEX_INFO_LEVELS fInfoLevelId,
                         LPVOID lpFileInformation) {
 struct stat64 info;
 LPWIN32_FILE_ATTRIBUTE_DATA out;
 if unlikely(!lpFileInformation ||
             fInfoLevelId != GetFileExInfoStandard) {
  SET_ERRNO(EINVAL);
  return FALSE;
 }
 if (dos_stat64(lpFileName,&info)) return FALSE;
 out = (LPWIN32_FILE_ATTRIBUTE_DATA)lpFileInformation;
 out->dwFileAttributes = K32_GetFileAttributesFromUnixMode(info.st_mode);
 out->ftCreationTime   = K32_TimespecToFiletime(info.st_ctim64);
 out->ftLastAccessTime = K32_TimespecToFiletime(info.st_atim64);
 out->ftLastWriteTime  = K32_TimespecToFiletime(info.st_mtim64);
 out->nFileSizeHigh    = (u32)((u64)info.st_size64 >> 32);
 out->nFileSizeLow     = info.st_size32;
 return TRUE;
}

INTERN DWORD WINAPI
K32_GetCompressedFileSizeA(LPCSTR lpFileName, LPDWORD lpFileSizeHigh) {
 struct stat64 info;
 if (dos_stat64(lpFileName,&info)) return INVALID_FILE_SIZE;
 /* Just return the uncompressed size. */
 if (lpFileSizeHigh) *lpFileSizeHigh = (u32)(info.st_size64 >> 32);
 return info.st_size32;
}

INTERN WINBOOL WINAPI
K32_GetFileAttributesExW(LPCWSTR lpFileName,
                         GET_FILEEX_INFO_LEVELS fInfoLevelId,
                         LPVOID lpFileInformation) {
 WINBOOL result; char *path;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpFileName);
 if unlikely(!path) return FALSE;
 result = K32_GetFileAttributesExA(path,fInfoLevelId,lpFileInformation);
 free(path);
 return result;
}
INTERN DWORD WINAPI
K32_GetCompressedFileSizeW(LPCWSTR lpFileName, LPDWORD lpFileSizeHigh) {
 DWORD result; char *path;
 if unlikely(!lpFileName) { SET_ERRNO(EINVAL); return FALSE; }
 path = uni_utf16to8m(lpFileName);
 if unlikely(!path) return FALSE;
 result = K32_GetCompressedFileSizeA(path,lpFileSizeHigh);
 free(path);
 return result;
}





/* Symbolic/Hard link API. */
INTERN BOOLEAN WINAPI
K32_CreateSymbolicLinkA(LPSTR lpSymLinkFileName,
                        LPSTR lpTargetFileName,
                        DWORD UNUSED(dwFlags)) {
 return !symlink(lpTargetFileName,lpSymLinkFileName);
}
INTERN BOOLEAN WINAPI
K32_CreateSymbolicLinkW(LPWSTR lpSymLinkFileName,
                        LPWSTR lpTargetFileName,
                        DWORD dwFlags) {
 char *link_name,*link_text; BOOLEAN result = FALSE;
 if (!lpSymLinkFileName || !lpTargetFileName) { SET_ERRNO(EINVAL); goto end; }
 link_name = uni_utf16to8m(lpSymLinkFileName);
 if unlikely(!link_name) goto end;
 link_text = uni_utf16to8m(lpTargetFileName);
 if unlikely(!link_text) goto end2;
 result = K32_CreateSymbolicLinkA(link_name,link_text,dwFlags);
 free(link_text);
end2: free(link_name);
end: return result;
}
INTERN WINBOOL WINAPI
K32_CreateHardLinkA(LPCSTR lpFileName, LPCSTR lpExistingFileName,
                    LPSECURITY_ATTRIBUTES UNUSED(lpSecurityAttributes)) {
 return !link(lpExistingFileName,lpFileName);
}
INTERN WINBOOL WINAPI
K32_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName,
                    LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
 char *link_name,*link_target; BOOLEAN result = FALSE;
 if (!lpFileName || !lpExistingFileName) { SET_ERRNO(EINVAL); goto end; }
 link_name = uni_utf16to8m(lpFileName);
 if unlikely(!link_name) goto end;
 link_target = uni_utf16to8m(lpExistingFileName);
 if unlikely(!link_target) goto end2;
 result = K32_CreateHardLinkA(link_name,link_target,lpSecurityAttributes);
 free(link_target);
end2: free(link_name);
end: return result;
}



INTERN char *WINAPI K32_MallocFReadLink(int fd) {
 ssize_t reqsize; size_t bufsize = 256;
 char *new_buf,*result = (char *)malloc(bufsize);
 if (!result) return NULL;
 reqsize = freadlink(fd,result,bufsize);
 if (reqsize < 0) { SET_ERRNO((errno_t)-reqsize); return NULL; }
 if ((size_t)reqsize > bufsize) {
  /* Allocate a new buffer dynamically. */
  do {
   bufsize = (size_t)reqsize;
   new_buf = (char *)realloc(result,bufsize);
   if unlikely(!new_buf) { free(result); return NULL; }
   result = new_buf;
  } while ((reqsize = freadlink(fd,result,bufsize),
            reqsize >= 0 && (size_t)reqsize != bufsize));
  if (reqsize < 0) { free(result); return NULL; }
 } else if ((size_t)reqsize < bufsize) {
  new_buf = (char *)realloc(result,(size_t)reqsize);
  if (new_buf) result = new_buf;
 }
 return result;
}


/* File Copy/Move APIs. */
INTERN WINBOOL WINAPI
K32_CopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName,
                LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData,
                LPBOOL pbCancel, DWORD dwCopyFlags) {
 char *wpos,*buffer = NULL; WINBOOL result = FALSE; DWORD status;
 ssize_t read_part,write_part; size_t write_missing;
 LARGE_INTEGER transferred = { .QuadPart = 0 };
 LARGE_INTEGER buffer_size,part_size = { .QuadPart = 0 };
 struct stat64 source_info; int sfd = -1,dfd = -1;
 int soflag = O_RDONLY,doflag = O_WRONLY|O_CREAT|O_TRUNC;
 if (dwCopyFlags&COPY_FILE_COPY_SYMLINK)          soflag |= O_SYMLINK;
 if (dwCopyFlags&COPY_FILE_FAIL_IF_EXISTS)        doflag |= O_EXCL,doflag &= ~O_TRUNC;
 if (dwCopyFlags&COPY_FILE_NO_BUFFERING)          soflag |= O_DIRECT,doflag |= O_DIRECT;
 if (dwCopyFlags&COPY_FILE_OPEN_SOURCE_FOR_WRITE) soflag |= O_RDWR;
 sfd = dos_open(lpExistingFileName,soflag);
 if (sfd < 0 || fstat64(sfd,&source_info)) goto end;
 if (dwCopyFlags&COPY_FILE_COPY_SYMLINK &&
     S_ISLNK(source_info.st_mode)) {
  /* Copy a symbolic link. */
  char *target = K32_MallocFReadLink(sfd);
  if unlikely(!target) goto end;
  result = !symlink(target,lpNewFileName);
  free(target);
  goto end;
 }

 /* Open a stream to the target. */
 dfd = dos_open(lpNewFileName,doflag);
 if (dfd < 0) goto end;
 if (source_info.st_blksize < 128)
     source_info.st_blksize = 128;
 buffer = (char *)malloc(source_info.st_blksize);
 buffer_size.QuadPart = source_info.st_blksize;
 if unlikely(!buffer) goto end;
 if (pbCancel && *pbCancel) goto err_cancel_delete;
 if (lpProgressRoutine) {
  status = (*lpProgressRoutine)(*(LARGE_INTEGER *)&source_info.st_size64,
                                  transferred,buffer_size,part_size,
                                 (DWORD)sfd,CALLBACK_STREAM_SWITCH,
                                  FD_TO_HANDLE(sfd),FD_TO_HANDLE(dfd),lpData);
  if (status == PROGRESS_CANCEL) goto err_cancel_delete;
  if (status == PROGRESS_STOP) goto err_cancel;
  if (status == PROGRESS_QUIET) lpProgressRoutine = NULL;
 }

 /* Transfer data to the target stream. */
 for (;;) {
  if (pbCancel && *pbCancel) goto err_cancel_delete;
  ERROR_REQUEST_ABORTED;
  read_part = read(sfd,buffer,source_info.st_blksize);
  if (!read_part) break; /* That was the last part. - we're done! */
  if (read_part < 0) goto end;
  assert((size_t)read_part <= source_info.st_blksize);
  part_size.QuadPart = (LONGLONG)read_part;

  /* Write everything we've just read. */
  wpos = buffer,write_missing = (size_t)read_part;
  while (write_missing) {
   write_part = write(dfd,wpos,write_missing);
   if (write_part < 0) goto end;
   if (!write_part) { SET_ERRNO(ENOSPC); goto end; }
   assert((size_t)write_part <= write_missing);
   write_missing -= (size_t)write_part;
   wpos          += (size_t)write_part;
  }

  /* Track the total amount of data transferred. */
  transferred.QuadPart += (LONGLONG)read_part;

  /* Execute the user-given progress callback. */
  if (lpProgressRoutine) {
   status = (*lpProgressRoutine)(*(LARGE_INTEGER *)&source_info.st_size64,
                                   transferred,buffer_size,part_size,
                                  (DWORD)sfd,CALLBACK_CHUNK_FINISHED,
                                   FD_TO_HANDLE(sfd),FD_TO_HANDLE(dfd),lpData);
   if (status == PROGRESS_CANCEL) goto err_cancel_delete;
   if (status == PROGRESS_STOP) goto err_cancel;
   if (status == PROGRESS_QUIET) lpProgressRoutine = NULL;
  }
 }
 /* Success! */
 result = TRUE;
end:
 free(buffer);
 if (dfd >= 0) close(dfd);
 if (sfd >= 0) close(sfd);
 return result;
err_cancel_delete:
 if (dfd >= 0) close(dfd),dfd = -1;
 dos_unlink(lpNewFileName);
err_cancel:
 SET_NT_ERRNO(ERROR_REQUEST_ABORTED);
 goto end;
}

INTERN WINBOOL WINAPI
K32_MoveFileWithProgressA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName,
                          LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData,
                          DWORD dwFlags) {
 bool did_delete_target = false;
again:
 if (!dos_rename(lpExistingFileName,lpNewFileName)) return TRUE;
 switch (GET_ERRNO()) {

 {
  DWORD copy_flags;
 case ENXIO:
  /* Can't move between different superblocks. */
  if (!(dwFlags&MOVEFILE_COPY_ALLOWED)) return FALSE;
  copy_flags = COPY_FILE_COPY_SYMLINK;
  if (!(dwFlags&MOVEFILE_REPLACE_EXISTING)) copy_flags |= COPY_FILE_FAIL_IF_EXISTS;
  if (dwFlags&MOVEFILE_WRITE_THROUGH) copy_flags |= COPY_FILE_NO_BUFFERING;
  if (!K32_CopyFileExA(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,NULL,copy_flags)) return FALSE;
  if (dos_unlink(lpExistingFileName)) { dos_unlink(lpNewFileName); return FALSE; }
  return TRUE;
 } break;

 case EEXIST:
  /* Target file already exists. */
  if (!(dwFlags&MOVEFILE_REPLACE_EXISTING) &&
      !did_delete_target) return FALSE;
  if (dos_unlinkat(AT_FDCWD,lpNewFileName,AT_REMOVEDIR|AT_REMOVEREG))
      return FALSE;
  did_delete_target = true;
  goto again;

 default: break;
 }
 return FALSE;
}

INTERN WINBOOL WINAPI
K32_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,
                LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData,
                LPBOOL pbCancel, DWORD dwCopyFlags) {
 WINBOOL result = FALSE; char *oldname,*newname;
 if (!lpExistingFileName || !lpNewFileName) { SET_ERRNO(EINVAL); goto end; }
 if ((oldname = uni_utf16to8m(lpExistingFileName)) == NULL) goto end;
 if ((newname = uni_utf16to8m(lpNewFileName)) == NULL) goto end2;
 result = K32_CopyFileExA(oldname,newname,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
 free(newname);
end2: free(oldname);
end: return result;
}
INTERN WINBOOL WINAPI
K32_MoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,
                          LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData,
                          DWORD dwFlags) {
 WINBOOL result = FALSE; char *oldname,*newname;
 if (!lpExistingFileName || !lpNewFileName) { SET_ERRNO(EINVAL); goto end; }
 if ((oldname = uni_utf16to8m(lpExistingFileName)) == NULL) goto end;
 if ((newname = uni_utf16to8m(lpNewFileName)) == NULL) goto end2;
 result = K32_MoveFileWithProgressA(oldname,newname,lpProgressRoutine,lpData,dwFlags);
 free(newname);
end2: free(oldname);
end: return result;
}
INTERN WINBOOL WINAPI K32_CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, WINBOOL bFailIfExists) { return K32_CopyFileExA(lpExistingFileName,lpNewFileName,NULL,NULL,NULL,bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0); }
INTERN WINBOOL WINAPI K32_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, WINBOOL bFailIfExists) { return K32_CopyFileExW(lpExistingFileName,lpNewFileName,NULL,NULL,NULL,bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0); }
INTERN WINBOOL WINAPI K32_MoveFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags) { return K32_MoveFileWithProgressA(lpExistingFileName,lpNewFileName,NULL,NULL,dwFlags); }
INTERN WINBOOL WINAPI K32_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags) { return K32_MoveFileWithProgressW(lpExistingFileName,lpNewFileName,NULL,NULL,dwFlags); }
INTERN WINBOOL WINAPI K32_MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) { return K32_MoveFileExA(lpExistingFileName,lpNewFileName,0); }
INTERN WINBOOL WINAPI K32_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) { return K32_MoveFileExW(lpExistingFileName,lpNewFileName,0); }







/* File-api Codepage API. */
DEFINE_PUBLIC_ALIAS(SetFileApisToOEM,K32_SetFileApisToOEM);
DEFINE_PUBLIC_ALIAS(SetFileApisToANSI,K32_SetFileApisToANSI);
DEFINE_PUBLIC_ALIAS(AreFileApisANSI,K32_AreFileApisANSI);

/* 16-bit compatibility File API.
 * HINT: These directly map to the underlying file descriptors! */
DEFINE_PUBLIC_ALIAS(OpenFile,K32_OpenFile);
DEFINE_PUBLIC_ALIAS(_lopen,K32_lopen);
DEFINE_PUBLIC_ALIAS(_lcreat,K32_lcreat);
DEFINE_PUBLIC_ALIAS(_lread,K32_lread);
DEFINE_PUBLIC_ALIAS(_lwrite,K32_lwrite);
DEFINE_PUBLIC_ALIAS(_hread,K32_hread);
DEFINE_PUBLIC_ALIAS(_hwrite,K32_hwrite);
DEFINE_PUBLIC_ALIAS(_lclose,K32_lclose);
DEFINE_PUBLIC_ALIAS(_llseek,K32_llseek);

/* Low-level Handle/File API. */
DEFINE_PUBLIC_ALIAS(CloseHandle,K32_CloseHandle);
DEFINE_PUBLIC_ALIAS(DuplicateHandle,K32_DuplicateHandle);
DEFINE_PUBLIC_ALIAS(GetHandleInformation,K32_GetHandleInformation);
DEFINE_PUBLIC_ALIAS(SetHandleInformation,K32_SetHandleInformation);
DEFINE_PUBLIC_ALIAS(GetFileTime,K32_GetFileTime);
DEFINE_PUBLIC_ALIAS(SetFileTime,K32_SetFileTime);
DEFINE_PUBLIC_ALIAS(SetFilePointer,K32_SetFilePointer);
DEFINE_PUBLIC_ALIAS(SetFilePointerEx,K32_SetFilePointerEx);
DEFINE_PUBLIC_ALIAS(SetEndOfFile,K32_SetEndOfFile);
DEFINE_PUBLIC_ALIAS(FlushFileBuffers,K32_FlushFileBuffers);
DEFINE_PUBLIC_ALIAS(GetStdHandle,K32_GetStdHandle);
DEFINE_PUBLIC_ALIAS(SetStdHandle,K32_SetStdHandle);
DEFINE_PUBLIC_ALIAS(WriteFile,K32_WriteFile);
DEFINE_PUBLIC_ALIAS(ReadFile,K32_ReadFile);
DEFINE_PUBLIC_ALIAS(GetFileInformationByHandle,K32_GetFileInformationByHandle);
DEFINE_PUBLIC_ALIAS(GetFileType,K32_GetFileType);
DEFINE_PUBLIC_ALIAS(GetFileSize,K32_GetFileSize);
DEFINE_PUBLIC_ALIAS(GetFileSizeEx,K32_GetFileSizeEx);
DEFINE_PUBLIC_ALIAS(LockFile,K32_LockFile);
DEFINE_PUBLIC_ALIAS(UnlockFile,K32_UnlockFile);
DEFINE_PUBLIC_ALIAS(LockFileEx,K32_LockFileEx);

/* Extended File APIs. */
DEFINE_PUBLIC_ALIAS(ReadFileEx,K32_ReadFileEx);
DEFINE_PUBLIC_ALIAS(WriteFileEx,K32_WriteFileEx);
DEFINE_PUBLIC_ALIAS(BackupRead,K32_BackupRead);
DEFINE_PUBLIC_ALIAS(BackupSeek,K32_BackupSeek);
DEFINE_PUBLIC_ALIAS(BackupWrite,K32_BackupWrite);
DEFINE_PUBLIC_ALIAS(ReadFileScatter,K32_ReadFileScatter);
DEFINE_PUBLIC_ALIAS(WriteFileGather,K32_WriteFileGather);

/* Directory scanning APIs. */
DEFINE_PUBLIC_ALIAS(FindFirstFileExA,K32_FindFirstFileExA);
DEFINE_PUBLIC_ALIAS(FindFirstFileExW,K32_FindFirstFileExW);
DEFINE_PUBLIC_ALIAS(FindFirstFileA,K32_FindFirstFileA);
DEFINE_PUBLIC_ALIAS(FindFirstFileW,K32_FindFirstFileW);
DEFINE_PUBLIC_ALIAS(FindNextFileA,K32_FindNextFileA);
DEFINE_PUBLIC_ALIAS(FindNextFileW,K32_FindNextFileW);
DEFINE_PUBLIC_ALIAS(FindClose,K32_FindClose);

/* Stuff that we're not implementing. */
DEFINE_PUBLIC_ALIAS(DeviceIoControl,K32_DeviceIoControl);
DEFINE_PUBLIC_ALIAS(RequestDeviceWakeup,K32_RequestDeviceWakeup);
DEFINE_PUBLIC_ALIAS(CancelDeviceWakeupRequest,K32_CancelDeviceWakeupRequest);
DEFINE_PUBLIC_ALIAS(GetDevicePowerState,K32_GetDevicePowerState);
DEFINE_PUBLIC_ALIAS(SetMessageWaitingIndicator,K32_SetMessageWaitingIndicator);
DEFINE_PUBLIC_ALIAS(SetFileValidData,K32_SetFileValidData);
DEFINE_PUBLIC_ALIAS(SetFileShortNameA,K32_SetFileShortNameA);
DEFINE_PUBLIC_ALIAS(SetFileShortNameW,K32_SetFileShortNameW);

/* PWD access. */
DEFINE_PUBLIC_ALIAS(SetCurrentDirectoryA,K32_SetCurrentDirectoryA);
DEFINE_PUBLIC_ALIAS(SetCurrentDirectoryW,K32_SetCurrentDirectoryW);
DEFINE_PUBLIC_ALIAS(GetCurrentDirectoryA,K32_GetCurrentDirectoryA);
DEFINE_PUBLIC_ALIAS(GetCurrentDirectoryW,K32_GetCurrentDirectoryW);

/* Disk-free access. */
DEFINE_PUBLIC_ALIAS(GetDiskFreeSpaceA,K32_GetDiskFreeSpaceA);
DEFINE_PUBLIC_ALIAS(GetDiskFreeSpaceW,K32_GetDiskFreeSpaceW);
DEFINE_PUBLIC_ALIAS(GetDiskFreeSpaceExA,K32_GetDiskFreeSpaceExA);
DEFINE_PUBLIC_ALIAS(GetDiskFreeSpaceExW,K32_GetDiskFreeSpaceExW);

/* Directory create/delete API. */
DEFINE_PUBLIC_ALIAS(CreateDirectoryA,K32_CreateDirectoryA);
DEFINE_PUBLIC_ALIAS(CreateDirectoryW,K32_CreateDirectoryW);
DEFINE_PUBLIC_ALIAS(CreateDirectoryExA,K32_CreateDirectoryExA);
DEFINE_PUBLIC_ALIAS(CreateDirectoryExW,K32_CreateDirectoryExW);
DEFINE_PUBLIC_ALIAS(RemoveDirectoryA,K32_RemoveDirectoryA);
DEFINE_PUBLIC_ALIAS(RemoveDirectoryW,K32_RemoveDirectoryW);

/* File creation/attribute API. */
DEFINE_PUBLIC_ALIAS(CreateFileA,K32_CreateFileA);
DEFINE_PUBLIC_ALIAS(CreateFileW,K32_CreateFileW);
DEFINE_PUBLIC_ALIAS(ReOpenFile,K32_ReOpenFile);
DEFINE_PUBLIC_ALIAS(SetFileAttributesA,K32_SetFileAttributesA);
DEFINE_PUBLIC_ALIAS(SetFileAttributesW,K32_SetFileAttributesW);
DEFINE_PUBLIC_ALIAS(GetFileAttributesA,K32_GetFileAttributesA);
DEFINE_PUBLIC_ALIAS(GetFileAttributesW,K32_GetFileAttributesW);
DEFINE_PUBLIC_ALIAS(DeleteFileA,K32_DeleteFileA);
DEFINE_PUBLIC_ALIAS(DeleteFileW,K32_DeleteFileW);
DEFINE_PUBLIC_ALIAS(GetFileAttributesExA,K32_GetFileAttributesExA);
DEFINE_PUBLIC_ALIAS(GetFileAttributesExW,K32_GetFileAttributesExW);
DEFINE_PUBLIC_ALIAS(GetCompressedFileSizeA,K32_GetCompressedFileSizeA);
DEFINE_PUBLIC_ALIAS(GetCompressedFileSizeW,K32_GetCompressedFileSizeW);

/* Symbolic/Hard link API. */
DEFINE_PUBLIC_ALIAS(CreateSymbolicLinkA,K32_CreateSymbolicLinkA);
DEFINE_PUBLIC_ALIAS(CreateSymbolicLinkW,K32_CreateSymbolicLinkW);
DEFINE_PUBLIC_ALIAS(CreateHardLinkA,K32_CreateHardLinkA);
DEFINE_PUBLIC_ALIAS(CreateHardLinkW,K32_CreateHardLinkW);

/* File Copy/Move APIs. */
DEFINE_PUBLIC_ALIAS(CopyFileA,K32_CopyFileA);
DEFINE_PUBLIC_ALIAS(CopyFileW,K32_CopyFileW);
DEFINE_PUBLIC_ALIAS(CopyFileExA,K32_CopyFileExA);
DEFINE_PUBLIC_ALIAS(CopyFileExW,K32_CopyFileExW);
DEFINE_PUBLIC_ALIAS(MoveFileA,K32_MoveFileA);
DEFINE_PUBLIC_ALIAS(MoveFileW,K32_MoveFileW);
DEFINE_PUBLIC_ALIAS(MoveFileExA,K32_MoveFileExA);
DEFINE_PUBLIC_ALIAS(MoveFileExW,K32_MoveFileExW);
DEFINE_PUBLIC_ALIAS(MoveFileWithProgressA,K32_MoveFileWithProgressA);
DEFINE_PUBLIC_ALIAS(MoveFileWithProgressW,K32_MoveFileWithProgressW);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_FILE_C */
