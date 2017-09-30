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
#ifndef GUARD_MODULES_NT_FLAVOR_H
#define GUARD_MODULES_NT_FLAVOR_H 1

#include <hybrid/compiler.h>
#include <winapi/ntdef.h>

DECL_BEGIN

/* NOTE: System call numbers are taken from
 *      'http://j00ru.vexillium.org/syscalls/nt/32/' */

/* The system call flavor version that should be emulated. */
#define NT_FLAVOR_WINDOWS_NT_SP3             0
#define NT_FLAVOR_WINDOWS_NT_SP4             1
#define NT_FLAVOR_WINDOWS_NT_SP5             2
#define NT_FLAVOR_WINDOWS_NT_SP6             3
#define NT_FLAVOR_WINDOWS_2000_SP0           4
#define NT_FLAVOR_WINDOWS_2000_SP1           5
#define NT_FLAVOR_WINDOWS_2000_SP2           6
#define NT_FLAVOR_WINDOWS_2000_SP3           7
#define NT_FLAVOR_WINDOWS_2000_SP4           8
#define NT_FLAVOR_WINDOWS_XP_SP0             9
#define NT_FLAVOR_WINDOWS_XP_SP1             10
#define NT_FLAVOR_WINDOWS_XP_SP2             11
#define NT_FLAVOR_WINDOWS_XP_SP3             12
#define NT_FLAVOR_WINDOWS_SERVER_2003_SP0    13
#define NT_FLAVOR_WINDOWS_SERVER_2003_SP1    14
#define NT_FLAVOR_WINDOWS_SERVER_2003_SP2    15
#define NT_FLAVOR_WINDOWS_SERVER_2003_R2     16
#define NT_FLAVOR_WINDOWS_SERVER_2003_R2_SP2 17
#define NT_FLAVOR_WINDOWS_VISTA_SP0          18
#define NT_FLAVOR_WINDOWS_VISTA_SP1          19
#define NT_FLAVOR_WINDOWS_VISTA_SP2          20
#define NT_FLAVOR_WINDOWS_SERVER_2008_SP0    21
#define NT_FLAVOR_WINDOWS_SERVER_2008_SP2    22
#define NT_FLAVOR_WINDOWS_7_SP0              23
#define NT_FLAVOR_WINDOWS_7_SP1              24
#define NT_FLAVOR_WINDOWS_8_0                25
#define NT_FLAVOR_WINDOWS_8_1                26
#define NT_FLAVOR_WINDOWS_10_1507            27
#define NT_FLAVOR_WINDOWS_10_1511            28
#define NT_FLAVOR_WINDOWS_10_1607            29
#define NT_FLAVOR_WINDOWS_10_1703            30
#define NT_FLAVOR_COUNT                      31

/* The default flavor used when the installed 'ntdll.dll' couldn't be recognized. */
#define NT_FLAVOR_DEFAULT   NT_FLAVOR_WINDOWS_7_SP1

#define NT_SYSCALL_COUNT 0x01c8 /* The greatest system call number of any (current) flavor +1. */
typedef NTSTATUS (NTAPI *nt_syscall)();

#define NT_FLAVOR_OFFSETOF_NAME  0
#define NT_FLAVOR_OFFSETOF_CALL  24
#define NT_FLAVOR_OFFSETOF_ARGC (24+(NT_SYSCALL_COUNT+1)*__SIZEOF_POINTER__)
#define NT_FLAVOR_SIZE          (24+((NT_SYSCALL_COUNT+1)*__SIZEOF_POINTER__)+ \
                                     (NT_SYSCALL_COUNT+__SIZEOF_POINTER__))

struct nt_flavor {
 char       nf_name[24]; /*< Flavor name. */
 nt_syscall nf_call[NT_SYSCALL_COUNT+1];
 u8         nf_argc[NT_SYSCALL_COUNT+__SIZEOF_POINTER__];
};


/* The currently set NT flavor and a way of changing it.
 * NOTE: To change the NT flavor, simply use an atomic store  */
DATDEF struct nt_flavor *nt_current_flavor;

/* Vector of known NT flavors. */
DATDEF struct nt_flavor nt_flavors[NT_FLAVOR_COUNT];



DECL_END

#endif /* !GUARD_MODULES_NT_FLAVOR_H */
