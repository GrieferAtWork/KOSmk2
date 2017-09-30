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
#ifndef GUARD_MODULES_NT_API_C
#define GUARD_MODULES_NT_API_C 1

#include "api.h"
#include <hybrid/compiler.h>
#include <sched/signal.h>
#include <sched/percpu.h>
#include <bits/signum.h>
#include <winapi/ntstatus.h>
#include <syslog.h>

DECL_BEGIN

/* Special system call executed as fallback for unimplemented IDs. */
INTERN void NTAPI NtBadSysCall(VOID) { task_kill(THIS_TASK,SIGSYS); }
INTERN void NTAPI NtSegFault(VOID) { task_kill(THIS_TASK,SIGSEGV); }

PRIVATE void KCALL noimp(int line, char const *func) {
 syslog(LOG_ERROR,__FILE__ "(%d) : %s : STATUS_NOT_IMPLEMENTED\n",line,func);
}

#define NOT_IMPLEMENTED_R(x) { noimp(__LINE__,__FUNCTION__); return x; }
#define NOT_IMPLEMENTED      { noimp(__LINE__,__FUNCTION__); return STATUS_NOT_IMPLEMENTED; }

/* ReactOS: "nsoskrnl/config/ntapi.c" */
INTERN NTSTATUS NTAPI NtCreateKey(OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG TitleIndex, IN PUNICODE_STRING Class OPTIONAL, IN ULONG CreateOptions, OUT PULONG Disposition OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenKey(OUT PHANDLE KeyHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteKey(IN HANDLE KeyHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtEnumerateKey(IN HANDLE KeyHandle, IN ULONG Index, IN KEY_INFORMATION_CLASS KeyInformationClass, OUT PVOID KeyInformation, IN ULONG Length, OUT PULONG ResultLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtEnumerateValueKey(IN HANDLE KeyHandle, IN ULONG Index, IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryKey(IN HANDLE KeyHandle, IN KEY_INFORMATION_CLASS KeyInformationClass, OUT PVOID KeyInformation, IN ULONG Length, OUT PULONG ResultLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, OUT PVOID KeyValueInformation, IN ULONG Length, OUT PULONG ResultLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName, IN ULONG TitleIndex, IN ULONG Type, IN PVOID Data, IN ULONG DataSize) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteValueKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ValueName) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFlushKey(IN HANDLE KeyHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLoadKey(IN POBJECT_ATTRIBUTES KeyObjectAttributes, IN POBJECT_ATTRIBUTES FileObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLoadKey2(IN POBJECT_ATTRIBUTES KeyObjectAttributes, IN POBJECT_ATTRIBUTES FileObjectAttributes, IN ULONG Flags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLoadKeyEx(IN POBJECT_ATTRIBUTES TargetKey, IN POBJECT_ATTRIBUTES SourceFile, IN ULONG Flags, IN HANDLE TrustClassKey) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtNotifyChangeKey(IN HANDLE KeyHandle, IN HANDLE Event, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG CompletionFilter, IN BOOLEAN WatchTree, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN Asynchronous) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtInitializeRegistry(IN USHORT Flag) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCompactKeys(IN ULONG Count, IN PHANDLE KeyArray) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCompressKey(IN HANDLE Key) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLockProductActivationKeys(IN PULONG pPrivateVer, IN PULONG pSafeMode) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLockRegistryKey(IN HANDLE KeyHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtNotifyChangeMultipleKeys(IN HANDLE MasterKeyHandle, IN ULONG Count, IN POBJECT_ATTRIBUTES SlaveObjects, IN HANDLE Event, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG CompletionFilter, IN BOOLEAN WatchTree, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN Asynchronous) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryMultipleValueKey(IN HANDLE KeyHandle, IN OUT PKEY_VALUE_ENTRY ValueList, IN ULONG NumberOfValues, OUT PVOID Buffer, IN OUT PULONG Length, OUT PULONG ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryOpenSubKeys(IN POBJECT_ATTRIBUTES TargetKey, OUT PULONG HandleCount) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryOpenSubKeysEx(IN POBJECT_ATTRIBUTES TargetKey, IN ULONG BufferLength, IN PVOID Buffer, IN PULONG RequiredSize) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ReplacementName) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReplaceKey(IN POBJECT_ATTRIBUTES ObjectAttributes, IN HANDLE Key, IN POBJECT_ATTRIBUTES ReplacedObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRestoreKey(IN HANDLE KeyHandle, IN HANDLE FileHandle, IN ULONG RestoreFlags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSaveKey(IN HANDLE KeyHandle, IN HANDLE FileHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSaveKeyEx(IN HANDLE KeyHandle, IN HANDLE FileHandle, IN ULONG Flags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSaveMergedKeys(IN HANDLE HighPrecedenceKeyHandle, IN HANDLE LowPrecedenceKeyHandle, IN HANDLE FileHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationKey(IN HANDLE KeyHandle, IN KEY_SET_INFORMATION_CLASS KeyInformationClass, IN PVOID KeyInformation, IN ULONG KeyInformationLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnloadKey(IN POBJECT_ATTRIBUTES KeyObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnloadKey2(IN POBJECT_ATTRIBUTES TargetKey, IN ULONG Flags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnloadKeyEx(IN POBJECT_ATTRIBUTES TargetKey, IN HANDLE Event) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/dbgk/dbgkobj.c" */
INTERN NTSTATUS NTAPI NtCreateDebugObject(OUT PHANDLE DebugHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG Flags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDebugContinue(IN HANDLE DebugHandle, IN PCLIENT_ID AppClientId, IN NTSTATUS ContinueStatus) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDebugActiveProcess(IN HANDLE ProcessHandle, IN HANDLE DebugHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRemoveProcessDebug(IN HANDLE ProcessHandle, IN HANDLE DebugHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationDebugObject(IN HANDLE DebugHandle, IN DEBUGOBJECTINFOCLASS DebugObjectInformationClass, IN PVOID DebugInformation, IN ULONG DebugInformationLength, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWaitForDebugEvent(IN HANDLE DebugHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL, OUT PDBGUI_WAIT_STATE_CHANGE StateChange) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/atom.c" */
INTERN NTSTATUS NTAPI NtAddAtom(IN PWSTR AtomName, IN ULONG AtomNameLength, OUT PRTL_ATOM Atom) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteAtom(IN RTL_ATOM Atom) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFindAtom(IN PWSTR AtomName, IN ULONG AtomNameLength, OUT PRTL_ATOM Atom) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryInformationAtom(RTL_ATOM Atom, ATOM_INFORMATION_CLASS AtomInformationClass, PVOID AtomInformation, ULONG AtomInformationLength, PULONG ReturnLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/dbgctrl.c" */
INTERN NTSTATUS NTAPI NtSystemDebugControl(SYSDBG_COMMAND ControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength, PULONG ReturnLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/efi.c" */
INTERN NTSTATUS NTAPI NtAddBootEntry(IN PBOOT_ENTRY Entry, IN ULONG Id) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAddDriverEntry(IN PEFI_DRIVER_ENTRY Entry, IN ULONG Id) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteBootEntry(IN ULONG Id) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteDriverEntry(IN ULONG Id) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtEnumerateBootEntries(IN PVOID Buffer, IN PULONG BufferLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtEnumerateDriverEntries(IN PVOID Buffer, IN PULONG BufferLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtModifyBootEntry(IN PBOOT_ENTRY BootEntry) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtModifyDriverEntry(IN PEFI_DRIVER_ENTRY DriverEntry) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryBootEntryOrder(IN PULONG Ids, IN PULONG Count) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryDriverEntryOrder(IN PULONG Ids, IN PULONG Count) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryBootOptions(IN PBOOT_OPTIONS BootOptions, IN PULONG BootOptionsLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetBootEntryOrder(IN PULONG Ids, IN PULONG Count) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetDriverEntryOrder(IN PULONG Ids, IN PULONG Count) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetBootOptions(IN PBOOT_OPTIONS BootOptions, IN ULONG FieldsToChange) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtTranslateFilePath(PFILE_PATH InputFilePath, ULONG OutputType, PFILE_PATH OutputFilePath, ULONG OutputFilePathLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/event.c" */
INTERN NTSTATUS NTAPI NtClearEvent(IN HANDLE EventHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateEvent(OUT PHANDLE EventHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL, IN EVENT_TYPE EventType, IN BOOLEAN InitialState) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenEvent(OUT PHANDLE EventHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtPulseEvent(IN HANDLE EventHandle, OUT PLONG PreviousState OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryEvent(IN HANDLE EventHandle, IN EVENT_INFORMATION_CLASS EventInformationClass, OUT PVOID EventInformation, IN ULONG EventInformationLength, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtResetEvent(IN HANDLE EventHandle, OUT PLONG PreviousState OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetEvent(IN HANDLE EventHandle, OUT PLONG PreviousState OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetEventBoostPriority(IN HANDLE EventHandle) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/evtpair.c" */
INTERN NTSTATUS NTAPI NtCreateEventPair(OUT PHANDLE EventPairHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenEventPair(OUT PHANDLE EventPairHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetHighEventPair(IN HANDLE EventPairHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetHighWaitLowEventPair(IN HANDLE EventPairHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetLowEventPair(IN HANDLE EventPairHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetLowWaitHighEventPair(IN HANDLE EventPairHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWaitLowEventPair(IN HANDLE EventPairHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWaitHighEventPair(IN HANDLE EventPairHandle) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/harderr.c" */
INTERN NTSTATUS NTAPI NtRaiseHardError(IN NTSTATUS ErrorStatus, IN ULONG NumberOfParameters, IN ULONG UnicodeStringParameterMask, IN PULONG_PTR Parameters, IN ULONG ValidResponseOptions, OUT PULONG Response) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetDefaultHardErrorPort(IN HANDLE PortHandle) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/keyedevt.c" */
INTERN NTSTATUS NTAPI NtCreateKeyedEvent(_Out_ PHANDLE OutHandle, _In_ ACCESS_MASK AccessMask, _In_ POBJECT_ATTRIBUTES ObjectAttributes, _In_ ULONG Flags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenKeyedEvent(_Out_ PHANDLE OutHandle, _In_ ACCESS_MASK AccessMask, _In_ POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWaitForKeyedEvent(_In_opt_ HANDLE Handle, _In_ PVOID Key, _In_ BOOLEAN Alertable, _In_opt_ PLARGE_INTEGER Timeout) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReleaseKeyedEvent(_In_opt_ HANDLE Handle, _In_ PVOID Key, _In_ BOOLEAN Alertable, _In_opt_ PLARGE_INTEGER Timeout) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/locale.c" */
INTERN NTSTATUS NTAPI NtQueryDefaultLocale(IN BOOLEAN UserProfile, OUT PLCID DefaultLocaleId) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetDefaultLocale(IN BOOLEAN UserProfile, IN LCID DefaultLocaleId) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryInstallUILanguage(OUT LANGID* LanguageId) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryDefaultUILanguage(OUT LANGID* LanguageId) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetDefaultUILanguage(IN LANGID LanguageId) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/mutant.c" */
INTERN NTSTATUS NTAPI NtCreateMutant(OUT PHANDLE MutantHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL, IN BOOLEAN InitialOwner) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenMutant(OUT PHANDLE MutantHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryMutant(IN HANDLE MutantHandle, IN MUTANT_INFORMATION_CLASS MutantInformationClass, OUT PVOID MutantInformation, IN ULONG MutantInformationLength, OUT PULONG ResultLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReleaseMutant(IN HANDLE MutantHandle, IN PLONG PreviousCount OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/profile.c" */
INTERN NTSTATUS NTAPI NtCreateProfile(OUT PHANDLE ProfileHandle, IN HANDLE Process OPTIONAL, IN PVOID RangeBase, IN SIZE_T RangeSize, IN ULONG BucketSize, IN PVOID Buffer, IN ULONG BufferSize, IN KPROFILE_SOURCE ProfileSource, IN KAFFINITY Affinity) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceCounter, OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtStartProfile(IN HANDLE ProfileHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtStopProfile(IN HANDLE ProfileHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryIntervalProfile(IN KPROFILE_SOURCE ProfileSource, OUT PULONG Interval) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetIntervalProfile(IN ULONG Interval, IN KPROFILE_SOURCE Source) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/sem.c" */
INTERN NTSTATUS NTAPI NtCreateSemaphore(OUT PHANDLE SemaphoreHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN LONG InitialCount, IN LONG MaximumCount) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenSemaphore(OUT PHANDLE SemaphoreHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQuerySemaphore(IN HANDLE SemaphoreHandle, IN SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass, OUT PVOID SemaphoreInformation, IN ULONG SemaphoreInformationLength, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReleaseSemaphore(IN HANDLE SemaphoreHandle, IN LONG ReleaseCount, OUT PLONG PreviousCount OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/shutdown.c" */
INTERN NTSTATUS NTAPI NtShutdownSystem(IN SHUTDOWN_ACTION Action) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/sysinfo.c" */
INTERN NTSTATUS NTAPI NtQuerySystemEnvironmentValue(IN PUNICODE_STRING VariableName, OUT PWSTR ValueBuffer, IN ULONG ValueBufferLength, IN OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetSystemEnvironmentValue(IN PUNICODE_STRING VariableName, IN PUNICODE_STRING Value) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtEnumerateSystemEnvironmentValuesEx(IN ULONG InformationClass, IN PVOID Buffer, IN ULONG BufferLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQuerySystemEnvironmentValueEx(IN PUNICODE_STRING VariableName, IN LPGUID VendorGuid, IN PVOID Value, IN OUT PULONG ReturnLength, IN OUT PULONG Attributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetSystemEnvironmentValueEx(IN PUNICODE_STRING VariableName, IN LPGUID VendorGuid, IN PVOID Value, IN OUT PULONG ReturnLength, IN OUT PULONG Attributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, OUT PVOID SystemInformation, IN ULONG Length, OUT PULONG UnsafeResultLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetSystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, IN PVOID SystemInformation, IN ULONG SystemInformationLength) NOT_IMPLEMENTED
INTERN ULONG NTAPI NtGetCurrentProcessorNumber(VOID) NOT_IMPLEMENTED_R(0)

/* ReactOS: "nsoskrnl/ex/time.c" */
INTERN NTSTATUS NTAPI NtSetSystemTime(IN PLARGE_INTEGER SystemTime, OUT PLARGE_INTEGER PreviousTime OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQuerySystemTime(OUT PLARGE_INTEGER SystemTime) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryTimerResolution(OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG ActualResolution) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetTimerResolution(IN ULONG DesiredResolution, IN BOOLEAN SetResolution, OUT PULONG CurrentResolution) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/timer.c" */
INTERN NTSTATUS NTAPI NtCancelTimer(IN HANDLE TimerHandle, OUT PBOOLEAN CurrentState OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateTimer(OUT PHANDLE TimerHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN TIMER_TYPE TimerType) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenTimer(OUT PHANDLE TimerHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryTimer(IN HANDLE TimerHandle, IN TIMER_INFORMATION_CLASS TimerInformationClass, OUT PVOID TimerInformation, IN ULONG TimerInformationLength, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetTimer(IN HANDLE TimerHandle, IN PLARGE_INTEGER DueTime, IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL, IN PVOID TimerContext OPTIONAL, IN BOOLEAN WakeTimer, IN LONG Period OPTIONAL, OUT PBOOLEAN PreviousState OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ex/uuid.c" */
INTERN NTSTATUS NTAPI NtAllocateLocallyUniqueId(OUT LUID *LocallyUniqueId) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAllocateUuids(OUT PULARGE_INTEGER Time, OUT PULONG Range, OUT PULONG Sequence, OUT PUCHAR Seed) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetUuidSeed(IN PUCHAR Seed) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/inbv/inbv.c" */
INTERN NTSTATUS NTAPI NtDisplayString(IN PUNICODE_STRING DisplayString) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/io/iomgr/driver.c" */
INTERN NTSTATUS NTAPI NtLoadDriver(IN PUNICODE_STRING DriverServiceName) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnloadDriver(IN PUNICODE_STRING DriverServiceName) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/io/iomgr/file.c" */
INTERN NTSTATUS NTAPI NtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocateSize, ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateMailslotFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG CreateOptions, IN ULONG MailslotQuota, IN ULONG MaxMessageSize, IN PLARGE_INTEGER TimeOut) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateNamedPipeFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG CreateDisposition, IN ULONG CreateOptions, IN ULONG NamedPipeType, IN ULONG ReadMode, IN ULONG CompletionMode, IN ULONG MaximumInstances, IN ULONG InboundQuota, IN ULONG OutboundQuota, IN PLARGE_INTEGER DefaultTimeout) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFlushWriteBuffer(VOID) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryAttributesFile(IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PFILE_BASIC_INFORMATION FileInformation) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryFullAttributesFile(IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCancelIoFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteFile(IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/io/iomgr/iocomp.c" */
INTERN NTSTATUS NTAPI NtCreateIoCompletion(OUT PHANDLE IoCompletionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG NumberOfConcurrentThreads) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenIoCompletion(OUT PHANDLE IoCompletionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryIoCompletion(IN HANDLE IoCompletionHandle, IN IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass, OUT PVOID IoCompletionInformation, IN ULONG IoCompletionInformationLength, OUT PULONG ResultLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRemoveIoCompletion(IN HANDLE IoCompletionHandle, OUT PVOID *KeyContext, OUT PVOID *ApcContext, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PLARGE_INTEGER Timeout OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetIoCompletion(IN HANDLE IoCompletionPortHandle, IN PVOID CompletionKey, IN PVOID CompletionContext, IN NTSTATUS CompletionStatus, IN ULONG CompletionInformation) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/io/iomgr/iofunc.c" */
INTERN NTSTATUS NTAPI NtDeviceIoControlFile(IN HANDLE DeviceHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG IoControlCode, IN PVOID InputBuffer, IN ULONG InputBufferLength OPTIONAL, OUT PVOID OutputBuffer, IN ULONG OutputBufferLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFsControlFile(IN HANDLE DeviceHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG IoControlCode, IN PVOID InputBuffer, IN ULONG InputBufferLength OPTIONAL, OUT PVOID OutputBuffer, IN ULONG OutputBufferLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFlushBuffersFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtNotifyChangeDirectoryFile(IN HANDLE FileHandle, IN HANDLE EventHandle OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG BufferSize, IN ULONG CompletionFilter, IN BOOLEAN WatchTree) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLockFile(IN HANDLE FileHandle, IN HANDLE EventHandle OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PLARGE_INTEGER ByteOffset, IN PLARGE_INTEGER Length, IN ULONG Key, IN BOOLEAN FailImmediately, IN BOOLEAN ExclusiveLock) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryDirectoryFile(IN HANDLE FileHandle, IN HANDLE EventHandle OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass, IN BOOLEAN ReturnSingleEntry, IN PUNICODE_STRING FileName OPTIONAL, IN BOOLEAN RestartScan) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryEaFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN ReturnSingleEntry, IN PVOID EaList OPTIONAL, IN ULONG EaListLength, IN PULONG EaIndex OPTIONAL, IN BOOLEAN RestartScan) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryQuotaInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG Length, IN BOOLEAN ReturnSingleEntry, IN PVOID SidList OPTIONAL, IN ULONG SidListLength, IN PSID StartSid OPTIONAL, IN BOOLEAN RestartScan) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReadFile(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID Buffer, IN ULONG Length, IN PLARGE_INTEGER ByteOffset OPTIONAL, IN PULONG Key OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReadFileScatter(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK UserIoStatusBlock, IN FILE_SEGMENT_ELEMENT BufferDescription[], IN ULONG BufferLength, IN PLARGE_INTEGER ByteOffset, IN PULONG Key OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetEaFile(IN HANDLE FileHandle, IN PIO_STATUS_BLOCK IoStatusBlock, IN PVOID EaBuffer, IN ULONG EaBufferSize) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID FileInformation, IN ULONG Length, IN FILE_INFORMATION_CLASS FileInformationClass) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetQuotaInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID Buffer, IN ULONG BufferLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnlockFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PLARGE_INTEGER ByteOffset, IN PLARGE_INTEGER Length, IN ULONG Key OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWriteFile(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE ApcRoutine OPTIONAL, IN PVOID ApcContext OPTIONAL, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID Buffer, IN ULONG Length, IN PLARGE_INTEGER ByteOffset OPTIONAL, IN PULONG Key OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWriteFileGather(IN HANDLE FileHandle, IN HANDLE Event OPTIONAL, IN PIO_APC_ROUTINE UserApcRoutine OPTIONAL, IN PVOID UserApcContext OPTIONAL, OUT PIO_STATUS_BLOCK UserIoStatusBlock, IN FILE_SEGMENT_ELEMENT BufferDescription[], IN ULONG BufferLength, IN PLARGE_INTEGER ByteOffset, IN PULONG Key OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryVolumeInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, OUT PVOID FsInformation, IN ULONG Length, IN FS_INFORMATION_CLASS FsInformationClass) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetVolumeInformationFile(IN HANDLE FileHandle, OUT PIO_STATUS_BLOCK IoStatusBlock, IN PVOID FsInformation, IN ULONG Length, IN FS_INFORMATION_CLASS FsInformationClass) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCancelDeviceWakeupRequest(IN HANDLE DeviceHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRequestDeviceWakeup(IN HANDLE DeviceHandle) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/io/pnpmgr/plugplay.c" */
INTERN NTSTATUS NTAPI NtGetPlugPlayEvent(IN ULONG Reserved1, IN ULONG Reserved2, OUT PPLUGPLAY_EVENT_BLOCK Buffer, IN ULONG BufferSize) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtPlugPlayControl(IN PLUGPLAY_CONTROL_CLASS PlugPlayControlClass, IN OUT PVOID Buffer, IN ULONG BufferLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/kd/kdmain.c" */
INTERN NTSTATUS NTAPI NtQueryDebugFilterState(IN ULONG ComponentId, IN ULONG Level) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetDebugFilterState(IN ULONG ComponentId, IN ULONG Level, IN BOOLEAN State) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ke/thrdschd.c" */
INTERN NTSTATUS NTAPI NtYieldExecution(VOID) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ke/wait.c" */
INTERN NTSTATUS NTAPI NtDelayExecution(IN BOOLEAN Alertable, IN PLARGE_INTEGER DelayInterval) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ke/except.c" */
INTERN NTSTATUS NTAPI NtRaiseException(IN PEXCEPTION_RECORD ExceptionRecord, IN PCONTEXT Context, IN BOOLEAN FirstChance) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtContinue(IN PCONTEXT Context, IN BOOLEAN TestAlert) NOT_IMPLEMENTED

#if defined(__i386__) || defined(__x86_64__)
/* ReactOS: "nsoskrnl/ke/i386/ldt.c" */
INTERN NTSTATUS NTAPI NtSetLdtEntries(ULONG Selector1, LDT_ENTRY LdtEntry1, ULONG Selector2, LDT_ENTRY LdtEntry2) NOT_IMPLEMENTED
#endif

/* ReactOS: "nsoskrnl/ke/i386/usercall.c" */
INTERN NTSTATUS NTAPI NtCallbackReturn(_In_ PVOID Result, _In_ ULONG ResultLength, _In_ NTSTATUS CallbackStatus) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ke/powerpc/stubs.c" */
INTERN ULONG NTAPI NtGetTickCount(void) NOT_IMPLEMENTED_R(0)

/* ReactOS: "nsoskrnl/lpc/connect.c" */
INTERN NTSTATUS NTAPI NtSecureConnectPort(OUT PHANDLE PortHandle, IN PUNICODE_STRING PortName, IN PSECURITY_QUALITY_OF_SERVICE SecurityQos, IN OUT PPORT_VIEW ClientView OPTIONAL, IN PSID ServerSid OPTIONAL, IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL, OUT PULONG MaxMessageLength OPTIONAL, IN OUT PVOID ConnectionInformation OPTIONAL, IN OUT PULONG ConnectionInformationLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtConnectPort(OUT PHANDLE PortHandle, IN PUNICODE_STRING PortName, IN PSECURITY_QUALITY_OF_SERVICE SecurityQos, IN OUT PPORT_VIEW ClientView OPTIONAL, IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL, OUT PULONG MaxMessageLength OPTIONAL, IN OUT PVOID ConnectionInformation OPTIONAL, IN OUT PULONG ConnectionInformationLength OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/lpc/create.c" */
INTERN NTSTATUS NTAPI NtCreatePort(OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectInfoLength, IN ULONG MaxDataLength, IN ULONG MaxPoolUsage) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateWaitablePort(OUT PHANDLE PortHandle, IN POBJECT_ATTRIBUTES ObjectAttributes, IN ULONG MaxConnectInfoLength, IN ULONG MaxDataLength, IN ULONG MaxPoolUsage) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/lpc/listen.c" */
INTERN NTSTATUS NTAPI NtListenPort(IN HANDLE PortHandle, OUT PPORT_MESSAGE ConnectMessage) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/lpc/port.c" */
INTERN NTSTATUS NTAPI NtImpersonateClientOfPort(IN HANDLE PortHandle, IN PPORT_MESSAGE ClientMessage) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryPortInformationProcess(VOID) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryInformationPort(IN HANDLE PortHandle, IN PORT_INFORMATION_CLASS PortInformationClass, OUT PVOID PortInformation, IN ULONG PortInformationLength, OUT PULONG ReturnLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/lpc/reply.c" */
INTERN NTSTATUS NTAPI NtReplyPort(IN HANDLE PortHandle, IN PPORT_MESSAGE ReplyMessage) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReplyWaitReceivePortEx(IN HANDLE PortHandle, OUT PVOID *PortContext OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE ReceiveMessage, IN PLARGE_INTEGER Timeout OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReplyWaitReceivePort(IN HANDLE PortHandle, OUT PVOID *PortContext OPTIONAL, IN PPORT_MESSAGE ReplyMessage OPTIONAL, OUT PPORT_MESSAGE ReceiveMessage) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReplyWaitReplyPort(IN HANDLE PortHandle, IN PPORT_MESSAGE ReplyMessage) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtReadRequestData(IN HANDLE PortHandle, IN PPORT_MESSAGE Message, IN ULONG Index, IN PVOID Buffer, IN ULONG BufferLength, OUT PULONG ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWriteRequestData(IN HANDLE PortHandle, IN PPORT_MESSAGE Message, IN ULONG Index, IN PVOID Buffer, IN ULONG BufferLength, OUT PULONG ReturnLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/lpc/send.c" */
INTERN NTSTATUS NTAPI NtRequestPort(IN HANDLE PortHandle, IN PPORT_MESSAGE LpcRequest) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRequestWaitReplyPort(IN HANDLE PortHandle, IN PPORT_MESSAGE LpcRequest, IN OUT PPORT_MESSAGE LpcReply) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/lpc/complete.c" */
INTERN NTSTATUS NTAPI NtAcceptConnectPort(OUT PHANDLE PortHandle, IN PVOID PortContext OPTIONAL, IN PPORT_MESSAGE ReplyMessage, IN BOOLEAN AcceptConnection, IN OUT PPORT_VIEW ServerView OPTIONAL, OUT PREMOTE_PORT_VIEW ClientView OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCompleteConnectPort(IN HANDLE PortHandle) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/mm/pagefile.c" */
INTERN NTSTATUS NTAPI NtCreatePagingFile(IN PUNICODE_STRING FileName, IN PLARGE_INTEGER InitialSize, IN PLARGE_INTEGER MaximumSize, IN ULONG Reserved) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/mm/section.c" */
INTERN NTSTATUS NTAPI NtQuerySection(_In_ HANDLE SectionHandle, _In_ SECTION_INFORMATION_CLASS SectionInformationClass, _Out_ PVOID SectionInformation, _In_ SIZE_T SectionInformationLength, _Out_opt_ PSIZE_T ResultLength) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/mm/ARM3/procsup.c" */
INTERN NTSTATUS NTAPI NtAllocateUserPhysicalPages(IN HANDLE ProcessHandle, IN OUT PULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtMapUserPhysicalPages(IN PVOID VirtualAddresses, IN ULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtMapUserPhysicalPagesScatter(IN PVOID *VirtualAddresses, IN ULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFreeUserPhysicalPages(IN HANDLE ProcessHandle, IN OUT PULONG_PTR NumberOfPages, IN OUT PULONG_PTR UserPfnArray) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/mm/ARM3/section.c" */
INTERN NTSTATUS NTAPI NtAreMappedFilesTheSame(IN PVOID File1MappedAsAnImage, IN PVOID File2MappedAsFile) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateSection(OUT PHANDLE SectionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN PLARGE_INTEGER MaximumSize OPTIONAL, IN ULONG SectionPageProtection OPTIONAL, IN ULONG AllocationAttributes, IN HANDLE FileHandle OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenSection(OUT PHANDLE SectionHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtMapViewOfSection(IN HANDLE SectionHandle, IN HANDLE ProcessHandle, IN OUT PVOID* BaseAddress, IN ULONG_PTR ZeroBits, IN SIZE_T CommitSize, IN OUT PLARGE_INTEGER SectionOffset OPTIONAL, IN OUT PSIZE_T ViewSize, IN SECTION_INHERIT InheritDisposition, IN ULONG AllocationType, IN ULONG Protect) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnmapViewOfSection(IN HANDLE ProcessHandle, IN PVOID BaseAddress) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtExtendSection(IN HANDLE SectionHandle, IN OUT PLARGE_INTEGER NewMaximumSize) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/mm/ARM3/virtual.c" */
INTERN NTSTATUS NTAPI NtReadVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, OUT PVOID Buffer, IN SIZE_T NumberOfBytesToRead, OUT PSIZE_T NumberOfBytesRead OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWriteVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, IN PVOID Buffer, IN SIZE_T NumberOfBytesToWrite, OUT PSIZE_T NumberOfBytesWritten OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFlushInstructionCache(_In_ HANDLE ProcessHandle, _In_opt_ PVOID BaseAddress, _In_ SIZE_T FlushSize) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtProtectVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *UnsafeBaseAddress, IN OUT SIZE_T *UnsafeNumberOfBytesToProtect, IN ULONG NewAccessProtection, OUT PULONG UnsafeOldAccessProtection) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtLockVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T NumberOfBytesToLock, IN ULONG MapType) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtUnlockVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T NumberOfBytesToUnlock, IN ULONG MapType) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFlushVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN OUT PSIZE_T NumberOfBytesToFlush, OUT PIO_STATUS_BLOCK IoStatusBlock) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtGetWriteWatch(IN HANDLE ProcessHandle, IN ULONG Flags, IN PVOID BaseAddress, IN SIZE_T RegionSize, IN PVOID *UserAddressArray, OUT PULONG_PTR EntriesInUserAddressArray, OUT PULONG Granularity) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtResetWriteWatch(IN HANDLE ProcessHandle, IN PVOID BaseAddress, IN SIZE_T RegionSize) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, IN MEMORY_INFORMATION_CLASS MemoryInformationClass, OUT PVOID MemoryInformation, IN SIZE_T MemoryInformationLength, OUT PSIZE_T ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAllocateVirtualMemory(IN HANDLE ProcessHandle, IN OUT PVOID* UBaseAddress, IN ULONG_PTR ZeroBits, IN OUT PSIZE_T URegionSize, IN ULONG AllocationType, IN ULONG Protect) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFreeVirtualMemory(IN HANDLE ProcessHandle, IN PVOID* UBaseAddress, IN PSIZE_T URegionSize, IN ULONG FreeType) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ob/oblife.c" */
INTERN NTSTATUS NTAPI NtMakeTemporaryObject(IN HANDLE ObjectHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtMakePermanentObject(IN HANDLE ObjectHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryObject(IN HANDLE ObjectHandle, IN OBJECT_INFORMATION_CLASS ObjectInformationClass, OUT PVOID ObjectInformation, IN ULONG Length, OUT PULONG ResultLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationObject(IN HANDLE ObjectHandle, IN OBJECT_INFORMATION_CLASS ObjectInformationClass, IN PVOID ObjectInformation, IN ULONG Length) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ob/oblink.c" */
INTERN NTSTATUS NTAPI NtCreateSymbolicLinkObject(OUT PHANDLE LinkHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PUNICODE_STRING LinkTarget) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenSymbolicLinkObject(OUT PHANDLE LinkHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQuerySymbolicLinkObject(IN HANDLE LinkHandle, OUT PUNICODE_STRING LinkTarget, OUT PULONG ResultLength OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ob/obsecure.c" */
INTERN NTSTATUS NTAPI NtQuerySecurityObject(IN HANDLE Handle, IN SECURITY_INFORMATION SecurityInformation, OUT PSECURITY_DESCRIPTOR SecurityDescriptor, IN ULONG Length, OUT PULONG ResultLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetSecurityObject(IN HANDLE Handle, IN SECURITY_INFORMATION SecurityInformation, IN PSECURITY_DESCRIPTOR SecurityDescriptor) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ob/obwait.c" */
INTERN NTSTATUS NTAPI NtWaitForMultipleObjects(IN ULONG ObjectCount, IN PHANDLE HandleArray, IN WAIT_TYPE WaitType, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWaitForMultipleObjects32(IN ULONG ObjectCount, IN PLONG Handles, IN WAIT_TYPE WaitType, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtWaitForSingleObject(IN HANDLE ObjectHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSignalAndWaitForSingleObject(IN HANDLE ObjectHandleToSignal, IN HANDLE WaitableObjectHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ob/obdir.c" */
INTERN NTSTATUS NTAPI NtOpenDirectoryObject(OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryDirectoryObject(IN HANDLE DirectoryHandle, OUT PVOID Buffer, IN ULONG BufferLength, IN BOOLEAN ReturnSingleEntry, IN BOOLEAN RestartScan, IN OUT PULONG Context, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateDirectoryObject(OUT PHANDLE DirectoryHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ob/obhandle.c" */
INTERN NTSTATUS NTAPI NtClose(IN HANDLE Handle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDuplicateObject(IN HANDLE SourceProcessHandle, IN HANDLE SourceHandle, IN HANDLE TargetProcessHandle OPTIONAL, OUT PHANDLE TargetHandle OPTIONAL, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, IN ULONG Options) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/po/power.c" */
INTERN NTSTATUS NTAPI NtInitiatePowerAction(IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG Flags, IN BOOLEAN Asynchronous) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtPowerInformation(IN POWER_INFORMATION_LEVEL PowerInformationLevel, IN PVOID InputBuffer OPTIONAL, IN ULONG InputBufferLength, OUT PVOID OutputBuffer OPTIONAL, IN ULONG OutputBufferLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtGetDevicePowerState(IN HANDLE Device, IN PDEVICE_POWER_STATE PowerState) NOT_IMPLEMENTED
INTERN BOOLEAN NTAPI NtIsSystemResumeAutomatic(VOID) NOT_IMPLEMENTED_R(FALSE)
INTERN NTSTATUS NTAPI NtRequestWakeupLatency(IN LATENCY_TIME Latency) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetThreadExecutionState(IN EXECUTION_STATE esFlags, OUT EXECUTION_STATE *PreviousFlags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetSystemPowerState(IN POWER_ACTION SystemAction, IN SYSTEM_POWER_STATE MinSystemState, IN ULONG Flags) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/process.c" */
INTERN NTSTATUS NTAPI NtCreateProcessEx(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ParentProcess, IN ULONG Flags, IN HANDLE SectionHandle OPTIONAL, IN HANDLE DebugPort OPTIONAL, IN HANDLE ExceptionPort OPTIONAL, IN BOOLEAN InJob) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateProcess(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ParentProcess, IN BOOLEAN InheritObjectTable, IN HANDLE SectionHandle OPTIONAL, IN HANDLE DebugPort OPTIONAL, IN HANDLE ExceptionPort OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenProcess(OUT PHANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/query.c" */
INTERN NTSTATUS NTAPI NtQueryInformationProcess(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationProcess(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, IN PVOID ProcessInformation, IN ULONG ProcessInformationLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationThread(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, IN PVOID ThreadInformation, IN ULONG ThreadInformationLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryInformationThread(IN HANDLE ThreadHandle, IN THREADINFOCLASS ThreadInformationClass, OUT PVOID ThreadInformation, IN ULONG ThreadInformationLength, OUT PULONG ReturnLength OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/security.c" */
INTERN NTSTATUS NTAPI NtOpenProcessToken(IN HANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, OUT PHANDLE TokenHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenProcessTokenEx(IN HANDLE ProcessHandle, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, OUT PHANDLE TokenHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtImpersonateThread(IN HANDLE ThreadHandle, IN HANDLE ThreadToImpersonateHandle, IN PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/state.c" */
INTERN NTSTATUS NTAPI NtAlertThread(IN HANDLE ThreadHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAlertResumeThread(IN HANDLE ThreadHandle, OUT PULONG SuspendCount) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtResumeThread(IN HANDLE ThreadHandle, OUT PULONG SuspendCount OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSuspendThread(IN HANDLE ThreadHandle, OUT PULONG PreviousSuspendCount OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSuspendProcess(IN HANDLE ProcessHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtResumeProcess(IN HANDLE ProcessHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtTestAlert(VOID) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueueApcThread(IN HANDLE ThreadHandle, IN PKNORMAL_ROUTINE ApcRoutine, IN PVOID NormalContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/thread.c" */
INTERN NTSTATUS NTAPI NtCreateThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN HANDLE ProcessHandle, OUT PCLIENT_ID ClientId, IN PCONTEXT ThreadContext, IN PINITIAL_TEB InitialTeb, IN BOOLEAN CreateSuspended) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenThread(OUT PHANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, IN PCLIENT_ID ClientId OPTIONAL) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/apphelp.c" */
INTERN NTSTATUS NTAPI NtApphelpCacheControl(_In_ APPHELPCACHESERVICECLASS Service, _In_opt_ PAPPHELP_CACHE_SERVICE_LOOKUP ServiceData) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/debug.c" */
INTERN NTSTATUS NTAPI NtGetContextThread(IN HANDLE ThreadHandle, IN OUT PCONTEXT ThreadContext) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetContextThread(IN HANDLE ThreadHandle, IN PCONTEXT ThreadContext) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/job.c" */
INTERN NTSTATUS NTAPI NtAssignProcessToJobObject(HANDLE JobHandle, HANDLE ProcessHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateJobSet(IN ULONG NumJob, IN PJOB_SET_ARRAY UserJobSet, IN ULONG Flags) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateJobObject(PHANDLE JobHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtIsProcessInJob(IN HANDLE ProcessHandle, IN HANDLE JobHandle OPTIONAL) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenJobObject(PHANDLE JobHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtQueryInformationJobObject(HANDLE JobHandle, JOBOBJECTINFOCLASS JobInformationClass, PVOID JobInformation, ULONG JobInformationLength, PULONG ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationJobObject(HANDLE JobHandle, JOBOBJECTINFOCLASS JobInformationClass, PVOID JobInformation, ULONG JobInformationLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtTerminateJobObject(HANDLE JobHandle, NTSTATUS ExitStatus) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/ps/kill.c" */
INTERN NTSTATUS NTAPI NtTerminateProcess(IN HANDLE ProcessHandle OPTIONAL, IN NTSTATUS ExitStatus) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtTerminateThread(IN HANDLE ThreadHandle, IN NTSTATUS ExitStatus) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtRegisterThreadTerminatePort(IN HANDLE PortHandle) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/se/accesschk.c" */
INTERN NTSTATUS NTAPI NtAccessCheck(IN PSECURITY_DESCRIPTOR SecurityDescriptor, IN HANDLE TokenHandle, IN ACCESS_MASK DesiredAccess, IN PGENERIC_MAPPING GenericMapping, OUT PPRIVILEGE_SET PrivilegeSet OPTIONAL, IN OUT PULONG PrivilegeSetLength, OUT PACCESS_MASK GrantedAccess, OUT PNTSTATUS AccessStatus) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAccessCheckByType(IN PSECURITY_DESCRIPTOR SecurityDescriptor, IN PSID PrincipalSelfSid, IN HANDLE ClientToken, IN ACCESS_MASK DesiredAccess, IN POBJECT_TYPE_LIST ObjectTypeList, IN ULONG ObjectTypeLength, IN PGENERIC_MAPPING GenericMapping, IN PPRIVILEGE_SET PrivilegeSet, IN OUT PULONG PrivilegeSetLength, OUT PACCESS_MASK GrantedAccess, OUT PNTSTATUS AccessStatus) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAccessCheckByTypeResultList(IN PSECURITY_DESCRIPTOR SecurityDescriptor, IN PSID PrincipalSelfSid, IN HANDLE ClientToken, IN ACCESS_MASK DesiredAccess, IN POBJECT_TYPE_LIST ObjectTypeList, IN ULONG ObjectTypeLength, IN PGENERIC_MAPPING GenericMapping, IN PPRIVILEGE_SET PrivilegeSet, IN OUT PULONG PrivilegeSetLength, OUT PACCESS_MASK GrantedAccess, OUT PNTSTATUS AccessStatus) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/se/audit.c" */
INTERN NTSTATUS NTAPI NtCloseObjectAuditAlarm(PUNICODE_STRING SubsystemName, PVOID HandleId, BOOLEAN GenerateOnClose) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDeleteObjectAuditAlarm(IN PUNICODE_STRING SubsystemName, IN PVOID HandleId, IN BOOLEAN GenerateOnClose) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenObjectAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_opt_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_ HANDLE ClientTokenHandle, _In_ ACCESS_MASK DesiredAccess, _In_ ACCESS_MASK GrantedAccess, _In_opt_ PPRIVILEGE_SET PrivilegeSet, _In_ BOOLEAN ObjectCreation, _In_ BOOLEAN AccessGranted, _Out_ PBOOLEAN GenerateOnClose) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtPrivilegedServiceAuditAlarm(_In_opt_ PUNICODE_STRING SubsystemName, _In_opt_ PUNICODE_STRING ServiceName, _In_ HANDLE ClientTokenHandle, _In_ PPRIVILEGE_SET Privileges, _In_ BOOLEAN AccessGranted) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtPrivilegeObjectAuditAlarm(IN PUNICODE_STRING SubsystemName, IN PVOID HandleId, IN HANDLE ClientToken, IN ULONG DesiredAccess, IN PPRIVILEGE_SET Privileges, IN BOOLEAN AccessGranted) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAccessCheckAndAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_ ACCESS_MASK DesiredAccess, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_ PACCESS_MASK GrantedAccess, _Out_ PNTSTATUS AccessStatus, _Out_ PBOOLEAN GenerateOnClose) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAccessCheckByTypeAndAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID PrincipalSelfSid, _In_ ACCESS_MASK DesiredAccess, _In_ AUDIT_EVENT_TYPE AuditType, _In_ ULONG Flags, _In_reads_opt_(ObjectTypeLength) POBJECT_TYPE_LIST ObjectTypeList, _In_ ULONG ObjectTypeLength, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_ PACCESS_MASK GrantedAccess, _Out_ PNTSTATUS AccessStatus, _Out_ PBOOLEAN GenerateOnClose) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAccessCheckByTypeResultListAndAuditAlarm(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID PrincipalSelfSid, _In_ ACCESS_MASK DesiredAccess, _In_ AUDIT_EVENT_TYPE AuditType, _In_ ULONG Flags, _In_reads_opt_(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList, _In_ ULONG ObjectTypeListLength, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_writes_(ObjectTypeListLength) PACCESS_MASK GrantedAccessList, _Out_writes_(ObjectTypeListLength) PNTSTATUS AccessStatusList, _Out_ PBOOLEAN GenerateOnClose) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAccessCheckByTypeResultListAndAuditAlarmByHandle(_In_ PUNICODE_STRING SubsystemName, _In_opt_ PVOID HandleId, _In_ HANDLE ClientToken, _In_ PUNICODE_STRING ObjectTypeName, _In_ PUNICODE_STRING ObjectName, _In_ PSECURITY_DESCRIPTOR SecurityDescriptor, _In_opt_ PSID PrincipalSelfSid, _In_ ACCESS_MASK DesiredAccess, _In_ AUDIT_EVENT_TYPE AuditType, _In_ ULONG Flags, _In_reads_opt_(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList, _In_ ULONG ObjectTypeListLength, _In_ PGENERIC_MAPPING GenericMapping, _In_ BOOLEAN ObjectCreation, _Out_writes_(ObjectTypeListLength) PACCESS_MASK GrantedAccessList, _Out_writes_(ObjectTypeListLength) PNTSTATUS AccessStatusList, _Out_ PBOOLEAN GenerateOnClose) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/se/priv.c" */
INTERN NTSTATUS NTAPI NtPrivilegeCheck(IN HANDLE ClientToken, IN PPRIVILEGE_SET RequiredPrivileges, OUT PBOOLEAN Result) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/se/token.c" */
INTERN NTSTATUS NTAPI NtQueryInformationToken(IN HANDLE TokenHandle, IN TOKEN_INFORMATION_CLASS TokenInformationClass, OUT PVOID TokenInformation, IN ULONG TokenInformationLength, OUT PULONG ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtSetInformationToken(IN HANDLE TokenHandle, IN TOKEN_INFORMATION_CLASS TokenInformationClass, IN PVOID TokenInformation, IN ULONG TokenInformationLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtDuplicateToken(IN HANDLE ExistingTokenHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL, IN BOOLEAN EffectiveOnly, IN TOKEN_TYPE TokenType, OUT PHANDLE NewTokenHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAdjustGroupsToken(IN HANDLE TokenHandle, IN BOOLEAN ResetToDefault, IN PTOKEN_GROUPS NewState, IN ULONG BufferLength, OUT PTOKEN_GROUPS PreviousState OPTIONAL, OUT PULONG ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtAdjustPrivilegesToken(_In_ HANDLE TokenHandle, _In_ BOOLEAN DisableAllPrivileges, _In_opt_ PTOKEN_PRIVILEGES NewState, _In_ ULONG BufferLength, _Out_writes_bytes_to_opt_(BufferLength,*ReturnLength) PTOKEN_PRIVILEGES PreviousState, _When_(PreviousState!=NULL, _Out_) PULONG ReturnLength) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCreateToken(_Out_ PHANDLE TokenHandle, _In_ ACCESS_MASK DesiredAccess, _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes, _In_ TOKEN_TYPE TokenType, _In_ PLUID AuthenticationId, _In_ PLARGE_INTEGER ExpirationTime, _In_ PTOKEN_USER TokenUser, _In_ PTOKEN_GROUPS TokenGroups, _In_ PTOKEN_PRIVILEGES TokenPrivileges, _In_opt_ PTOKEN_OWNER TokenOwner, _In_ PTOKEN_PRIMARY_GROUP TokenPrimaryGroup, _In_opt_ PTOKEN_DEFAULT_DACL TokenDefaultDacl, _In_ PTOKEN_SOURCE TokenSource) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenThreadTokenEx(IN HANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN BOOLEAN OpenAsSelf, IN ULONG HandleAttributes, OUT PHANDLE TokenHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtOpenThreadToken(IN HANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN BOOLEAN OpenAsSelf, OUT PHANDLE TokenHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtCompareTokens(IN HANDLE FirstTokenHandle, IN HANDLE SecondTokenHandle, OUT PBOOLEAN Equal) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtFilterToken(IN HANDLE ExistingTokenHandle, IN ULONG Flags, IN PTOKEN_GROUPS SidsToDisable OPTIONAL, IN PTOKEN_PRIVILEGES PrivilegesToDelete OPTIONAL, IN PTOKEN_GROUPS RestrictedSids OPTIONAL, OUT PHANDLE NewTokenHandle) NOT_IMPLEMENTED
INTERN NTSTATUS NTAPI NtImpersonateAnonymousToken(IN HANDLE Thread) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/vdm/vdmmain.c" */
INTERN NTSTATUS NTAPI NtVdmControl(IN ULONG ControlCode, IN PVOID ControlData) NOT_IMPLEMENTED

/* ReactOS: "nsoskrnl/wmi/wmi.c" */
INTERN NTSTATUS NTAPI NtTraceEvent(IN ULONG TraceHandle, IN ULONG Flags, IN ULONG TraceHeaderLength, IN struct _EVENT_TRACE_HEADER* TraceHeader) NOT_IMPLEMENTED

DECL_END

#endif /* !GUARD_MODULES_NT_API_C */
