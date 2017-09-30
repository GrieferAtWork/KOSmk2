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
#ifndef GUARD_MODULES_NT_API_H
#define GUARD_MODULES_NT_API_H 1

#include <hybrid/compiler.h>
#include <winapi/windows.h>
#include <winapi/ntdef.h>
#include <winapi/winnt.h>
#include <sal.h>

DECL_BEGIN

/* Special system call executed as fallback for unimplemented IDs. */
INTDEF void NTAPI NtBadSysCall(VOID);
INTDEF void NTAPI NtSegFault(VOID);

/* Missing types... (From the faaaar future) */
#define PPORT_MESSAGE                   void *
#define PPORT_VIEW                      void *
#define PREMOTE_PORT_VIEW               void *
#define RTL_ATOM                        void *
#define PRTL_ATOM                       void *
#define ATOM_INFORMATION_CLASS          int
#define SYSDBG_COMMAND                  int
#define PBOOT_ENTRY                     void *
#define PEFI_DRIVER_ENTRY               void *
#define PFILE_PATH                      void *
#define PBOOT_OPTIONS                   void *
#define EVENT_INFORMATION_CLASS         int
#define MUTANT_INFORMATION_CLASS        int
#define KPROFILE_SOURCE                 int
#define SHUTDOWN_ACTION                 int
#define SYSTEM_INFORMATION_CLASS        int
#define TIMER_INFORMATION_CLASS         int
#define PTIMER_APC_ROUTINE              void *
#define PIO_STATUS_BLOCK                void *
#define PFILE_BASIC_INFORMATION         void *
#define PFILE_NETWORK_OPEN_INFORMATION  void *
#define IO_COMPLETION_INFORMATION_CLASS int
#define PIO_APC_ROUTINE                 void *
#define FILE_INFORMATION_CLASS          int
#define FS_INFORMATION_CLASS            int
#define PPLUGPLAY_EVENT_BLOCK           void *
#define PLUGPLAY_CONTROL_CLASS          int
#define PORT_INFORMATION_CLASS          int
#define SECTION_INFORMATION_CLASS       int
#define OBJECT_INFORMATION_CLASS        int
#define PCLIENT_ID                      void *
#define PROCESSINFOCLASS                void *
#define THREADINFOCLASS                 int
#define PKNORMAL_ROUTINE                void *
#define PINITIAL_TEB                    void *
#define APPHELPCACHESERVICECLASS        int
#define PAPPHELP_CACHE_SERVICE_LOOKUP   void *
#define SECTION_INHERIT                 int
#define MEMORY_INFORMATION_CLASS        int
#define KEY_INFORMATION_CLASS           int
#define KEY_VALUE_INFORMATION_CLASS     int
#define PKEY_VALUE_ENTRY                void *
#define KEY_SET_INFORMATION_CLASS       int
#define DEBUGOBJECTINFOCLASS            int
#define PDBGUI_WAIT_STATE_CHANGE        void *
#define SEMAPHORE_INFORMATION_CLASS     int

/* NOTE: Function prototypes are taken from ReactOS kernel source files,
 *       which are licensed under 'GNU GENERAL PUBLIC LICENSE'.
 *
 * NT API Functions are grouped by the files they appear inside within ReactOS,
 * with each group being commented with the filename inside which it, as well
 * as additional information about the original author can be found.
 *
 * >> I DID NOT REVERSE ENGINEER THESE PROTOTYPES!
 *    I'M MERELY RE-USING OPEN-SOURCE WORK OTHERS ALREADY DID. */

 
/* ReactOS: "nsoskrnl/config/ntapi.c" */
INTDEF NTSTATUS NTAPI NtCreateKey(OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG TitleIndex, IN PUNICODE_STRING Class OPTIONAL, IN ULONG CreateOptions, OUT PULONG Disposition OPTIONAL);
INTDEF NTSTATUS NTAPI NtOpenKey(OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtDeleteKey(IN HANDLE KeyHandle);
INTDEF NTSTATUS NTAPI NtEnumerateKey(IN HANDLE KeyHandle, IN ULONG Index, IN KEY_INFORMATION_CLASS KeyInformationClass, OUT PVOID KeyInformation, IN ULONG Length, OUT PULONG ResultLength);
INTDEF NTSTATUS NTAPI NtEnumerateValueKey(IN HANDLE KeyHandle, IN ULONG Index, IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength);
INTDEF NTSTATUS NTAPI NtQueryKey(IN HANDLE KeyHandle, IN KEY_INFORMATION_CLASS KeyInformationClass, OUT PVOID KeyInformation, IN ULONG Length, OUT PULONG ResultLength);
INTDEF NTSTATUS NTAPI NtQueryValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength);
INTDEF NTSTATUS NTAPI NtSetValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN ULONG TitleIndex, IN ULONG Type, IN PVOID Data, IN ULONG DataSize);
INTDEF NTSTATUS NTAPI NtDeleteValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName);
INTDEF NTSTATUS NTAPI NtFlushKey(IN HANDLE KeyHandle);
INTDEF NTSTATUS NTAPI NtLoadKey(IN POBJECT_ATTRIBUTES KeyObjectAttributes, IN POBJECT_ATTRIBUTES FileObjectAttributes);
INTDEF NTSTATUS NTAPI NtLoadKey2(IN POBJECT_ATTRIBUTES KeyObjectAttributes, IN POBJECT_ATTRIBUTES FileObjectAttributes, IN ULONG Flags);
INTDEF NTSTATUS NTAPI NtLoadKeyEx(IN POBJECT_ATTRIBUTES TargetKey, IN POBJECT_ATTRIBUTES SourceFile, IN ULONG Flags, IN HANDLE TrustClassKey);
INTDEF NTSTATUS NTAPI NtNotifyChangeKey(IN HANDLE KeyHandle, IN HANDLE Event, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG CompletionFilter, IN BOOLEAN WatchTree, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN Asynchronous);
INTDEF NTSTATUS NTAPI NtInitializeRegistry(IN USHORT Flag);
INTDEF NTSTATUS NTAPI NtCompactKeys(IN ULONG Count, IN PHANDLE KeyArray);
INTDEF NTSTATUS NTAPI NtCompressKey(IN HANDLE Key);
INTDEF NTSTATUS NTAPI NtLockProductActivationKeys(IN PULONG pPrivateVer, IN PULONG pSafeMode);
INTDEF NTSTATUS NTAPI NtLockRegistryKey(IN HANDLE KeyHandle);
INTDEF NTSTATUS NTAPI NtNotifyChangeMultipleKeys(IN HANDLE MasterKeyHandle, IN ULONG Count, IN POBJECT_ATTRIBUTES SlaveObjects, IN HANDLE Event, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG CompletionFilter, IN BOOLEAN WatchTree, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN Asynchronous);
INTDEF NTSTATUS NTAPI NtQueryMultipleValueKey(IN HANDLE KeyHandle, IN OUT PKEY_VALUE_ENTRY ValueList, IN ULONG NumberOfValues, OUT PVOID Buffer, IN OUT PULONG Length, OUT PULONG ReturnLength);
INTDEF NTSTATUS NTAPI NtQueryOpenSubKeys(IN POBJECT_ATTRIBUTES TargetKey, OUT PULONG HandleCount);
INTDEF NTSTATUS NTAPI NtQueryOpenSubKeysEx(IN POBJECT_ATTRIBUTES TargetKey, IN ULONG BufferLength, IN PVOID Buffer, IN PULONG RequiredSize);
INTDEF NTSTATUS NTAPI NtRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ReplacementName);
INTDEF NTSTATUS NTAPI NtReplaceKey(IN POBJECT_ATTRIBUTES ObjectAttributes, IN HANDLE Key, IN POBJECT_ATTRIBUTES ReplacedObjectAttributes);
INTDEF NTSTATUS NTAPI NtRestoreKey(IN HANDLE KeyHandle, IN HANDLE FileHandle, IN ULONG RestoreFlags);
INTDEF NTSTATUS NTAPI NtSaveKey(IN HANDLE KeyHandle, IN HANDLE FileHandle);
INTDEF NTSTATUS NTAPI NtSaveKeyEx(IN HANDLE KeyHandle, IN HANDLE FileHandle, IN ULONG Flags);
INTDEF NTSTATUS NTAPI NtSaveMergedKeys(IN HANDLE HighPrecedenceKeyHandle, IN HANDLE LowPrecedenceKeyHandle, IN HANDLE FileHandle);
INTDEF NTSTATUS NTAPI NtSetInformationKey(IN HANDLE KeyHandle, IN KEY_SET_INFORMATION_CLASS KeyInformationClass, IN PVOID KeyInformation, IN ULONG KeyInformationLength);
INTDEF NTSTATUS NTAPI NtUnloadKey(IN POBJECT_ATTRIBUTES KeyObjectAttributes);
INTDEF NTSTATUS NTAPI NtUnloadKey2(IN POBJECT_ATTRIBUTES TargetKey, IN ULONG Flags);
INTDEF NTSTATUS NTAPI NtUnloadKeyEx(IN POBJECT_ATTRIBUTES TargetKey, IN HANDLE Event);

/* ReactOS: "nsoskrnl/dbgk/dbgkobj.c" */
INTDEF NTSTATUS NTAPI NtCreateDebugObject(OUT PHANDLE DebugHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG Flags);
INTDEF NTSTATUS NTAPI NtDebugContinue(IN HANDLE DebugHandle, IN PCLIENT_ID AppClientId, IN NTSTATUS ContinueStatus);
INTDEF NTSTATUS NTAPI NtDebugActiveProcess(IN HANDLE ProcessHandle, IN HANDLE DebugHandle);
INTDEF NTSTATUS NTAPI NtRemoveProcessDebug(IN HANDLE ProcessHandle, IN HANDLE DebugHandle);
INTDEF NTSTATUS NTAPI NtSetInformationDebugObject(IN HANDLE DebugHandle, IN DEBUGOBJECTINFOCLASS DebugObjectInformationClass, IN PVOID DebugInformation, IN ULONG DebugInformationLength, OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtWaitForDebugEvent(IN HANDLE DebugHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL, OUT PDBGUI_WAIT_STATE_CHANGE StateChange);

/* ReactOS: "nsoskrnl/ex/atom.c" */
INTDEF NTSTATUS NTAPI NtAddAtom(IN PWSTR AtomName, IN ULONG AtomNameLength, OUT PRTL_ATOM Atom);
INTDEF NTSTATUS NTAPI NtDeleteAtom(IN RTL_ATOM Atom);
INTDEF NTSTATUS NTAPI NtFindAtom(IN PWSTR AtomName, IN ULONG AtomNameLength, OUT PRTL_ATOM Atom);
INTDEF NTSTATUS NTAPI NtQueryInformationAtom(RTL_ATOM Atom, ATOM_INFORMATION_CLASS AtomInformationClass, PVOID AtomInformation, ULONG AtomInformationLength, PULONG ReturnLength);

/* ReactOS: "nsoskrnl/ex/dbgctrl.c" */
INTDEF NTSTATUS NTAPI NtSystemDebugControl(SYSDBG_COMMAND ControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength, PULONG ReturnLength);

/* ReactOS: "nsoskrnl/ex/efi.c" */
INTDEF NTSTATUS NTAPI NtAddBootEntry(IN PBOOT_ENTRY Entry, IN ULONG Id);
INTDEF NTSTATUS NTAPI NtAddDriverEntry(IN PEFI_DRIVER_ENTRY Entry, IN ULONG Id);
INTDEF NTSTATUS NTAPI NtDeleteBootEntry(IN ULONG Id);
INTDEF NTSTATUS NTAPI NtDeleteDriverEntry(IN ULONG Id);
INTDEF NTSTATUS NTAPI NtEnumerateBootEntries(IN PVOID Buffer, IN PULONG BufferLength);
INTDEF NTSTATUS NTAPI NtEnumerateDriverEntries(IN PVOID Buffer, IN PULONG BufferLength);
INTDEF NTSTATUS NTAPI NtModifyBootEntry(IN PBOOT_ENTRY BootEntry);
INTDEF NTSTATUS NTAPI NtModifyDriverEntry(IN PEFI_DRIVER_ENTRY DriverEntry);
INTDEF NTSTATUS NTAPI NtQueryBootEntryOrder(IN PULONG Ids, IN PULONG Count);
INTDEF NTSTATUS NTAPI NtQueryDriverEntryOrder(IN PULONG Ids, IN PULONG Count);
INTDEF NTSTATUS NTAPI NtQueryBootOptions(IN PBOOT_OPTIONS BootOptions, IN PULONG BootOptionsLength);
INTDEF NTSTATUS NTAPI NtSetBootEntryOrder(IN PULONG Ids, IN PULONG Count);
INTDEF NTSTATUS NTAPI NtSetDriverEntryOrder(IN PULONG Ids, IN PULONG Count);
INTDEF NTSTATUS NTAPI NtSetBootOptions(IN PBOOT_OPTIONS BootOptions, IN ULONG FieldsToChange);
INTDEF NTSTATUS NTAPI NtTranslateFilePath(PFILE_PATH InputFilePath, ULONG OutputType, PFILE_PATH OutputFilePath, ULONG OutputFilePathLength);

/* ReactOS: "nsoskrnl/ex/event.c" */
INTDEF NTSTATUS NTAPI NtClearEvent(IN HANDLE EventHandle);
INTDEF NTSTATUS NTAPI NtCreateEvent(OUT PHANDLE EventHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL, IN EVENT_TYPE EventType, IN BOOLEAN InitialState);
INTDEF NTSTATUS NTAPI NtOpenEvent(OUT PHANDLE EventHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtPulseEvent(IN HANDLE EventHandle, OUT PLONG PreviousState OPTIONAL);
INTDEF NTSTATUS NTAPI NtQueryEvent(IN HANDLE EventHandle, IN EVENT_INFORMATION_CLASS EventInformationClass, OUT PVOID EventInformation, IN ULONG EventInformationLength, OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtResetEvent(IN HANDLE EventHandle, OUT PLONG PreviousState OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetEvent(IN HANDLE EventHandle, OUT PLONG PreviousState OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetEventBoostPriority(IN HANDLE EventHandle);

/* ReactOS: "nsoskrnl/ex/evtpair.c" */
INTDEF NTSTATUS NTAPI NtCreateEventPair(OUT PHANDLE EventPairHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtOpenEventPair(OUT PHANDLE EventPairHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtSetHighEventPair(IN HANDLE EventPairHandle);
INTDEF NTSTATUS NTAPI NtSetHighWaitLowEventPair(IN HANDLE EventPairHandle);
INTDEF NTSTATUS NTAPI NtSetLowEventPair(IN HANDLE EventPairHandle);
INTDEF NTSTATUS NTAPI NtSetLowWaitHighEventPair(IN HANDLE EventPairHandle);
INTDEF NTSTATUS NTAPI NtWaitLowEventPair(IN HANDLE EventPairHandle);
INTDEF NTSTATUS NTAPI NtWaitHighEventPair(IN HANDLE EventPairHandle);

/* ReactOS: "nsoskrnl/ex/harderr.c" */
INTDEF NTSTATUS NTAPI NtRaiseHardError(IN NTSTATUS ErrorStatus, IN ULONG NumberOfParameters, IN ULONG UnicodeStringParameterMask, IN PULONG_PTR Parameters, IN ULONG ValidResponseOptions, OUT PULONG Response);
INTDEF NTSTATUS NTAPI NtSetDefaultHardErrorPort(IN HANDLE PortHandle);

/* ReactOS: "nsoskrnl/ex/keyedevt.c" */
INTDEF NTSTATUS NTAPI NtCreateKeyedEvent(_Out_ PHANDLE OutHandle, _In_ ACCESS_MASK AccessMask, _In_ POBJECT_ATTRIBUTES ObjectAttributes, _In_ ULONG Flags);
INTDEF NTSTATUS NTAPI NtOpenKeyedEvent(_Out_ PHANDLE OutHandle, _In_ ACCESS_MASK AccessMask, _In_ POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtWaitForKeyedEvent(_In_opt_ HANDLE Handle, _In_ PVOID Key, _In_ BOOLEAN Alertable, _In_opt_ PLARGE_INTEGER Timeout);
INTDEF NTSTATUS NTAPI NtReleaseKeyedEvent(_In_opt_ HANDLE Handle, _In_ PVOID Key, _In_ BOOLEAN Alertable, _In_opt_ PLARGE_INTEGER Timeout);

/* ReactOS: "nsoskrnl/ex/locale.c" */
INTDEF NTSTATUS NTAPI NtQueryDefaultLocale(IN BOOLEAN UserProfile, OUT PLCID DefaultLocaleId);
INTDEF NTSTATUS NTAPI NtSetDefaultLocale(IN BOOLEAN UserProfile, IN LCID DefaultLocaleId);
INTDEF NTSTATUS NTAPI NtQueryInstallUILanguage(OUT LANGID* LanguageId);
INTDEF NTSTATUS NTAPI NtQueryDefaultUILanguage(OUT LANGID* LanguageId);
INTDEF NTSTATUS NTAPI NtSetDefaultUILanguage(IN LANGID LanguageId);

/* ReactOS: "nsoskrnl/ex/mutant.c" */
INTDEF NTSTATUS NTAPI NtCreateMutant(OUT PHANDLE MutantHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL, IN BOOLEAN InitialOwner);
INTDEF NTSTATUS NTAPI NtOpenMutant(OUT PHANDLE MutantHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQueryMutant(IN HANDLE MutantHandle, IN MUTANT_INFORMATION_CLASS MutantInformationClass, OUT PVOID MutantInformation, IN ULONG MutantInformationLength, OUT PULONG ResultLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtReleaseMutant(IN HANDLE MutantHandle, IN PLONG PreviousCount OPTIONAL);

/* ReactOS: "nsoskrnl/ex/profile.c" */
INTDEF NTSTATUS NTAPI NtCreateProfile(OUT PHANDLE ProfileHandle, IN HANDLE Process OPTIONAL, IN PVOID RangeBase, IN SIZE_T RangeSize, IN ULONG BucketSize, IN PVOID Buffer, IN ULONG BufferSize, IN KPROFILE_SOURCE ProfileSource, IN KAFFINITY Affinity);
INTDEF NTSTATUS NTAPI NtQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceCounter, OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL);
INTDEF NTSTATUS NTAPI NtStartProfile(IN HANDLE ProfileHandle);
INTDEF NTSTATUS NTAPI NtStopProfile(IN HANDLE ProfileHandle);
INTDEF NTSTATUS NTAPI NtQueryIntervalProfile(IN KPROFILE_SOURCE ProfileSource, OUT PULONG Interval);
INTDEF NTSTATUS NTAPI NtSetIntervalProfile(IN ULONG Interval, IN KPROFILE_SOURCE Source);

/* ReactOS: "nsoskrnl/ex/sem.c" */
INTDEF NTSTATUS NTAPI NtCreateSemaphore(OUT PHANDLE SemaphoreHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN LONG InitialCount, IN LONG MaximumCount);
INTDEF NTSTATUS NTAPI NtOpenSemaphore(OUT PHANDLE SemaphoreHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQuerySemaphore(IN HANDLE SemaphoreHandle, IN SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass, OUT PVOID SemaphoreInformation, IN ULONG SemaphoreInformationLength, OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtReleaseSemaphore(IN HANDLE SemaphoreHandle, IN LONG ReleaseCount, OUT PLONG PreviousCount OPTIONAL);

/* ReactOS: "nsoskrnl/ex/shutdown.c" */
INTDEF NTSTATUS NTAPI NtShutdownSystem(IN SHUTDOWN_ACTION Action);

/* ReactOS: "nsoskrnl/ex/sysinfo.c" */
INTDEF NTSTATUS NTAPI NtQuerySystemEnvironmentValue(IN PUNICODE_STRING VariableName, OUT PWSTR ValueBuffer, IN ULONG ValueBufferLength, IN OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetSystemEnvironmentValue(IN PUNICODE_STRING VariableName, IN PUNICODE_STRING Value);
INTDEF NTSTATUS NTAPI NtEnumerateSystemEnvironmentValuesEx(IN ULONG InformationClass, IN PVOID Buffer, IN ULONG BufferLength);
INTDEF NTSTATUS NTAPI NtQuerySystemEnvironmentValueEx(IN PUNICODE_STRING VariableName, IN LPGUID VendorGuid, IN PVOID Value, IN OUT PULONG ReturnLength, IN OUT PULONG Attributes);
INTDEF NTSTATUS NTAPI NtSetSystemEnvironmentValueEx(IN PUNICODE_STRING VariableName, IN LPGUID VendorGuid, IN PVOID Value, IN OUT PULONG ReturnLength, IN OUT PULONG Attributes);
INTDEF NTSTATUS NTAPI NtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, OUT PVOID SystemInformation, IN ULONG Length, OUT PULONG UnsafeResultLength);
INTDEF NTSTATUS NTAPI NtSetSystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, IN PVOID SystemInformation, IN ULONG SystemInformationLength);
INTDEF ULONG NTAPI NtGetCurrentProcessorNumber(VOID);

/* ReactOS: "nsoskrnl/ex/time.c" */
INTDEF NTSTATUS NTAPI NtSetSystemTime(IN PLARGE_INTEGER SystemTime, OUT PLARGE_INTEGER PreviousTime OPTIONAL);
INTDEF NTSTATUS NTAPI NtQuerySystemTime(OUT PLARGE_INTEGER SystemTime);
INTDEF NTSTATUS NTAPI NtQueryTimerResolution(OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG ActualResolution);
INTDEF NTSTATUS NTAPI NtSetTimerResolution(IN ULONG DesiredResolution, IN BOOLEAN SetResolution, OUT PULONG CurrentResolution);

/* ReactOS: "nsoskrnl/ex/timer.c" */
INTDEF NTSTATUS NTAPI NtCancelTimer(IN HANDLE TimerHandle, OUT PBOOLEAN CurrentState OPTIONAL);
INTDEF NTSTATUS NTAPI NtCreateTimer(OUT PHANDLE TimerHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN TIMER_TYPE TimerType);
INTDEF NTSTATUS NTAPI NtOpenTimer(OUT PHANDLE TimerHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQueryTimer(IN HANDLE TimerHandle, IN TIMER_INFORMATION_CLASS TimerInformationClass, OUT PVOID TimerInformation, IN ULONG TimerInformationLength, OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetTimer(IN HANDLE TimerHandle, IN PLARGE_INTEGER DueTime, IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL, IN PVOID TimerContext OPTIONAL, IN BOOLEAN WakeTimer, IN LONG Period OPTIONAL, OUT PBOOLEAN PreviousState OPTIONAL);

/* ReactOS: "nsoskrnl/ex/uuid.c" */
INTDEF NTSTATUS NTAPI NtAllocateLocallyUniqueId(OUT LUID *LocallyUniqueId);
INTDEF NTSTATUS NTAPI NtAllocateUuids(OUT PULARGE_INTEGER Time, OUT PULONG Range, OUT PULONG Sequence, OUT PUCHAR Seed);
INTDEF NTSTATUS NTAPI NtSetUuidSeed(IN PUCHAR Seed);

/* ReactOS: "nsoskrnl/inbv/inbv.c" */
INTDEF NTSTATUS NTAPI NtDisplayString(IN PUNICODE_STRING DisplayString);

/* ReactOS: "nsoskrnl/io/iomgr/driver.c" */
INTDEF NTSTATUS NTAPI NtLoadDriver(IN PUNICODE_STRING DriverServiceName);
INTDEF NTSTATUS NTAPI NtUnloadDriver(IN PUNICODE_STRING DriverServiceName);

/* ReactOS: "nsoskrnl/io/iomgr/file.c" */
INTDEF NTSTATUS NTAPI NtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocateSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);
INTDEF NTSTATUS NTAPI NtCreateMailslotFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG CreateOptions, IN ULONG MailslotQuota, IN ULONG MaxMessageSize, IN PLARGE_INTEGER TimeOut);
INTDEF NTSTATUS NTAPI NtCreateNamedPipeFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG CreateDisposition, IN ULONG CreateOptions, IN ULONG NamedPipeType, IN ULONG ReadMode, IN ULONG CompletionMode, IN ULONG MaximumInstances, IN ULONG InboundQuota, IN ULONG OutboundQuota, IN PLARGE_INTEGER DefaultTimeout);
INTDEF NTSTATUS NTAPI NtFlushWriteBuffer(VOID);
INTDEF NTSTATUS NTAPI NtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions);
INTDEF NTSTATUS NTAPI NtQueryAttributesFile(IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PFILE_BASIC_INFORMATION FileInformation);
INTDEF NTSTATUS NTAPI NtQueryFullAttributesFile(IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation);
INTDEF NTSTATUS NTAPI NtCancelIoFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock);
INTDEF NTSTATUS NTAPI NtDeleteFile(IN POBJECT_ATTRIBUTES ObjectAttributes);

/* ReactOS: "nsoskrnl/io/iomgr/iocomp.c" */
INTDEF NTSTATUS NTAPI NtCreateIoCompletion(OUT PHANDLE IoCompletionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG NumberOfConcurrentThreads);
INTDEF NTSTATUS NTAPI NtOpenIoCompletion(OUT PHANDLE IoCompletionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQueryIoCompletion(IN HANDLE IoCompletionHandle, IN IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass, OUT PVOID IoCompletionInformation, IN ULONG IoCompletionInformationLength, OUT PULONG ResultLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtRemoveIoCompletion(IN HANDLE IoCompletionHandle, OUT PVOID *KeyContext, OUT PVOID *ApcContext, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PLARGE_INTEGER Timeout OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetIoCompletion(IN HANDLE IoCompletionPortHandle, IN PVOID CompletionKey, IN PVOID CompletionContext, IN NTSTATUS CompletionStatus, IN ULONG CompletionInformation);

/* ReactOS: "nsoskrnl/io/iomgr/iofunc.c" */
INTDEF NTSTATUS NTAPI NtDeviceIoControlFile(IN HANDLE DeviceHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG IoControlCode, IN PVOID InputBuffer, IN ULONG InputBufferLength OPTIONAL, OUT PVOID OutputBuffer, IN ULONG OutputBufferLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtFsControlFile(IN HANDLE DeviceHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG IoControlCode, IN PVOID InputBuffer, IN ULONG InputBufferLength OPTIONAL, OUT PVOID OutputBuffer, IN ULONG OutputBufferLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtFlushBuffersFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock);
INTDEF NTSTATUS NTAPI NtNotifyChangeDirectoryFile(IN HANDLE FileHandle, IN HANDLE EventHandle OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG BufferSize, IN ULONG CompletionFilter, IN BOOLEAN WatchTree);
INTDEF NTSTATUS NTAPI NtLockFile(IN HANDLE FileHandle, IN HANDLE EventHandle OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PLARGE_INTEGER ByteOffset, IN PLARGE_INTEGER Length, IN ULONG Key, IN BOOLEAN FailImmediately, IN BOOLEAN ExclusiveLock);
INTDEF NTSTATUS NTAPI NtQueryDirectoryFile(IN HANDLE FileHandle, IN HANDLE EventHandle OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass, IN BOOLEAN ReturnSingleEntry, IN PUNICODE_STRING FileName OPTIONAL, IN BOOLEAN RestartScan);
INTDEF NTSTATUS NTAPI NtQueryEaFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN ReturnSingleEntry, IN PVOID EaList OPTIONAL, IN ULONG EaListLength, IN PULONG EaIndex OPTIONAL, IN BOOLEAN RestartScan);
INTDEF NTSTATUS NTAPI NtQueryInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass);
INTDEF NTSTATUS NTAPI NtQueryQuotaInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN ReturnSingleEntry, IN PVOID SidList OPTIONAL, IN ULONG SidListLength, IN PSID StartSid OPTIONAL, IN BOOLEAN RestartScan);
INTDEF NTSTATUS NTAPI NtReadFile(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG Length, IN PLARGE_INTEGER ByteOffset OPTIONAL, IN PULONG Key OPTIONAL);
INTDEF NTSTATUS NTAPI NtReadFileScatter(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK UserIoStatusBlock, IN FILE_SEGMENT_ELEMENT BufferDescription[], IN ULONG BufferLength, IN PLARGE_INTEGER ByteOffset, IN PULONG Key OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetEaFile(IN HANDLE FileHandle, IN PIO_STATUS_BLOCK IoStatusBlock, IN PVOID EaBuffer, IN ULONG EaBufferSize);
INTDEF NTSTATUS NTAPI NtSetInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass);
INTDEF NTSTATUS NTAPI NtSetQuotaInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID Buffer, IN ULONG BufferLength);
INTDEF NTSTATUS NTAPI NtUnlockFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PLARGE_INTEGER ByteOffset, IN PLARGE_INTEGER Length, IN ULONG Key OPTIONAL);
INTDEF NTSTATUS NTAPI NtWriteFile(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID Buffer, IN ULONG Length, IN PLARGE_INTEGER ByteOffset OPTIONAL, IN PULONG Key OPTIONAL);
INTDEF NTSTATUS NTAPI NtWriteFileGather(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK UserIoStatusBlock, IN FILE_SEGMENT_ELEMENT BufferDescription[], IN ULONG BufferLength, IN PLARGE_INTEGER ByteOffset, IN PULONG Key OPTIONAL);
INTDEF NTSTATUS NTAPI NtQueryVolumeInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID FsInformation, IN ULONG Length, IN FS_INFORMATION_CLASS FsInformationClass);
INTDEF NTSTATUS NTAPI NtSetVolumeInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID FsInformation, IN ULONG Length, IN FS_INFORMATION_CLASS FsInformationClass);
INTDEF NTSTATUS NTAPI NtCancelDeviceWakeupRequest(IN HANDLE DeviceHandle);
INTDEF NTSTATUS NTAPI NtRequestDeviceWakeup(IN HANDLE DeviceHandle);

/* ReactOS: "nsoskrnl/io/pnpmgr/plugplay.c" */
INTDEF NTSTATUS NTAPI NtGetPlugPlayEvent(IN ULONG Reserved1, IN ULONG Reserved2, OUT PPLUGPLAY_EVENT_BLOCK Buffer, IN ULONG BufferSize);
INTDEF NTSTATUS NTAPI NtPlugPlayControl(IN PLUGPLAY_CONTROL_CLASS PlugPlayControlClass, IN OUT PVOID Buffer, IN ULONG BufferLength);

/* ReactOS: "nsoskrnl/kd/kdmain.c" */
INTDEF NTSTATUS NTAPI NtQueryDebugFilterState(IN ULONG ComponentId, IN ULONG Level);
INTDEF NTSTATUS NTAPI NtSetDebugFilterState(IN ULONG ComponentId, IN ULONG Level, IN BOOLEAN State);

/* ReactOS: "nsoskrnl/ke/thrdschd.c" */
INTDEF NTSTATUS NTAPI NtYieldExecution(VOID);

/* ReactOS: "nsoskrnl/ke/wait.c" */
INTDEF NTSTATUS NTAPI NtDelayExecution(IN BOOLEAN Alertable, IN PLARGE_INTEGER DelayInterval);

/* ReactOS: "nsoskrnl/ke/except.c" */
INTDEF NTSTATUS NTAPI NtRaiseException(IN PEXCEPTION_RECORD ExceptionRecord, IN PCONTEXT Context, IN BOOLEAN FirstChance);
INTDEF NTSTATUS NTAPI NtContinue(IN PCONTEXT Context, IN BOOLEAN TestAlert);

#if defined(__i386__) || defined(__x86_64__)
/* ReactOS: "nsoskrnl/ke/i386/ldt.c" */
INTDEF NTSTATUS NTAPI NtSetLdtEntries(ULONG Selector1, LDT_ENTRY LdtEntry1, ULONG Selector2, LDT_ENTRY LdtEntry2);
#endif

/* ReactOS: "nsoskrnl/ke/i386/usercall.c" */
INTDEF NTSTATUS NTAPI NtCallbackReturn(_In_ PVOID Result, _In_ ULONG ResultLength, _In_ NTSTATUS CallbackStatus);

/* ReactOS: "nsoskrnl/ke/powerpc/stubs.c" */
INTDEF ULONG NTAPI NtGetTickCount(void);

/* ReactOS: "nsoskrnl/lpc/connect.c" */
INTDEF NTSTATUS NTAPI NtSecureConnectPort(OUT PHANDLE PortHandle, IN PUNICODE_STRING PortName, IN PSECURITY_QUALITY_OF_SERVICE SecurityQos, IN OUT PPORT_VIEW ClientView OPTIONAL, IN PSID ServerSid OPTIONAL, IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL, OUT PULONG MaxMessageLength OPTIONAL, IN OUT PVOID ConnectionInformation OPTIONAL, IN OUT PULONG ConnectionInformationLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtConnectPort(OUT PHANDLE PortHandle, IN PUNICODE_STRING PortName, IN PSECURITY_QUALITY_OF_SERVICE SecurityQos, IN OUT PPORT_VIEW ClientView OPTIONAL, IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL, OUT PULONG MaxMessageLength OPTIONAL, IN OUT PVOID ConnectionInformation OPTIONAL, IN OUT PULONG ConnectionInformationLength OPTIONAL);

/* ReactOS: "nsoskrnl/lpc/create.c" */
INTDEF NTSTATUS NTAPI NtCreatePort(OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectInfoLength, IN ULONG MaxDataLength, IN ULONG MaxPoolUsage);
INTDEF NTSTATUS NTAPI NtCreateWaitablePort(OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectInfoLength, IN ULONG MaxDataLength, IN ULONG MaxPoolUsage);

/* ReactOS: "nsoskrnl/lpc/listen.c" */
INTDEF NTSTATUS NTAPI NtListenPort(IN HANDLE PortHandle, OUT PPORT_MESSAGE ConnectMessage);

/* ReactOS: "nsoskrnl/lpc/port.c" */
INTDEF NTSTATUS NTAPI NtImpersonateClientOfPort(IN HANDLE PortHandle, IN PPORT_MESSAGE ClientMessage);
INTDEF NTSTATUS NTAPI NtQueryPortInformationProcess(VOID);
INTDEF NTSTATUS NTAPI NtQueryInformationPort(IN HANDLE PortHandle, IN PORT_INFORMATION_CLASS PortInformationClass, OUT PVOID PortInformation, IN ULONG PortInformationLength, OUT PULONG ReturnLength);

/* ReactOS: "nsoskrnl/lpc/reply.c" */
INTDEF NTSTATUS NTAPI NtReplyPort(IN HANDLE PortHandle, IN PPORT_MESSAGE ReplyMessage);
INTDEF NTSTATUS NTAPI NtReplyWaitReceivePortEx(IN HANDLE PortHandle, OUT PVOID *PortContext OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE ReceiveMessage, IN PLARGE_INTEGER Timeout OPTIONAL);
INTDEF NTSTATUS NTAPI NtReplyWaitReceivePort(IN HANDLE PortHandle, OUT PVOID *PortContext OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE ReceiveMessage);
INTDEF NTSTATUS NTAPI NtReplyWaitReplyPort(IN HANDLE PortHandle, IN PPORT_MESSAGE ReplyMessage);
INTDEF NTSTATUS NTAPI NtReadRequestData(IN HANDLE PortHandle, IN PPORT_MESSAGE Message, IN ULONG Index, IN PVOID Buffer, IN ULONG BufferLength, OUT PULONG ReturnLength);
INTDEF NTSTATUS NTAPI NtWriteRequestData(IN HANDLE PortHandle, IN PPORT_MESSAGE Message, IN ULONG Index, IN PVOID Buffer, IN ULONG BufferLength, OUT PULONG ReturnLength);

/* ReactOS: "nsoskrnl/lpc/send.c" */
INTDEF NTSTATUS NTAPI NtRequestPort(IN HANDLE PortHandle, IN PPORT_MESSAGE LpcRequest);
INTDEF NTSTATUS NTAPI NtRequestWaitReplyPort(IN HANDLE PortHandle, IN PPORT_MESSAGE LpcRequest, IN OUT PPORT_MESSAGE LpcReply);

/* ReactOS: "nsoskrnl/lpc/complete.c" */
INTDEF NTSTATUS NTAPI NtAcceptConnectPort(OUT PHANDLE PortHandle, IN PVOID PortContext OPTIONAL, IN PPORT_MESSAGE ReplyMessage, IN BOOLEAN AcceptConnection, IN OUT PPORT_VIEW ServerView OPTIONAL, OUT PREMOTE_PORT_VIEW ClientView OPTIONAL);
INTDEF NTSTATUS NTAPI NtCompleteConnectPort(IN HANDLE PortHandle);

/* ReactOS: "nsoskrnl/mm/pagefile.c" */
INTDEF NTSTATUS NTAPI NtCreatePagingFile(IN PUNICODE_STRING FileName, IN PLARGE_INTEGER InitialSize, IN PLARGE_INTEGER MaximumSize, IN ULONG Reserved);

/* ReactOS: "nsoskrnl/mm/section.c" */
INTDEF NTSTATUS NTAPI NtQuerySection(_In_ HANDLE SectionHandle, _In_ SECTION_INFORMATION_CLASS SectionInformationClass, _Out_ PVOID SectionInformation, _In_ SIZE_T SectionInformationLength, _Out_opt_ PSIZE_T ResultLength);

/* ReactOS: "nsoskrnl/mm/ARM3/procsup.c" */
INTDEF NTSTATUS NTAPI NtAllocateUserPhysicalPages(IN HANDLE ProcessHandle, IN OUT PULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray);
INTDEF NTSTATUS NTAPI NtMapUserPhysicalPages(IN PVOID VirtualAddresses, IN ULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray);
INTDEF NTSTATUS NTAPI NtMapUserPhysicalPagesScatter(IN PVOID *VirtualAddresses, IN ULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray);
INTDEF NTSTATUS NTAPI NtFreeUserPhysicalPages(IN HANDLE ProcessHandle, IN OUT PULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray);

/* ReactOS: "nsoskrnl/mm/ARM3/section.c" */
INTDEF NTSTATUS NTAPI NtAreMappedFilesTheSame(IN PVOID File1MappedAsAnImage, IN PVOID File2MappedAsFile);
INTDEF NTSTATUS NTAPI NtCreateSection(OUT PHANDLE SectionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN PLARGE_INTEGER MaximumSize OPTIONAL, IN ULONG SectionPageProtection OPTIONAL, IN ULONG AllocationAttributes, IN HANDLE FileHandle OPTIONAL);
INTDEF NTSTATUS NTAPI NtOpenSection(OUT PHANDLE SectionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtMapViewOfSection(IN HANDLE SectionHandle, IN HANDLE ProcessHandle, IN OUT PVOID* BaseAddress, IN ULONG_PTR ZeroBits, IN SIZE_T CommitSize, IN OUT PLARGE_INTEGER SectionOffset OPTIONAL, IN OUT PSIZE_T ViewSize, IN SECTION_INHERIT InheritDisposition, IN ULONG AllocationType, IN ULONG Protect);
INTDEF NTSTATUS NTAPI NtUnmapViewOfSection(IN HANDLE ProcessHandle, IN PVOID BaseAddress);
INTDEF NTSTATUS NTAPI NtExtendSection(IN HANDLE SectionHandle, IN OUT PLARGE_INTEGER NewMaximumSize);

/* ReactOS: "nsoskrnl/mm/ARM3/virtual.c" */
INTDEF NTSTATUS NTAPI NtReadVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, OUT PVOID Buffer, IN SIZE_T NumberOfBytesToRead, OUT PSIZE_T NumberOfBytesRead OPTIONAL);
INTDEF NTSTATUS NTAPI NtWriteVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, IN PVOID Buffer, IN SIZE_T NumberOfBytesToWrite, OUT PSIZE_T NumberOfBytesWritten OPTIONAL);
INTDEF NTSTATUS NTAPI NtFlushInstructionCache(_In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress, _In_ SIZE_T FlushSize);
INTDEF NTSTATUS NTAPI NtProtectVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *UnsafeBaseAddress, IN OUT SIZE_T *UnsafeNumberOfBytesToProtect, IN ULONG NewAccessProtection, OUT PULONG UnsafeOldAccessProtection);
INTDEF NTSTATUS NTAPI NtLockVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T NumberOfBytesToLock, IN ULONG MapType);
INTDEF NTSTATUS NTAPI NtUnlockVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T NumberOfBytesToUnlock, IN ULONG MapType);
INTDEF NTSTATUS NTAPI NtFlushVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T NumberOfBytesToFlush, OUT PIO_STATUS_BLOCK IoStatusBlock);
INTDEF NTSTATUS NTAPI NtGetWriteWatch(IN HANDLE ProcessHandle, IN ULONG Flags, IN PVOID BaseAddress, IN SIZE_T RegionSize, IN PVOID *UserAddressArray, OUT PULONG_PTR EntriesInUserAddressArray, OUT PULONG Granularity);
INTDEF NTSTATUS NTAPI NtResetWriteWatch(IN HANDLE ProcessHandle, IN PVOID BaseAddress, IN SIZE_T RegionSize);
INTDEF NTSTATUS NTAPI NtQueryVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, IN MEMORY_INFORMATION_CLASS MemoryInformationClass, OUT PVOID MemoryInformation, IN SIZE_T MemoryInformationLength, OUT PSIZE_T ReturnLength);
INTDEF NTSTATUS NTAPI NtAllocateVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID* UBaseAddress, IN ULONG_PTR ZeroBits, IN OUT PSIZE_T URegionSize, IN ULONG AllocationType, IN ULONG Protect);
INTDEF NTSTATUS NTAPI NtFreeVirtualMemory(IN HANDLE ProcessHandle, IN PVOID* UBaseAddress, IN PSIZE_T URegionSize, IN ULONG FreeType);

/* ReactOS: "nsoskrnl/ob/oblife.c" */
INTDEF NTSTATUS NTAPI NtMakeTemporaryObject(IN HANDLE ObjectHandle);
INTDEF NTSTATUS NTAPI NtMakePermanentObject(IN HANDLE ObjectHandle);
INTDEF NTSTATUS NTAPI NtQueryObject(IN HANDLE ObjectHandle, IN OBJECT_INFORMATION_CLASS ObjectInformationClass, OUT PVOID ObjectInformation, IN ULONG Length, OUT PULONG ResultLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetInformationObject(IN HANDLE ObjectHandle, IN OBJECT_INFORMATION_CLASS ObjectInformationClass, IN PVOID ObjectInformation, IN ULONG Length);

/* ReactOS: "nsoskrnl/ob/oblink.c" */
INTDEF NTSTATUS NTAPI NtCreateSymbolicLinkObject(OUT PHANDLE LinkHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PUNICODE_STRING LinkTarget);
INTDEF NTSTATUS NTAPI NtOpenSymbolicLinkObject(OUT PHANDLE LinkHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQuerySymbolicLinkObject(IN HANDLE LinkHandle, OUT PUNICODE_STRING LinkTarget, OUT PULONG ResultLength OPTIONAL);

/* ReactOS: "nsoskrnl/ob/obsecure.c" */
INTDEF NTSTATUS NTAPI NtQuerySecurityObject(IN HANDLE Handle, IN SECURITY_INFORMATION SecurityInformation, OUT PSECURITY_DESCRIPTOR SecurityDescriptor, IN ULONG Length, OUT PULONG ResultLength);
INTDEF NTSTATUS NTAPI NtSetSecurityObject(IN HANDLE Handle, IN SECURITY_INFORMATION SecurityInformation, IN PSECURITY_DESCRIPTOR SecurityDescriptor);

/* ReactOS: "nsoskrnl/ob/obwait.c" */
INTDEF NTSTATUS NTAPI NtWaitForMultipleObjects(IN ULONG ObjectCount, IN PHANDLE HandleArray, IN WAIT_TYPE WaitType, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL);
INTDEF NTSTATUS NTAPI NtWaitForMultipleObjects32(IN ULONG ObjectCount, IN PLONG Handles, IN WAIT_TYPE WaitType, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL);
INTDEF NTSTATUS NTAPI NtWaitForSingleObject(IN HANDLE ObjectHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL);
INTDEF NTSTATUS NTAPI NtSignalAndWaitForSingleObject(IN HANDLE ObjectHandleToSignal, IN HANDLE WaitableObjectHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL);

/* ReactOS: "nsoskrnl/ob/obdir.c" */
INTDEF NTSTATUS NTAPI NtOpenDirectoryObject(OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQueryDirectoryObject(IN HANDLE DirectoryHandle, OUT PVOID Buffer, IN ULONG BufferLength, IN BOOLEAN ReturnSingleEntry, IN BOOLEAN RestartScan, IN OUT PULONG Context, OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtCreateDirectoryObject(OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes);

/* ReactOS: "nsoskrnl/ob/obhandle.c" */
INTDEF NTSTATUS NTAPI NtClose(IN HANDLE Handle);
INTDEF NTSTATUS NTAPI NtDuplicateObject(IN HANDLE SourceProcessHandle, IN HANDLE SourceHandle, IN HANDLE TargetProcessHandle OPTIONAL, OUT PHANDLE TargetHandle OPTIONAL, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, IN ULONG Options);

/* ReactOS: "nsoskrnl/po/power.c" */
INTDEF NTSTATUS NTAPI NtInitiatePowerAction(IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG Flags, IN BOOLEAN Asynchronous);
INTDEF NTSTATUS NTAPI NtPowerInformation(IN POWER_INFORMATION_LEVEL PowerInformationLevel, IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength, OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength);
INTDEF NTSTATUS NTAPI NtGetDevicePowerState(IN HANDLE Device, IN PDEVICE_POWER_STATE PowerState);
INTDEF BOOLEAN NTAPI NtIsSystemResumeAutomatic(VOID);
INTDEF NTSTATUS NTAPI NtRequestWakeupLatency(IN LATENCY_TIME Latency);
INTDEF NTSTATUS NTAPI NtSetThreadExecutionState(IN EXECUTION_STATE esFlags, OUT EXECUTION_STATE *PreviousFlags);
INTDEF NTSTATUS NTAPI NtSetSystemPowerState(IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG Flags);

/* ReactOS: "nsoskrnl/ps/process.c" */
INTDEF NTSTATUS NTAPI NtCreateProcessEx(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ParentProcess, IN ULONG Flags, IN HANDLE SectionHandle OPTIONAL, IN HANDLE DebugPort OPTIONAL, IN HANDLE ExceptionPort OPTIONAL, IN BOOLEAN InJob);
INTDEF NTSTATUS NTAPI NtCreateProcess(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ParentProcess, IN BOOLEAN InheritObjectTable, IN HANDLE SectionHandle OPTIONAL, IN HANDLE DebugPort OPTIONAL, IN HANDLE ExceptionPort OPTIONAL);
INTDEF NTSTATUS NTAPI NtOpenProcess(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId);

/* ReactOS: "nsoskrnl/ps/query.c" */
INTDEF NTSTATUS NTAPI NtQueryInformationProcess(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL);
INTDEF NTSTATUS NTAPI NtSetInformationProcess(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, IN PVOID ProcessInformation, IN ULONG ProcessInformationLength);
INTDEF NTSTATUS NTAPI NtSetInformationThread(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, IN PVOID ThreadInformation, IN ULONG ThreadInformationLength);
INTDEF NTSTATUS NTAPI NtQueryInformationThread(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, OUT PVOID ThreadInformation, IN ULONG ThreadInformationLength, OUT PULONG ReturnLength OPTIONAL);

/* ReactOS: "nsoskrnl/ps/security.c" */
INTDEF NTSTATUS NTAPI NtOpenProcessToken(IN HANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, OUT PHANDLE TokenHandle);
INTDEF NTSTATUS NTAPI NtOpenProcessTokenEx(IN HANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, OUT PHANDLE TokenHandle);
INTDEF NTSTATUS NTAPI NtImpersonateThread(IN HANDLE ThreadHandle, IN HANDLE ThreadToImpersonateHandle, IN PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService);

/* ReactOS: "nsoskrnl/ps/state.c" */
INTDEF NTSTATUS NTAPI NtAlertThread(IN HANDLE ThreadHandle);
INTDEF NTSTATUS NTAPI NtAlertResumeThread(IN HANDLE ThreadHandle, OUT PULONG SuspendCount);
INTDEF NTSTATUS NTAPI NtResumeThread(IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL);
INTDEF NTSTATUS NTAPI NtSuspendThread(IN HANDLE ThreadHandle, OUT PULONG PreviousSuspendCount OPTIONAL);
INTDEF NTSTATUS NTAPI NtSuspendProcess(IN HANDLE ProcessHandle);
INTDEF NTSTATUS NTAPI NtResumeProcess(IN HANDLE ProcessHandle);
INTDEF NTSTATUS NTAPI NtTestAlert(VOID);
INTDEF NTSTATUS NTAPI NtQueueApcThread(IN HANDLE ThreadHandle, IN PKNORMAL_ROUTINE ApcRoutine, IN PVOID NormalContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2);

/* ReactOS: "nsoskrnl/ps/thread.c" */
INTDEF NTSTATUS NTAPI NtCreateThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ProcessHandle, OUT PCLIENT_ID ClientId, IN PCONTEXT ThreadContext, IN PINITIAL_TEB InitialTeb, IN BOOLEAN CreateSuspended);
INTDEF NTSTATUS NTAPI NtOpenThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId OPTIONAL);

/* ReactOS: "nsoskrnl/ps/apphelp.c" */
INTDEF NTSTATUS NTAPI NtApphelpCacheControl(_In_ APPHELPCACHESERVICECLASS Service, _In_opt_ PAPPHELP_CACHE_SERVICE_LOOKUP ServiceData);

/* ReactOS: "nsoskrnl/ps/debug.c" */
INTDEF NTSTATUS NTAPI NtGetContextThread(IN HANDLE ThreadHandle, IN OUT PCONTEXT ThreadContext);
INTDEF NTSTATUS NTAPI NtSetContextThread(IN HANDLE ThreadHandle, IN PCONTEXT ThreadContext);

/* ReactOS: "nsoskrnl/ps/job.c" */
INTDEF NTSTATUS NTAPI NtAssignProcessToJobObject(HANDLE JobHandle, HANDLE ProcessHandle);
INTDEF NTSTATUS NTAPI NtCreateJobSet(IN ULONG NumJob, IN PJOB_SET_ARRAY UserJobSet, IN ULONG Flags);
INTDEF NTSTATUS NTAPI NtCreateJobObject(PHANDLE JobHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtIsProcessInJob(IN HANDLE ProcessHandle, IN HANDLE JobHandle OPTIONAL);
INTDEF NTSTATUS NTAPI NtOpenJobObject(PHANDLE JobHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes);
INTDEF NTSTATUS NTAPI NtQueryInformationJobObject(HANDLE JobHandle, JOBOBJECTINFOCLASS JobInformationClass, PVOID JobInformation, ULONG JobInformationLength, PULONG ReturnLength);
INTDEF NTSTATUS NTAPI NtSetInformationJobObject(HANDLE JobHandle, JOBOBJECTINFOCLASS JobInformationClass, PVOID JobInformation, ULONG JobInformationLength);
INTDEF NTSTATUS NTAPI NtTerminateJobObject(HANDLE JobHandle, NTSTATUS ExitStatus);

/* ReactOS: "nsoskrnl/ps/kill.c" */
INTDEF NTSTATUS NTAPI NtTerminateProcess(IN HANDLE ProcessHandle OPTIONAL, IN NTSTATUS ExitStatus);
INTDEF NTSTATUS NTAPI NtTerminateThread(IN HANDLE ThreadHandle, IN NTSTATUS ExitStatus);
INTDEF NTSTATUS NTAPI NtRegisterThreadTerminatePort(IN HANDLE PortHandle);

/* ReactOS: "nsoskrnl/se/accesschk.c" */
INTDEF NTSTATUS NTAPI NtAccessCheck(IN PSECURITY_DESCRIPTOR SecurityDescriptor, IN HANDLE TokenHandle, IN ACCESS_MASK DesiredAccess, IN PGENERIC_MAPPING GenericMapping, OUT PPRIVILEGE_SET PrivilegeSet OPTIONAL, IN OUT PULONG PrivilegeSetLength, OUT PACCESS_MASK GrantedAccess, OUT PNTSTATUS AccessStatus);
INTDEF NTSTATUS NTAPI NtAccessCheckByType(IN PSECURITY_DESCRIPTOR SecurityDescriptor, IN PSID PrincipalSelfSid, IN HANDLE ClientToken, IN ACCESS_MASK DesiredAccess, IN POBJECT_TYPE_LIST ObjectTypeList, IN ULONG ObjectTypeLength, IN PGENERIC_MAPPING GenericMapping, IN PPRIVILEGE_SET PrivilegeSet, IN OUT PULONG PrivilegeSetLength, OUT PACCESS_MASK GrantedAccess, OUT PNTSTATUS AccessStatus);
INTDEF NTSTATUS NTAPI NtAccessCheckByTypeResultList(IN PSECURITY_DESCRIPTOR SecurityDescriptor, IN PSID PrincipalSelfSid, IN HANDLE ClientToken, IN ACCESS_MASK DesiredAccess, IN POBJECT_TYPE_LIST ObjectTypeList, IN ULONG ObjectTypeLength, IN PGENERIC_MAPPING GenericMapping, IN PPRIVILEGE_SET PrivilegeSet, IN OUT PULONG PrivilegeSetLength, OUT PACCESS_MASK GrantedAccess, OUT PNTSTATUS AccessStatus);

/* ReactOS: "nsoskrnl/se/audit.c" */
INTDEF NTSTATUS NTAPI NtCloseObjectAuditAlarm(PUNICODE_STRING SubsystemName, PVOID HandleId, BOOLEAN GenerateOnClose);
INTDEF NTSTATUS NTAPI NtDeleteObjectAuditAlarm(IN PUNICODE_STRING SubsystemName, IN PVOID HandleId, IN BOOLEAN GenerateOnClose);
INTDEF NTSTATUS NTAPI NtOpenObjectAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_opt_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_ HANDLE ClientTokenHandle, _In_ ACCESS_MASK DesiredAccess, _In_ ACCESS_MASK GrantedAccess, _In_opt_ PPRIVILEGE_SET PrivilegeSet, _In_ BOOLEAN ObjectCreation, _In_ BOOLEAN AccessGranted, _Out_ PBOOLEAN GenerateOnClose);
INTDEF NTSTATUS NTAPI NtPrivilegedServiceAuditAlarm(_In_opt_ PUNICODE_STRING SubsystemName, _In_opt_ PUNICODE_STRING ServiceName, _In_ HANDLE ClientTokenHandle, _In_ PPRIVILEGE_SET Privileges, _In_ BOOLEAN AccessGranted);
INTDEF NTSTATUS NTAPI NtPrivilegeObjectAuditAlarm(IN PUNICODE_STRING SubsystemName, IN PVOID HandleId, IN HANDLE ClientToken, IN ULONG DesiredAccess, IN PPRIVILEGE_SET Privileges, IN BOOLEAN AccessGranted);
INTDEF NTSTATUS NTAPI NtAccessCheckAndAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_ ACCESS_MASK DesiredAccess, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_ PACCESS_MASK GrantedAccess, _Out_ PNTSTATUS AccessStatus, _Out_ PBOOLEAN GenerateOnClose);
INTDEF NTSTATUS NTAPI NtAccessCheckByTypeAndAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID PrincipalSelfSid, _In_ ACCESS_MASK DesiredAccess, _In_ AUDIT_EVENT_TYPE AuditType, _In_ ULONG Flags, _In_reads_opt_(ObjectTypeLength) POBJECT_TYPE_LIST ObjectTypeList, _In_ ULONG ObjectTypeLength, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_ PACCESS_MASK GrantedAccess, _Out_ PNTSTATUS AccessStatus, _Out_ PBOOLEAN GenerateOnClose);
INTDEF NTSTATUS NTAPI NtAccessCheckByTypeResultListAndAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID PrincipalSelfSid, _In_ ACCESS_MASK DesiredAccess, _In_ AUDIT_EVENT_TYPE AuditType, _In_ ULONG Flags, _In_reads_opt_(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList, _In_ ULONG ObjectTypeListLength, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_writes_(ObjectTypeListLength) PACCESS_MASK GrantedAccessList, _Out_writes_(ObjectTypeListLength) PNTSTATUS AccessStatusList, _Out_ PBOOLEAN GenerateOnClose);
INTDEF NTSTATUS NTAPI NtAccessCheckByTypeResultListAndAuditAlarmByHandle(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ HANDLE ClientToken, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID PrincipalSelfSid, _In_ ACCESS_MASK DesiredAccess, _In_ AUDIT_EVENT_TYPE AuditType, _In_ ULONG Flags, _In_reads_opt_(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList, _In_ ULONG ObjectTypeListLength, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_writes_(ObjectTypeListLength) PACCESS_MASK GrantedAccessList, _Out_writes_(ObjectTypeListLength) PNTSTATUS AccessStatusList, _Out_ PBOOLEAN GenerateOnClose);

/* ReactOS: "nsoskrnl/se/priv.c" */
INTDEF NTSTATUS NTAPI NtPrivilegeCheck(IN HANDLE ClientToken, IN PPRIVILEGE_SET RequiredPrivileges, OUT PBOOLEAN Result);

/* ReactOS: "nsoskrnl/se/token.c" */
INTDEF NTSTATUS NTAPI NtQueryInformationToken(IN HANDLE TokenHandle, IN TOKEN_INFORMATION_CLASS TokenInformationClass, OUT PVOID TokenInformation, IN ULONG TokenInformationLength, OUT PULONG ReturnLength);
INTDEF NTSTATUS NTAPI NtSetInformationToken(IN HANDLE TokenHandle, IN TOKEN_INFORMATION_CLASS TokenInformationClass, IN PVOID TokenInformation, IN ULONG TokenInformationLength);
INTDEF NTSTATUS NTAPI NtDuplicateToken(IN HANDLE ExistingTokenHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN BOOLEAN EffectiveOnly, IN TOKEN_TYPE TokenType, OUT PHANDLE NewTokenHandle);
INTDEF NTSTATUS NTAPI NtAdjustGroupsToken(IN HANDLE TokenHandle, IN BOOLEAN ResetToDefault, IN PTOKEN_GROUPS NewState, IN ULONG BufferLength, OUT PTOKEN_GROUPS PreviousState OPTIONAL, OUT PULONG ReturnLength);
INTDEF NTSTATUS NTAPI NtAdjustPrivilegesToken(_In_ HANDLE TokenHandle, _In_ BOOLEAN DisableAllPrivileges, _In_opt_ PTOKEN_PRIVILEGES NewState, _In_ ULONG BufferLength, _Out_writes_bytes_to_opt_(BufferLength,*ReturnLength) PTOKEN_PRIVILEGES PreviousState, _When_(PreviousState!=NULL, _Out_) PULONG ReturnLength);
INTDEF NTSTATUS NTAPI NtCreateToken(_Out_ PHANDLE TokenHandle, _In_ ACCESS_MASK DesiredAccess, _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes, _In_ TOKEN_TYPE TokenType, _In_ PLUID AuthenticationId, _In_ PLARGE_INTEGER ExpirationTime, _In_ PTOKEN_USER TokenUser, _In_ PTOKEN_GROUPS TokenGroups, _In_ PTOKEN_PRIVILEGES TokenPrivileges, _In_opt_ PTOKEN_OWNER TokenOwner, _In_ PTOKEN_PRIMARY_GROUP TokenPrimaryGroup, _In_opt_ PTOKEN_DEFAULT_DACL TokenDefaultDacl, _In_ PTOKEN_SOURCE TokenSource);
INTDEF NTSTATUS NTAPI NtOpenThreadTokenEx(IN HANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN BOOLEAN OpenAsSelf, IN ULONG HandleAttributes, OUT PHANDLE TokenHandle);
INTDEF NTSTATUS NTAPI NtOpenThreadToken(IN HANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN BOOLEAN OpenAsSelf, OUT PHANDLE TokenHandle);
INTDEF NTSTATUS NTAPI NtCompareTokens(IN HANDLE FirstTokenHandle, IN HANDLE SecondTokenHandle, OUT PBOOLEAN Equal);
INTDEF NTSTATUS NTAPI NtFilterToken(IN HANDLE ExistingTokenHandle, IN ULONG Flags, IN PTOKEN_GROUPS SidsToDisable OPTIONAL, IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL, IN PTOKEN_GROUPS RestrictedSids OPTIONAL, OUT PHANDLE NewTokenHandle);
INTDEF NTSTATUS NTAPI NtImpersonateAnonymousToken(IN HANDLE Thread);

/* ReactOS: "nsoskrnl/vdm/vdmmain.c" */
INTDEF NTSTATUS NTAPI NtVdmControl(IN ULONG ControlCode, IN PVOID ControlData);

/* ReactOS: "nsoskrnl/wmi/wmi.c" */
struct _EVENT_TRACE_HEADER;
INTDEF NTSTATUS NTAPI NtTraceEvent(IN ULONG TraceHandle, IN ULONG Flags, IN ULONG TraceHeaderLength, IN struct _EVENT_TRACE_HEADER* TraceHeader);

/*
NtAcquireCMFViewOwnership
NtAcquireProcessActivityReference
NtAddAtomEx

NtAdjustTokenClaimsAndDeviceGroups
NtAlertThreadByThreadId
NtAllocateReserveObject

NtAlpcAcceptConnectPort
NtAlpcCancelMessage
NtAlpcConnectPort
NtAlpcConnectPortEx
NtAlpcCreatePort
NtAlpcCreatePortSection
NtAlpcCreateResourceReserve
NtAlpcCreateSectionView
NtAlpcCreateSecurityContext
NtAlpcDeletePortSection
NtAlpcDeleteResourceReserve
NtAlpcDeleteSectionView
NtAlpcDeleteSecurityContext
NtAlpcDisconnectPort
NtAlpcImpersonateClientContainerOfPort
NtAlpcImpersonateClientOfPort
NtAlpcOpenSenderProcess
NtAlpcOpenSenderThread
NtAlpcQueryInformation
NtAlpcQueryInformationMessage
NtAlpcRevokeSecurityContext
NtAlpcSendWaitReceivePort
NtAlpcSetInformation

NtAssociateWaitCompletionPacket

NtCancelIoFileEx
NtCancelSynchronousIoFile

NtCancelTimer2
NtCancelWaitCompletionPacket
NtClearAllSavepointsTransaction

NtClearSavepointTransaction
NtCommitComplete
NtCommitEnlistment
NtCommitRegistryTransaction
NtCommitTransaction
NtCompareObjects
NtCompareSigningLevels

NtConvertBetweenAuxiliaryCounterAndPerformanceCounter
NtCreateChannel

NtCreateDirectoryObjectEx
NtCreateEnclave
NtCreateEnlistment

NtCreateIRTimer

NtCreateKeyTransacted

NtCreateLowBoxToken
NtCreatePartition

NtCreatePrivateNamespace
NtCreateProcess
NtCreateProcessEx

NtCreateProfileEx
NtCreateRegistryTransaction
NtCreateResourceManager

NtCreateThreadEx
NtCreateTimer2

NtCreateTokenEx
NtCreateTransaction
NtCreateTransactionManager
NtCreateUserProcess
NtCreateWaitCompletionPacket

NtCreateWinStation
NtCreateWnfStateName
NtCreateWorkerFactory

NtDeletePrivateNamespace
NtDeleteWnfStateData
NtDeleteWnfStateName
NtDisableLastKnownGood

NtDrawText

NtEnableLastKnownGood

NtEnumerateTransactionObject
NtFilterBootOption

NtFilterTokenEx
NtFlushBuffersFileEx
NtFlushInstallUILanguage

NtFlushProcessWriteBuffers

NtFreezeRegistry
NtFreezeTransactions

NtGetCachedSigningLevel
NtGetCompleteWnfStateSubscription

NtGetCurrentProcessorNumberEx
NtGetMUIRegistryInfo
NtGetNextProcess
NtGetNextThread
NtGetNlsSectionPtr
NtGetNotificationResourceManager

NtInitializeEnclave
NtInitializeNlsFiles

NtIsUILanguageComitted
NtListTransactions
NtListenChannel

NtLoadEnclaveData
NtLoadHotPatch

NtManagePartition
NtMapCMFModule

NtMarshallTransaction

NtNotifyChangeSession
NtOpenChannel
NtOpenEnlistment

NtOpenKeyEx
NtOpenKeyTransacted
NtOpenKeyTransactedEx

NtOpenPartition
NtOpenPrivateNamespace

NtOpenRegistryTransaction
NtOpenResourceManager

NtOpenSession

NtOpenTransaction
NtOpenTransactionManager
NtOpenWinStation

NtPrePrepareComplete
NtPrePrepareEnlistment
NtPrepareComplete
NtPrepareEnlistment
NtPropagationComplete
NtPropagationFailed
NtPullTransaction

NtQueryAuxiliaryCounterFrequency

NtQueryDriverEntryOrder

NtQueryInformationByName
NtQueryInformationEnlistment

NtQueryInformationResourceManager
NtQueryInformationTransaction
NtQueryInformationTransactionManager
NtQueryInformationWorkerFactory

NtQueryLicenseValue

NtQueryOleDirectoryFile

NtQuerySection
NtQuerySecurityAttributesToken
NtQuerySecurityPolicy

NtQuerySystemInformationEx

NtQueryWinStationInformation
NtQueryWnfStateData
NtQueryWnfStateNameInformation
NtQueueApcThreadEx

NtReadOnlyEnlistment
NtRecoverEnlistment
NtRecoverResourceManager
NtRecoverTransactionManager
NtRegisterProtocolAddressInformation
NtReleaseCMFViewOwnership

NtReleaseWorkerFactoryWorker
NtRemoveIoCompletionEx

NtRenameTransactionManager

NtReplacePartitionUnit

NtReplyWaitSendChannel

NtRevertContainerImpersonation
NtRollbackComplete
NtRollbackEnlistment
NtRollbackRegistryTransaction
NtRollbackSavepointTransaction
NtRollbackTransaction
NtRollforwardTransactionManager

NtSavepointComplete
NtSavepointTransaction

NtSendWaitReplyChannel
NtSerializeBoot

NtSetCachedSigningLevel
NtSetCachedSigningLevel2
NtSetContextChannel

NtSetHighWaitLowThread
NtSetIRTimer
NtSetInformationEnlistment

NtSetInformationResourceManager
NtSetInformationSymbolicLink
NtSetInformationTransaction
NtSetInformationTransactionManager
NtSetInformationVirtualMemory
NtSetInformationWorkerFactory

NtSetIoCompletionEx

NtSetLowWaitHighThread

NtSetTimer2
NtSetTimerEx

NtSetWinStationInformation
NtSetWnfProcessNotificationEvent

NtShutdownWorkerFactory
NtSinglePhaseReject

NtStartTm

NtSubscribeWnfStateChange

NtThawRegistry
NtThawTransactions
NtTraceControl

NtUmsThreadYield

NtUnmapViewOfSectionEx
NtUnsubscribeWnfStateChange
NtUpdateWnfStateData

NtW32Call
NtWaitForAlertByThreadId

NtWaitForWnfNotifications
NtWaitForWorkViaWorkerFactory

NtWorkerFactoryWorkerReady
NtWriteErrorLogEntry

*/

DECL_END

#endif /* !GUARD_MODULES_NT_API_H */
