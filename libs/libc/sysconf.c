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
#ifndef GUARD_LIBS_LIBC_SYSCONF_C
#define GUARD_LIBS_LIBC_SYSCONF_C 1
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE   1
#define _KOS_SOURCE     1

#include "libc.h"
#include "system.h"
#include "sysconf.h"

#include <linux/limits.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>

DECL_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
PRIVATE long int const sysconf_values[_SC_COUNT] = {
    [0 ... _SC_COUNT-1] = -1,
/*[[[deemon
function opt(sc,name) {
    print "#ifdef",name;
    print "    [{}] = {},".format({ sc,name });
    print "#endif /" "* !"+name,"*" "/";
}
opt("_SC_ARG_MAX","ARG_MAX");
opt("_SC_CHILD_MAX","CHILD_MAX");
opt("_SC_NGROUPS_MAX","NGROUPS_MAX");
opt("_SC_JOB_CONTROL","_POSIX_JOB_CONTROL");
opt("_SC_VERSION","_POSIX_VERSION");
opt("_SC_AIO_LISTIO_MAX","AIO_LISTIO_MAX");
opt("_SC_AIO_MAX","AIO_MAX");
opt("_SC_AIO_PRIO_DELTA_MAX","AIO_PRIO_DELTA_MAX");
opt("_SC_DELAYTIMER_MAX","DELAYTIMER_MAX");
opt("_SC_MQ_OPEN_MAX","MQ_OPEN_MAX");
opt("_SC_MQ_PRIO_MAX","MQ_PRIO_MAX");
opt("_SC_RTSIG_MAX","RTSIG_MAX");
opt("_SC_SEM_NSEMS_MAX","SEM_NSEMS_MAX");
opt("_SC_SEM_VALUE_MAX","SEM_VALUE_MAX");
opt("_SC_SIGQUEUE_MAX","SIGQUEUE_MAX");
opt("_SC_TIMER_MAX","TIMER_MAX");
opt("_SC_BC_BASE_MAX","BC_BASE_MAX");
opt("_SC_BC_DIM_MAX","BC_DIM_MAX");
opt("_SC_BC_SCALE_MAX","BC_SCALE_MAX");
opt("_SC_BC_STRING_MAX","BC_STRING_MAX");
opt("_SC_COLL_WEIGHTS_MAX","COLL_WEIGHTS_MAX");
opt("_SC_EXPR_NEST_MAX","EXPR_NEST_MAX");
opt("_SC_LINE_MAX","LINE_MAX");
opt("_SC_RE_DUP_MAX","RE_DUP_MAX");
opt("_SC_CHARCLASS_NAME_MAX","CHARCLASS_NAME_MAX");
opt("_SC_UIO_MAXIOV","UIO_MAXIOV");
opt("_SC_T_IOV_MAX","_T_IOV_MAX");
opt("_SC_2_VERSION","_POSIX2_VERSION");
opt("_SC_2_C_BIND","_POSIX2_C_BIND");
opt("_SC_2_C_DEV","_POSIX2_C_DEV");
opt("_SC_2_C_VERSION","_POSIX2_C_VERSION");
opt("_SC_2_FORT_DEV","_POSIX2_FORT_DEV");
opt("_SC_2_FORT_RUN","_POSIX2_FORT_RUN");
opt("_SC_2_LOCALEDEF","_POSIX2_LOCALEDEF");
opt("_SC_2_SW_DEV","_POSIX2_SW_DEV");
opt("_SC_2_CHAR_TERM","_POSIX2_CHAR_TERM");
opt("_SC_2_UPE","_POSIX2_UPE");
opt("_SC_GETGR_R_SIZE_MAX","NSS_BUFLEN_GROUP");
opt("_SC_GETPW_R_SIZE_MAX","NSS_BUFLEN_PASSWD");
opt("_SC_LOGIN_NAME_MAX","LOGIN_NAME_MAX");
opt("_SC_TTY_NAME_MAX","TTY_NAME_MAX");
opt("_SC_ATEXIT_MAX","INT_MAX");
opt("_SC_PASS_MAX","BUFSIZ");
opt("_SC_XOPEN_VERSION","_XOPEN_VERSION");
opt("_SC_XOPEN_XCU_VERSION","_XOPEN_XCU_VERSION");
opt("_SC_XOPEN_UNIX","_XOPEN_UNIX");
opt("_SC_XOPEN_CRYPT","_XOPEN_CRYPT");
opt("_SC_XOPEN_ENH_I18N","_XOPEN_ENH_I18N");
opt("_SC_XOPEN_SHM","_XOPEN_SHM");
opt("_SC_XOPEN_XPG2","_XOPEN_XPG2");
opt("_SC_XOPEN_XPG3","_XOPEN_XPG3");
opt("_SC_XOPEN_XPG4","_XOPEN_XPG4");
opt("_SC_NL_ARGMAX","NL_ARGMAX");
opt("_SC_NL_LANGMAX","NL_LANGMAX");
opt("_SC_NL_MSGMAX","NL_MSGMAX");
opt("_SC_NL_NMAX","NL_NMAX");
opt("_SC_NL_SETMAX","NL_SETMAX");
opt("_SC_NL_TEXTMAX","NL_TEXTMAX");
opt("_SC_XOPEN_LEGACY","_XOPEN_LEGACY");
opt("_SC_XOPEN_REALTIME","_XOPEN_REALTIME");
opt("_SC_XOPEN_REALTIME_THREADS","_XOPEN_REALTIME_THREADS");
opt("_SC_2_PBS","_POSIX2_PBS");
opt("_SC_2_PBS_ACCOUNTING","_POSIX2_PBS_ACCOUNTING");
opt("_SC_2_PBS_CHECKPOINT","_POSIX2_PBS_CHECKPOINT");
opt("_SC_2_PBS_LOCATE","_POSIX2_PBS_LOCATE");
opt("_SC_2_PBS_MESSAGE","_POSIX2_PBS_MESSAGE");
opt("_SC_2_PBS_TRACK","_POSIX2_PBS_TRACK");
opt("_SC_SYMLOOP_MAX","SYMLOOP_MAX");
opt("_SC_STREAMS","_XOPEN_STREAMS");
opt("_SC_HOST_NAME_MAX","HOST_NAME_MAX");
]]]*/
#ifdef ARG_MAX
    [_SC_ARG_MAX] = ARG_MAX,
#endif /* !ARG_MAX */
#ifdef CHILD_MAX
    [_SC_CHILD_MAX] = CHILD_MAX,
#endif /* !CHILD_MAX */
#ifdef NGROUPS_MAX
    [_SC_NGROUPS_MAX] = NGROUPS_MAX,
#endif /* !NGROUPS_MAX */
#ifdef _POSIX_JOB_CONTROL
    [_SC_JOB_CONTROL] = _POSIX_JOB_CONTROL,
#endif /* !_POSIX_JOB_CONTROL */
#ifdef _POSIX_VERSION
    [_SC_VERSION] = _POSIX_VERSION,
#endif /* !_POSIX_VERSION */
#ifdef AIO_LISTIO_MAX
    [_SC_AIO_LISTIO_MAX] = AIO_LISTIO_MAX,
#endif /* !AIO_LISTIO_MAX */
#ifdef AIO_MAX
    [_SC_AIO_MAX] = AIO_MAX,
#endif /* !AIO_MAX */
#ifdef AIO_PRIO_DELTA_MAX
    [_SC_AIO_PRIO_DELTA_MAX] = AIO_PRIO_DELTA_MAX,
#endif /* !AIO_PRIO_DELTA_MAX */
#ifdef DELAYTIMER_MAX
    [_SC_DELAYTIMER_MAX] = DELAYTIMER_MAX,
#endif /* !DELAYTIMER_MAX */
#ifdef MQ_OPEN_MAX
    [_SC_MQ_OPEN_MAX] = MQ_OPEN_MAX,
#endif /* !MQ_OPEN_MAX */
#ifdef MQ_PRIO_MAX
    [_SC_MQ_PRIO_MAX] = MQ_PRIO_MAX,
#endif /* !MQ_PRIO_MAX */
#ifdef RTSIG_MAX
    [_SC_RTSIG_MAX] = RTSIG_MAX,
#endif /* !RTSIG_MAX */
#ifdef SEM_NSEMS_MAX
    [_SC_SEM_NSEMS_MAX] = SEM_NSEMS_MAX,
#endif /* !SEM_NSEMS_MAX */
#ifdef SEM_VALUE_MAX
    [_SC_SEM_VALUE_MAX] = SEM_VALUE_MAX,
#endif /* !SEM_VALUE_MAX */
#ifdef SIGQUEUE_MAX
    [_SC_SIGQUEUE_MAX] = SIGQUEUE_MAX,
#endif /* !SIGQUEUE_MAX */
#ifdef TIMER_MAX
    [_SC_TIMER_MAX] = TIMER_MAX,
#endif /* !TIMER_MAX */
#ifdef BC_BASE_MAX
    [_SC_BC_BASE_MAX] = BC_BASE_MAX,
#endif /* !BC_BASE_MAX */
#ifdef BC_DIM_MAX
    [_SC_BC_DIM_MAX] = BC_DIM_MAX,
#endif /* !BC_DIM_MAX */
#ifdef BC_SCALE_MAX
    [_SC_BC_SCALE_MAX] = BC_SCALE_MAX,
#endif /* !BC_SCALE_MAX */
#ifdef BC_STRING_MAX
    [_SC_BC_STRING_MAX] = BC_STRING_MAX,
#endif /* !BC_STRING_MAX */
#ifdef COLL_WEIGHTS_MAX
    [_SC_COLL_WEIGHTS_MAX] = COLL_WEIGHTS_MAX,
#endif /* !COLL_WEIGHTS_MAX */
#ifdef EXPR_NEST_MAX
    [_SC_EXPR_NEST_MAX] = EXPR_NEST_MAX,
#endif /* !EXPR_NEST_MAX */
#ifdef LINE_MAX
    [_SC_LINE_MAX] = LINE_MAX,
#endif /* !LINE_MAX */
#ifdef RE_DUP_MAX
    [_SC_RE_DUP_MAX] = RE_DUP_MAX,
#endif /* !RE_DUP_MAX */
#ifdef CHARCLASS_NAME_MAX
    [_SC_CHARCLASS_NAME_MAX] = CHARCLASS_NAME_MAX,
#endif /* !CHARCLASS_NAME_MAX */
#ifdef UIO_MAXIOV
    [_SC_UIO_MAXIOV] = UIO_MAXIOV,
#endif /* !UIO_MAXIOV */
#ifdef _T_IOV_MAX
    [_SC_T_IOV_MAX] = _T_IOV_MAX,
#endif /* !_T_IOV_MAX */
#ifdef _POSIX2_VERSION
    [_SC_2_VERSION] = _POSIX2_VERSION,
#endif /* !_POSIX2_VERSION */
#ifdef _POSIX2_C_BIND
    [_SC_2_C_BIND] = _POSIX2_C_BIND,
#endif /* !_POSIX2_C_BIND */
#ifdef _POSIX2_C_DEV
    [_SC_2_C_DEV] = _POSIX2_C_DEV,
#endif /* !_POSIX2_C_DEV */
#ifdef _POSIX2_C_VERSION
    [_SC_2_C_VERSION] = _POSIX2_C_VERSION,
#endif /* !_POSIX2_C_VERSION */
#ifdef _POSIX2_FORT_DEV
    [_SC_2_FORT_DEV] = _POSIX2_FORT_DEV,
#endif /* !_POSIX2_FORT_DEV */
#ifdef _POSIX2_FORT_RUN
    [_SC_2_FORT_RUN] = _POSIX2_FORT_RUN,
#endif /* !_POSIX2_FORT_RUN */
#ifdef _POSIX2_LOCALEDEF
    [_SC_2_LOCALEDEF] = _POSIX2_LOCALEDEF,
#endif /* !_POSIX2_LOCALEDEF */
#ifdef _POSIX2_SW_DEV
    [_SC_2_SW_DEV] = _POSIX2_SW_DEV,
#endif /* !_POSIX2_SW_DEV */
#ifdef _POSIX2_CHAR_TERM
    [_SC_2_CHAR_TERM] = _POSIX2_CHAR_TERM,
#endif /* !_POSIX2_CHAR_TERM */
#ifdef _POSIX2_UPE
    [_SC_2_UPE] = _POSIX2_UPE,
#endif /* !_POSIX2_UPE */
#ifdef NSS_BUFLEN_GROUP
    [_SC_GETGR_R_SIZE_MAX] = NSS_BUFLEN_GROUP,
#endif /* !NSS_BUFLEN_GROUP */
#ifdef NSS_BUFLEN_PASSWD
    [_SC_GETPW_R_SIZE_MAX] = NSS_BUFLEN_PASSWD,
#endif /* !NSS_BUFLEN_PASSWD */
#ifdef LOGIN_NAME_MAX
    [_SC_LOGIN_NAME_MAX] = LOGIN_NAME_MAX,
#endif /* !LOGIN_NAME_MAX */
#ifdef TTY_NAME_MAX
    [_SC_TTY_NAME_MAX] = TTY_NAME_MAX,
#endif /* !TTY_NAME_MAX */
#ifdef INT_MAX
    [_SC_ATEXIT_MAX] = INT_MAX,
#endif /* !INT_MAX */
#ifdef BUFSIZ
    [_SC_PASS_MAX] = BUFSIZ,
#endif /* !BUFSIZ */
#ifdef _XOPEN_VERSION
    [_SC_XOPEN_VERSION] = _XOPEN_VERSION,
#endif /* !_XOPEN_VERSION */
#ifdef _XOPEN_XCU_VERSION
    [_SC_XOPEN_XCU_VERSION] = _XOPEN_XCU_VERSION,
#endif /* !_XOPEN_XCU_VERSION */
#ifdef _XOPEN_UNIX
    [_SC_XOPEN_UNIX] = _XOPEN_UNIX,
#endif /* !_XOPEN_UNIX */
#ifdef _XOPEN_CRYPT
    [_SC_XOPEN_CRYPT] = _XOPEN_CRYPT,
#endif /* !_XOPEN_CRYPT */
#ifdef _XOPEN_ENH_I18N
    [_SC_XOPEN_ENH_I18N] = _XOPEN_ENH_I18N,
#endif /* !_XOPEN_ENH_I18N */
#ifdef _XOPEN_SHM
    [_SC_XOPEN_SHM] = _XOPEN_SHM,
#endif /* !_XOPEN_SHM */
#ifdef _XOPEN_XPG2
    [_SC_XOPEN_XPG2] = _XOPEN_XPG2,
#endif /* !_XOPEN_XPG2 */
#ifdef _XOPEN_XPG3
    [_SC_XOPEN_XPG3] = _XOPEN_XPG3,
#endif /* !_XOPEN_XPG3 */
#ifdef _XOPEN_XPG4
    [_SC_XOPEN_XPG4] = _XOPEN_XPG4,
#endif /* !_XOPEN_XPG4 */
#ifdef NL_ARGMAX
    [_SC_NL_ARGMAX] = NL_ARGMAX,
#endif /* !NL_ARGMAX */
#ifdef NL_LANGMAX
    [_SC_NL_LANGMAX] = NL_LANGMAX,
#endif /* !NL_LANGMAX */
#ifdef NL_MSGMAX
    [_SC_NL_MSGMAX] = NL_MSGMAX,
#endif /* !NL_MSGMAX */
#ifdef NL_NMAX
    [_SC_NL_NMAX] = NL_NMAX,
#endif /* !NL_NMAX */
#ifdef NL_SETMAX
    [_SC_NL_SETMAX] = NL_SETMAX,
#endif /* !NL_SETMAX */
#ifdef NL_TEXTMAX
    [_SC_NL_TEXTMAX] = NL_TEXTMAX,
#endif /* !NL_TEXTMAX */
#ifdef _XOPEN_LEGACY
    [_SC_XOPEN_LEGACY] = _XOPEN_LEGACY,
#endif /* !_XOPEN_LEGACY */
#ifdef _XOPEN_REALTIME
    [_SC_XOPEN_REALTIME] = _XOPEN_REALTIME,
#endif /* !_XOPEN_REALTIME */
#ifdef _XOPEN_REALTIME_THREADS
    [_SC_XOPEN_REALTIME_THREADS] = _XOPEN_REALTIME_THREADS,
#endif /* !_XOPEN_REALTIME_THREADS */
#ifdef _POSIX2_PBS
    [_SC_2_PBS] = _POSIX2_PBS,
#endif /* !_POSIX2_PBS */
#ifdef _POSIX2_PBS_ACCOUNTING
    [_SC_2_PBS_ACCOUNTING] = _POSIX2_PBS_ACCOUNTING,
#endif /* !_POSIX2_PBS_ACCOUNTING */
#ifdef _POSIX2_PBS_CHECKPOINT
    [_SC_2_PBS_CHECKPOINT] = _POSIX2_PBS_CHECKPOINT,
#endif /* !_POSIX2_PBS_CHECKPOINT */
#ifdef _POSIX2_PBS_LOCATE
    [_SC_2_PBS_LOCATE] = _POSIX2_PBS_LOCATE,
#endif /* !_POSIX2_PBS_LOCATE */
#ifdef _POSIX2_PBS_MESSAGE
    [_SC_2_PBS_MESSAGE] = _POSIX2_PBS_MESSAGE,
#endif /* !_POSIX2_PBS_MESSAGE */
#ifdef _POSIX2_PBS_TRACK
    [_SC_2_PBS_TRACK] = _POSIX2_PBS_TRACK,
#endif /* !_POSIX2_PBS_TRACK */
#ifdef SYMLOOP_MAX
    [_SC_SYMLOOP_MAX] = SYMLOOP_MAX,
#endif /* !SYMLOOP_MAX */
#ifdef _XOPEN_STREAMS
    [_SC_STREAMS] = _XOPEN_STREAMS,
#endif /* !_XOPEN_STREAMS */
#ifdef HOST_NAME_MAX
    [_SC_HOST_NAME_MAX] = HOST_NAME_MAX,
#endif /* !HOST_NAME_MAX */
//[[[end]]]


#ifdef STREAM_MAX
    [_SC_STREAM_MAX] = STREAM_MAX,
#else
    [_SC_STREAM_MAX] = FOPEN_MAX,
#endif

    [_SC_CHAR_BIT]   = CHAR_BIT,
    [_SC_CHAR_MAX]   = CHAR_MAX,
    [_SC_CHAR_MIN]   = CHAR_MIN,
    [_SC_INT_MAX]    = INT_MAX,
    [_SC_INT_MIN]    = INT_MIN,
    [_SC_LONG_BIT]   = LONG_BIT,
    [_SC_WORD_BIT]   = WORD_BIT,
    [_SC_MB_LEN_MAX] = MB_LEN_MAX,
    [_SC_NZERO]      = NZERO,
    [_SC_SSIZE_MAX]  = _POSIX_SSIZE_MAX,
    [_SC_SCHAR_MAX]  = SCHAR_MAX,
    [_SC_SCHAR_MIN]  = SCHAR_MIN,
    [_SC_SHRT_MAX]   = SHRT_MAX,
    [_SC_SHRT_MIN]   = SHRT_MIN,
    [_SC_UCHAR_MAX]  = UCHAR_MAX,
    [_SC_UINT_MAX]   = UINT_MAX,
    [_SC_ULONG_MAX]  = ULONG_MAX,
    [_SC_USHRT_MAX]  = USHRT_MAX,

    [_SC_TZNAME_MAX]            = -1,
    [_SC_SAVED_IDS]             = -1,
    [_SC_REALTIME_SIGNALS]      = -1, /* ??? */
    [_SC_PRIORITY_SCHEDULING]   = 1,
    [_SC_TIMERS]                = -1, /* ??? */
    [_SC_ASYNCHRONOUS_IO]       = -1, /* ??? */
    [_SC_PRIORITIZED_IO]        = -1, /* ??? */
    [_SC_SYNCHRONIZED_IO]       = -1, /* ??? */
    [_SC_FSYNC]                 = 1,
    [_SC_MAPPED_FILES]          = 1,
    [_SC_MEMLOCK]               = -1, /* ??? */
    [_SC_MEMLOCK_RANGE]         = -1, /* ??? */
    [_SC_MEMORY_PROTECTION]     = 1,
    [_SC_MESSAGE_PASSING]       = -1, /* ??? */
    [_SC_SEMAPHORES]            = 1,
    [_SC_SHARED_MEMORY_OBJECTS] = 1,
    [_SC_PII]                   = -1, /* ??? */
    [_SC_PII_XTI]               = -1, /* ??? */
    [_SC_PII_SOCKET]            = -1, /* ??? */
    [_SC_PII_INTERNET]          = -1, /* ??? */
    [_SC_PII_OSI]               = -1, /* ??? */
    [_SC_POLL]                  = -1, /* ??? */
    [_SC_SELECT]                = -1, /* TODO */

    [_SC_THREADS]                      = -1, /* TODO */
    [_SC_THREAD_SAFE_FUNCTIONS]        = -1, /* TODO */
    [_SC_THREAD_DESTRUCTOR_ITERATIONS] = -1, /* TODO */
    [_SC_THREAD_KEYS_MAX]              = -1, /* TODO */
    [_SC_THREAD_STACK_MIN]             = -1, /* TODO */
    [_SC_THREAD_THREADS_MAX]           = -1, /* TODO */
    [_SC_THREAD_ATTR_STACKADDR]        = -1, /* TODO */
    [_SC_THREAD_ATTR_STACKSIZE]        = -1, /* TODO */
    [_SC_THREAD_PRIORITY_SCHEDULING]   = -1, /* TODO */
    [_SC_THREAD_PRIO_INHERIT]          = -1, /* TODO */
    [_SC_THREAD_PRIO_PROTECT]          = -1, /* TODO */
    [_SC_THREAD_PROCESS_SHARED]        = -1, /* TODO */

    [_SC_ADVISORY_INFO]     = 1,
    [_SC_BARRIERS]          = -1, /* ??? */
    [_SC_BASE]              = -1, /* ??? */
    [_SC_C_LANG_SUPPORT]    = 1, /* ??? */
    [_SC_C_LANG_SUPPORT_R]  = 1, /* ??? */
    [_SC_CLOCK_SELECTION]   = -1, /* ??? */
    [_SC_CPUTIME]           = -1, /* ??? */
    [_SC_DEVICE_IO]         = -1, /* ??? */
    [_SC_DEVICE_SPECIFIC]   = -1, /* ??? */
    [_SC_DEVICE_SPECIFIC_R] = -1, /* ??? */
    [_SC_FD_MGMT]           = -1, /* ??? */
    [_SC_FIFO]              = 1,
    [_SC_PIPE]              = 1,
    [_SC_FILE_ATTRIBUTES]   = 1,
    [_SC_FILE_LOCKING]      = 1,
    [_SC_FILE_SYSTEM]       = 1,
    [_SC_MONOTONIC_CLOCK]   = 1,
    [_SC_MULTI_PROCESS]     = 1,
    [_SC_SINGLE_PROCESS]    = 1,

    [_SC_PII_INTERNET_STREAM] = -1, /* ??? */
    [_SC_PII_INTERNET_DGRAM]  = -1, /* ??? */
    [_SC_PII_OSI_COTS]        = -1, /* ??? */
    [_SC_PII_OSI_CLTS]        = -1, /* ??? */
    [_SC_PII_OSI_M]           = -1, /* ??? */
    [_SC_NETWORKING]          = -1, /* ??? */
    [_SC_IPV6]                = -1, // todo
    [_SC_RAW_SOCKETS]         = -1, // todo

    [_SC_REGEXP]        = -1, /* todo? */
    [_SC_REGEX_VERSION] = -1, /* todo? */

    [_SC_SPORADIC_SERVER]        = -1, /* ??? */
    [_SC_THREAD_SPORADIC_SERVER] = -1, /* ??? */
    [_SC_SYSTEM_DATABASE]        = -1, /* ??? */
    [_SC_SYSTEM_DATABASE_R]      = -1, /* ??? */
    [_SC_THREAD_CPUTIME]         = -1, /* TODO */

    [_SC_READER_WRITER_LOCKS]  = 1,
    [_SC_SPIN_LOCKS]           = 1,
    [_SC_SHELL]                = 1,
    [_SC_SIGNALS]              = 1,
    [_SC_SPAWN]                = 1,
    [_SC_TIMEOUTS]             = 1,
    [_SC_TYPED_MEMORY_OBJECTS] = -1, /* ??? */
    [_SC_USER_GROUPS]          = -1, /* ??? */
    [_SC_USER_GROUPS_R]        = -1, /* ??? */

    [_SC_TRACE]              = -1, /* ??? */
    [_SC_TRACE_EVENT_FILTER] = -1, /* ??? */
    [_SC_TRACE_INHERIT]      = -1, /* ??? */
    [_SC_TRACE_LOG]          = -1, /* ??? */


    [_SC_TRACE_EVENT_NAME_MAX] = -1, /* ??? */
    [_SC_TRACE_NAME_MAX]       = -1, /* ??? */
    [_SC_TRACE_SYS_MAX]        = -1, /* ??? */
    [_SC_TRACE_USER_EVENT_MAX] = -1, /* ??? */

    [_SC_XOPEN_STREAMS] = -1, /* ??? */

    /* Return ZERO(0) to indicate no specific information. */
    [_SC_LEVEL1_ICACHE_SIZE]     = 0,
    [_SC_LEVEL1_ICACHE_ASSOC]    = 0,
    [_SC_LEVEL1_ICACHE_LINESIZE] = 0,
    [_SC_LEVEL1_DCACHE_SIZE]     = 0,
    [_SC_LEVEL1_DCACHE_ASSOC]    = 0,
    [_SC_LEVEL1_DCACHE_LINESIZE] = 0,
    [_SC_LEVEL2_CACHE_SIZE]      = 0,
    [_SC_LEVEL2_CACHE_ASSOC]     = 0,
    [_SC_LEVEL2_CACHE_LINESIZE]  = 0,
    [_SC_LEVEL3_CACHE_SIZE]      = 0,
    [_SC_LEVEL3_CACHE_ASSOC]     = 0,
    [_SC_LEVEL3_CACHE_LINESIZE]  = 0,
    [_SC_LEVEL4_CACHE_SIZE]      = 0,
    [_SC_LEVEL4_CACHE_ASSOC]     = 0,
    [_SC_LEVEL4_CACHE_LINESIZE]  = 0,

    [_SC_V7_ILP32_OFF32]             = -1, /* ??? */
    [_SC_V7_ILP32_OFFBIG]            = -1, /* ??? */
    [_SC_V7_LP64_OFF64]              = -1, /* ??? */
    [_SC_V7_LPBIG_OFFBIG]            = -1, /* ??? */
    [_SC_SS_REPL_MAX]                = -1, /* ??? */
    [_SC_THREAD_ROBUST_PRIO_INHERIT] = -1, /* ??? */
    [_SC_THREAD_ROBUST_PRIO_PROTECT] = -1, /* ??? */

    [_SC_PAGESIZE] = PAGESIZE,
};
#pragma GCC diagnostic pop

INTERN long int LIBCCALL libc_sysconf(int name) {
 switch (name) {

 case _SC_CLK_TCK:          return -1; /* TODO: __getclktck(); */
 case _SC_OPEN_MAX:         return -1; /* TODO: __getdtablesize(); */
 case _SC_NPROCESSORS_CONF: return -1; /* TODO: __get_nprocs_conf(); */
 case _SC_NPROCESSORS_ONLN: return -1; /* TODO: __get_nprocs(); */
 case _SC_PHYS_PAGES:       return -1; /* TODO: __get_phys_pages(); */
 case _SC_AVPHYS_PAGES:     return -1; /* TODO: __get_avphys_pages(); */

 default:
  if (name < _SC_COUNT)
      return sysconf_values[name];
 case _SC_EQUIV_CLASS_MAX:
  SET_ERRNO(-EINVAL);
  break;
 }
 return -1;
}

DEFINE_PUBLIC_ALIAS(sysconf,libc_sysconf);

DECL_END

#endif /* !GUARD_LIBS_LIBC_SYSCONF_C */
