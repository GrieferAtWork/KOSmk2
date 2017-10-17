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
#include <dirent.h>
#include <errno.h>
#include <unicode.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


DECL_BEGIN

INTERN WINBOOL WINAPI K32_CloseHandle(HANDLE hObject) {
 switch (HANDLE_TYPE(hObject)) {
 case HANDLE_TYPE_FD:
  return !close(HANDLE_TO_FD(hObject));
 case HANDLE_TYPE_PID:
  break;
 default:
  __set_errno(EBADF);
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
 if (!lpTargetHandle) { __set_errno(EINVAL); return FALSE; }
 if (!HANDLE_IS_PID(hSourceProcessHandle) ||
      hSourceProcessHandle != hTargetProcessHandle) {
  __set_errno(EACCES);
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
  __set_errno(EPERM);
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
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
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
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
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
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return INVALID_SET_FILE_POINTER; }
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
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
 result = lseek64(HANDLE_TO_FD(hFile),liDistanceToMove.QuadPart,(int)dwMoveMethod);
 if (result < 0) return FALSE;
 if (lpNewFilePointer) lpNewFilePointer->QuadPart = result;
 return TRUE;
}

INTERN WINBOOL WINAPI K32_SetEndOfFile(HANDLE hFile) {
 off64_t file_length;
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
 file_length = lseek64(HANDLE_TO_FD(hFile),0,SEEK_CUR);
 if (file_length < 0) return FALSE;
 return !ftruncate64(HANDLE_TO_FD(hFile),file_length);
}

INTERN WINBOOL WINAPI K32_FlushFileBuffers(HANDLE hFile) {
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
 return !fdatasync(HANDLE_TO_FD(hFile));
}

INTERN HANDLE WINAPI K32_GetStdHandle(DWORD nStdHandle) {
 switch (nStdHandle) {
 case STD_INPUT_HANDLE:  return FD_TO_HANDLE(STDIN_FILENO);
 case STD_OUTPUT_HANDLE: return FD_TO_HANDLE(STDOUT_FILENO);
 case STD_ERROR_HANDLE:  return FD_TO_HANDLE(STDERR_FILENO);
 default: break;
 }
 __set_errno(EINVAL);
 return INVALID_HANDLE_VALUE;
}

INTERN WINBOOL WINAPI
K32_SetStdHandle(DWORD nStdHandle, HANDLE hHandle) {
 HANDLE handno = K32_GetStdHandle(nStdHandle);
 if (!HANDLE_IS_FD(handno)) return FALSE;
 if (!HANDLE_IS_FD(hHandle)) { __set_errno(EBADF); return FALSE; }
 return !dup2(HANDLE_TO_FD(handno),HANDLE_TO_FD(hHandle));
}

INTERN WINBOOL WINAPI
K32_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
              LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
 ssize_t error;
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
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
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
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
 if (!lpFileInformation) { __set_errno(EINVAL); return FALSE; }
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
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
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FILE_TYPE_UNKNOWN; }
 if (fstat64(HANDLE_TO_FD(hFile),&info)) return FILE_TYPE_UNKNOWN;
 if (S_ISCHR(info.st_mode)) return FILE_TYPE_CHAR;
 if (S_ISFIFO(info.st_mode)) return FILE_TYPE_PIPE;
 if (S_ISSOCK(info.st_mode)) return FILE_TYPE_REMOTE;
 return FILE_TYPE_DISK;
}

INTERN WINBOOL WINAPI
K32_GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
 struct stat64 info;
 if (!lpFileSize) { __set_errno(EINVAL); return FALSE; }
 if (!HANDLE_IS_FD(hFile)) { __set_errno(EBADF); return FALSE; }
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




__LIBC DIR *LIBCCALL dos_opendir(char const *name) ASMNAME(".dos.opendir");
__LIBC DIR *LIBCCALL dos_opendirat(int dfd, char const *name) ASMNAME(".dos.opendirat");

struct find_query {
 DIR              *fq_stream; /*< [1..1][owned] Underlying libc-style directory stream. */
 char             *fq_query;  /*< [1..1][owned] Wildcard-style string matching */
 FINDEX_SEARCH_OPS fq_mode;   /*< [const] Search mode. */
};

#define FINDHANDLE_ISVALID(p) \
  ((p) && (p) != INVALID_HANDLE_VALUE)

INTDEF HANDLE WINAPI
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
  __set_errno(EINVAL);
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
INTDEF WINBOOL WINAPI
K32_FindNextFileA(HANDLE hFindFile,
                  LPWIN32_FIND_DATAA lpFindFileData) {
 struct find_query *query;
 struct dirent64 *ent;
 struct stat64 info;
 if (!FINDHANDLE_ISVALID(hFindFile)) { __set_errno(EINVAL); return FALSE; }
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

INTDEF WINBOOL WINAPI K32_FindClose(HANDLE hFindFile) {
 struct find_query *query;
 if (!FINDHANDLE_ISVALID(hFindFile)) { __set_errno(EINVAL); return FALSE; }
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
INTDEF HANDLE WINAPI
K32_FindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId,
                     LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp,
                     LPVOID lpSearchFilter, DWORD dwAdditionalFlags) {
 WIN32_FIND_DATAA temp; HANDLE result;
 char *query = uni_utf16to8m(lpFileName);
 if unlikely(!query) return INVALID_HANDLE_VALUE;
 result = K32_FindFirstFileExA(query,fInfoLevelId,&temp,fSearchOp,
                               lpSearchFilter,dwAdditionalFlags);
 free(query);
 if (result != INVALID_HANDLE_VALUE)
     K32_FinddataAToFinddataW((LPWIN32_FIND_DATAW)lpFindFileData,&temp);
 return result;
}
INTDEF WINBOOL WINAPI
K32_FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData) {
 WIN32_FIND_DATAA temp; WINBOOL result;
 result = K32_FindNextFileA(hFindFile,&temp);
 if (result) K32_FinddataAToFinddataW(lpFindFileData,&temp);
 return result;
}
INTDEF HANDLE WINAPI
K32_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
 return K32_FindFirstFileExA(lpFileName,FindExInfoStandard,lpFindFileData,
                             FindExSearchNameMatch,NULL,0);
}
INTDEF HANDLE WINAPI
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
 __set_errno(ENOSYS);
 return FALSE;
}
INTERN WINBOOL WINAPI K32_RequestDeviceWakeup(HANDLE hDevice) { __set_errno(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_CancelDeviceWakeupRequest(HANDLE hDevice) { __set_errno(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_GetDevicePowerState(HANDLE hDevice, WINBOOL *pfOn) { __set_errno(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_SetMessageWaitingIndicator(HANDLE hMsgIndicator, ULONG ulMsgCount) { __set_errno(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_SetFileValidData(HANDLE hFile, LONGLONG ValidDataLength) { __set_errno(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_SetFileShortNameA(HANDLE hFile, LPCSTR lpShortName) { __set_errno(ENOSYS); return FALSE; }
INTERN WINBOOL WINAPI K32_SetFileShortNameW(HANDLE hFile, LPCWSTR lpShortName) { __set_errno(ENOSYS); return FALSE; }


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

/* Directory scanning API. */
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

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_FILE_C */
