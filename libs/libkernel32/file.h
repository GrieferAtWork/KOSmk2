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
#ifndef GUARD_LIBS_LIBKERNEL32_FILE_H
#define GUARD_LIBS_LIBKERNEL32_FILE_H 1

#include "k32.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN


#define HANDLE_TYPEMASK  0xf000
/* NOTE: Don't use types 0xf and 0x0, because they
 *       overlap with INVALID_HANDLE_VALUE and NULL. */
#define HANDLE_TYPE_PID  0xd000
#define HANDLE_TYPE_FD   0xe000
#define HANDLE_TYPE(x)   ((uintptr_t)(x)&HANDLE_TYPEMASK)

#define HANDLE_IS_FD(x)   (HANDLE_TYPE(x) == HANDLE_TYPE_FD)
#define HANDLE_IS_PID(x)  (HANDLE_TYPE(x) == HANDLE_TYPE_PID)

#define HANDLE_TO_FD(x)     ((int)((uintptr_t)(x)&~HANDLE_TYPEMASK))
#define HANDLE_TO_PID(x)    ((int)((uintptr_t)(x)&~HANDLE_TYPEMASK))
#define FD_TO_HANDLE(x)  ((HANDLE)((uintptr_t)(x)|HANDLE_TYPE_FD))
#define PID_TO_HANDLE(x) ((HANDLE)((uintptr_t)(x)|HANDLE_TYPE_PID))

INTDEF DWORD   WINAPI K32_GetFileAttributesFromUnixMode(mode_t val);
INTDEF oflag_t WINAPI K32_DesiredAccessToOflags(DWORD dwDesiredAccess);
INTDEF char   *WINAPI K32_MallocFReadLink(int fd);

/* File-api Codepage API. */
INTDEF void    WINAPI K32_SetFileApisToOEM(void);
INTDEF void    WINAPI K32_SetFileApisToANSI(void);
INTDEF WINBOOL WINAPI K32_AreFileApisANSI(void);

/* 16-bit compatibility File API.
 * HINT: These directly map to the underlying file descriptors! */
INTDEF HFILE WINAPI K32_OpenFile(LPCSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT uStyle);
INTDEF HFILE WINAPI K32_lopen(LPCSTR lpPathName, int iReadWrite);
INTDEF HFILE WINAPI K32_lcreat(LPCSTR lpPathName, int iAttribute);
INTDEF UINT  WINAPI K32_lread(HFILE hFile, LPVOID lpBuffer, UINT uBytes);
INTDEF UINT  WINAPI K32_lwrite(HFILE hFile, LPCCH lpBuffer, UINT uBytes);
INTDEF LONG  WINAPI K32_hread(HFILE hFile, LPVOID lpBuffer, LONG lBytes);
INTDEF LONG  WINAPI K32_hwrite(HFILE hFile, LPCCH lpBuffer, LONG lBytes);
INTDEF HFILE WINAPI K32_lclose(HFILE hFile);
INTDEF LONG  WINAPI K32_llseek(HFILE hFile, LONG lOffset, int iOrigin);

/* Low-level Handle/File API. */
INTDEF WINBOOL WINAPI K32_CloseHandle(HANDLE hObject);
INTDEF WINBOOL WINAPI K32_DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, WINBOOL bInheritHandle, DWORD dwOptions);
INTDEF WINBOOL WINAPI K32_GetHandleInformation(HANDLE hObject, LPDWORD lpdwFlags);
INTDEF WINBOOL WINAPI K32_SetHandleInformation(HANDLE hObject, DWORD dwMask, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime);
INTDEF WINBOOL WINAPI K32_SetFileTime(HANDLE hFile, CONST FILETIME *lpCreationTime, CONST FILETIME *lpLastAccessTime, CONST FILETIME *lpLastWriteTime);
INTDEF DWORD   WINAPI K32_SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
INTDEF WINBOOL WINAPI K32_SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
INTDEF WINBOOL WINAPI K32_SetEndOfFile(HANDLE hFile);
INTDEF WINBOOL WINAPI K32_FlushFileBuffers(HANDLE hFile);
INTDEF HANDLE  WINAPI K32_GetStdHandle(DWORD nStdHandle);
INTDEF WINBOOL WINAPI K32_SetStdHandle(DWORD nStdHandle, HANDLE hHandle);
INTDEF WINBOOL WINAPI K32_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
INTDEF WINBOOL WINAPI K32_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
INTDEF WINBOOL WINAPI K32_GetFileInformationByHandle(HANDLE hFile, LPBY_HANDLE_FILE_INFORMATION lpFileInformation);
INTDEF DWORD   WINAPI K32_GetFileType(HANDLE hFile);
INTDEF DWORD   WINAPI K32_GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
INTDEF WINBOOL WINAPI K32_GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);
INTDEF WINBOOL WINAPI K32_LockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh, DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh);
INTDEF WINBOOL WINAPI K32_UnlockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh, DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh);
INTDEF WINBOOL WINAPI K32_LockFileEx(HANDLE hFile, DWORD dwFlags, DWORD dwReserved, DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh, LPOVERLAPPED lpOverlapped);

/* Extended File APIs. */
INTDEF WINBOOL WINAPI K32_ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
INTDEF WINBOOL WINAPI K32_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
INTDEF WINBOOL WINAPI K32_BackupRead(HANDLE hFile, LPBYTE lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, WINBOOL bAbort, WINBOOL bProcessSecurity, LPVOID *lpContext);
INTDEF WINBOOL WINAPI K32_BackupSeek(HANDLE hFile, DWORD dwLowBytesToSeek, DWORD dwHighBytesToSeek, LPDWORD lpdwLowByteSeeked, LPDWORD lpdwHighByteSeeked, LPVOID *lpContext);
INTDEF WINBOOL WINAPI K32_BackupWrite(HANDLE hFile, LPBYTE lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, WINBOOL bAbort, WINBOOL bProcessSecurity, LPVOID *lpContext);
INTDEF WINBOOL WINAPI K32_ReadFileScatter(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToRead, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped);
INTDEF WINBOOL WINAPI K32_WriteFileGather(HANDLE hFile, FILE_SEGMENT_ELEMENT aSegmentArray[], DWORD nNumberOfBytesToWrite, LPDWORD lpReserved, LPOVERLAPPED lpOverlapped);

/* Directory scanning APIs. */
INTDEF HANDLE  WINAPI K32_FindFirstFileExA(LPCSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
INTDEF HANDLE  WINAPI K32_FindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
INTDEF HANDLE  WINAPI K32_FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
INTDEF HANDLE  WINAPI K32_FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
INTDEF WINBOOL WINAPI K32_FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
INTDEF WINBOOL WINAPI K32_FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
INTDEF WINBOOL WINAPI K32_FindClose(HANDLE hFindFile);

/* Missing... */
INTDEF WINBOOL WINAPI K32_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
INTDEF WINBOOL WINAPI K32_RequestDeviceWakeup(HANDLE hDevice);
INTDEF WINBOOL WINAPI K32_CancelDeviceWakeupRequest(HANDLE hDevice);
INTDEF WINBOOL WINAPI K32_GetDevicePowerState(HANDLE hDevice, WINBOOL *pfOn);
INTDEF WINBOOL WINAPI K32_SetMessageWaitingIndicator(HANDLE hMsgIndicator, ULONG ulMsgCount);
INTDEF WINBOOL WINAPI K32_SetFileValidData(HANDLE hFile, LONGLONG ValidDataLength);
INTDEF WINBOOL WINAPI K32_SetFileShortNameA(HANDLE hFile, LPCSTR lpShortName);
INTDEF WINBOOL WINAPI K32_SetFileShortNameW(HANDLE hFile, LPCWSTR lpShortName);

/* PWD access. */
INTDEF WINBOOL WINAPI K32_SetCurrentDirectoryA(LPCSTR lpPathName);
INTDEF WINBOOL WINAPI K32_SetCurrentDirectoryW(LPCWSTR lpPathName);
INTDEF DWORD   WINAPI K32_GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer);
INTDEF DWORD   WINAPI K32_GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer);

/* Disk-free access. */
INTDEF WINBOOL WINAPI K32_GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters);
INTDEF WINBOOL WINAPI K32_GetDiskFreeSpaceW(LPCWSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters);
INTDEF WINBOOL WINAPI K32_GetDiskFreeSpaceExA(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);
INTDEF WINBOOL WINAPI K32_GetDiskFreeSpaceExW(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);

/* Directory create/delete API. */
INTDEF WINBOOL WINAPI K32_CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
INTDEF WINBOOL WINAPI K32_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
INTDEF WINBOOL WINAPI K32_CreateDirectoryExA(LPCSTR lpTemplateDirectory, LPCSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
INTDEF WINBOOL WINAPI K32_CreateDirectoryExW(LPCWSTR lpTemplateDirectory, LPCWSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
INTDEF WINBOOL WINAPI K32_RemoveDirectoryA(LPCSTR lpPathName);
INTDEF WINBOOL WINAPI K32_RemoveDirectoryW(LPCWSTR lpPathName);

/* File creation/attribute API. */
INTDEF HANDLE  WINAPI K32_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
INTDEF HANDLE  WINAPI K32_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
INTDEF HANDLE  WINAPI K32_ReOpenFile(HANDLE hOriginalFile, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwFlagsAndAttributes);
INTDEF WINBOOL WINAPI K32_SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes);
INTDEF WINBOOL WINAPI K32_SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);
INTDEF DWORD   WINAPI K32_GetFileAttributesA(LPCSTR lpFileName);
INTDEF DWORD   WINAPI K32_GetFileAttributesW(LPCWSTR lpFileName);
INTDEF WINBOOL WINAPI K32_DeleteFileA(LPCSTR lpFileName);
INTDEF WINBOOL WINAPI K32_DeleteFileW(LPCWSTR lpFileName);
INTDEF WINBOOL WINAPI K32_GetFileAttributesExA(LPCSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
INTDEF WINBOOL WINAPI K32_GetFileAttributesExW(LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
INTDEF DWORD   WINAPI K32_GetCompressedFileSizeA(LPCSTR lpFileName, LPDWORD lpFileSizeHigh);
INTDEF DWORD   WINAPI K32_GetCompressedFileSizeW(LPCWSTR lpFileName, LPDWORD lpFileSizeHigh);

/* Symbolic/Hard link API. */
INTDEF BOOLEAN WINAPI K32_CreateSymbolicLinkA(LPSTR lpSymLinkFileName, LPSTR lpTargetFileName, DWORD dwFlags);
INTDEF BOOLEAN WINAPI K32_CreateSymbolicLinkW(LPWSTR lpSymLinkFileName, LPWSTR lpTargetFileName, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_CreateHardLinkA(LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
INTDEF WINBOOL WINAPI K32_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* File Copy/Move APIs. */
INTDEF WINBOOL WINAPI K32_CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, WINBOOL bFailIfExists);
INTDEF WINBOOL WINAPI K32_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, WINBOOL bFailIfExists);
INTDEF WINBOOL WINAPI K32_CopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
INTDEF WINBOOL WINAPI K32_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
INTDEF WINBOOL WINAPI K32_MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
INTDEF WINBOOL WINAPI K32_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
INTDEF WINBOOL WINAPI K32_MoveFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_MoveFileWithProgressA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
INTDEF WINBOOL WINAPI K32_MoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);


DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_FILE_H */
