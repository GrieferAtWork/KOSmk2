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
#ifndef GUARD_MODULES_NT_FLAVOR_C
#define GUARD_MODULES_NT_FLAVOR_C 1

#include <hybrid/compiler.h>

#include "api.h"
#include "flavor.h"

DECL_BEGIN

STATIC_ASSERT(offsetof(struct nt_flavor,nf_name) == NT_FLAVOR_OFFSETOF_NAME);
STATIC_ASSERT(offsetof(struct nt_flavor,nf_call) == NT_FLAVOR_OFFSETOF_CALL);
STATIC_ASSERT(offsetof(struct nt_flavor,nf_argc) == NT_FLAVOR_OFFSETOF_ARGC);
STATIC_ASSERT(sizeof(struct nt_flavor) == NT_FLAVOR_SIZE);

#define ARRAY_GET0(a,...)         a
#define ARRAY_GET1(a,b,...)       b
#define ARRAY_GET2(a,b,c,...)     c
#define ARRAY_GET3(a,b,c,d,...)   d
#define ARRAY_GET4(a,b,c,d,e,...) e

#define NT_F0(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET0 i0] = (nt_syscall)&Nt##name,
#define NT_F1(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET1 i0] = (nt_syscall)&Nt##name,
#define NT_F2(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET2 i0] = (nt_syscall)&Nt##name,
#define NT_F3(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET3 i0] = (nt_syscall)&Nt##name,
#define NT_F4(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET0 i1] = (nt_syscall)&Nt##name,
#define NT_F5(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET1 i1] = (nt_syscall)&Nt##name,
#define NT_F6(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET2 i1] = (nt_syscall)&Nt##name,
#define NT_F7(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET3 i1] = (nt_syscall)&Nt##name,
#define NT_F8(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET4 i1] = (nt_syscall)&Nt##name,
#define NT_F9(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8)  [ARRAY_GET0 i2] = (nt_syscall)&Nt##name,
#define NT_F10(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i2] = (nt_syscall)&Nt##name,
#define NT_F11(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET2 i2] = (nt_syscall)&Nt##name,
#define NT_F12(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET3 i2] = (nt_syscall)&Nt##name,
#define NT_F13(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET0 i3] = (nt_syscall)&Nt##name,
#define NT_F14(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i3] = (nt_syscall)&Nt##name,
#define NT_F15(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET2 i3] = (nt_syscall)&Nt##name,
#define NT_F16(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET3 i3] = (nt_syscall)&Nt##name,
#define NT_F17(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET3 i3] = (nt_syscall)&Nt##name,
#define NT_F18(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET0 i4] = (nt_syscall)&Nt##name,
#define NT_F19(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i4] = (nt_syscall)&Nt##name,
#define NT_F20(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET2 i4] = (nt_syscall)&Nt##name,
#define NT_F21(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET0 i5] = (nt_syscall)&Nt##name,
#define NT_F22(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i5] = (nt_syscall)&Nt##name,
#define NT_F23(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET0 i6] = (nt_syscall)&Nt##name,
#define NT_F24(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i6] = (nt_syscall)&Nt##name,
#define NT_F25(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET0 i7] = (nt_syscall)&Nt##name,
#define NT_F26(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i7] = (nt_syscall)&Nt##name,
#define NT_F27(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET0 i8] = (nt_syscall)&Nt##name,
#define NT_F28(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET1 i8] = (nt_syscall)&Nt##name,
#define NT_F29(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET2 i8] = (nt_syscall)&Nt##name,
#define NT_F30(name,argc,i0,i1,i2,i3,i4,i5,i6,i7,i8) [ARRAY_GET3 i8] = (nt_syscall)&Nt##name,

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

#ifndef __INTELLISENSE__
#define NOTSUP   NT_SYSCALL_COUNT
PUBLIC struct nt_flavor nt_flavors[NT_FLAVOR_COUNT] = {
/*[[[deemon
#include <file>
#include <util>
local flavor_names = [none]*NT_FLAVOR_COUNT;
for (local line: file.open("flavor.h")) {
    local name,id;
    try {
       name,id = line.scanf(" # define NT_FLAVOR_%[^ ] %[0-9]")...;
       id = (int)id;
    } catch (...) continue;
    if (id < #flavor_names)
        flavor_names[id] = name;
}
for (local i: util::range(NT_FLAVOR_COUNT)) {
    local name = flavor_names[i];
    if (name is none) continue;
    print "   [NT_FLAVOR_"+name+"] = {";
    print "       .nf_call = {";
    print "           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,";
    print "#define NT NT_F"+i;
    print "#include \"flavor-link.inl\"";
    print "       },";
    print "       .nf_name = "+repr(name.lower().replace("_","-").lsstrip("windows-"))+",";
    print "   },";
}
]]]*/
   [NT_FLAVOR_WINDOWS_NT_SP3] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F0
#include "flavor-link.inl"
       },
       .nf_name = "nt-sp3",
   },
   [NT_FLAVOR_WINDOWS_NT_SP4] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F1
#include "flavor-link.inl"
       },
       .nf_name = "nt-sp4",
   },
   [NT_FLAVOR_WINDOWS_NT_SP5] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F2
#include "flavor-link.inl"
       },
       .nf_name = "nt-sp5",
   },
   [NT_FLAVOR_WINDOWS_NT_SP6] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F3
#include "flavor-link.inl"
       },
       .nf_name = "nt-sp6",
   },
   [NT_FLAVOR_WINDOWS_2000_SP0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F4
#include "flavor-link.inl"
       },
       .nf_name = "2000-sp0",
   },
   [NT_FLAVOR_WINDOWS_2000_SP1] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F5
#include "flavor-link.inl"
       },
       .nf_name = "2000-sp1",
   },
   [NT_FLAVOR_WINDOWS_2000_SP2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F6
#include "flavor-link.inl"
       },
       .nf_name = "2000-sp2",
   },
   [NT_FLAVOR_WINDOWS_2000_SP3] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F7
#include "flavor-link.inl"
       },
       .nf_name = "2000-sp3",
   },
   [NT_FLAVOR_WINDOWS_2000_SP4] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F8
#include "flavor-link.inl"
       },
       .nf_name = "2000-sp4",
   },
   [NT_FLAVOR_WINDOWS_XP_SP0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F9
#include "flavor-link.inl"
       },
       .nf_name = "xp-sp0",
   },
   [NT_FLAVOR_WINDOWS_XP_SP1] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F10
#include "flavor-link.inl"
       },
       .nf_name = "xp-sp1",
   },
   [NT_FLAVOR_WINDOWS_XP_SP2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F11
#include "flavor-link.inl"
       },
       .nf_name = "xp-sp2",
   },
   [NT_FLAVOR_WINDOWS_XP_SP3] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F12
#include "flavor-link.inl"
       },
       .nf_name = "xp-sp3",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2003_SP0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F13
#include "flavor-link.inl"
       },
       .nf_name = "server-2003-sp0",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2003_SP1] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F14
#include "flavor-link.inl"
       },
       .nf_name = "server-2003-sp1",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2003_SP2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F15
#include "flavor-link.inl"
       },
       .nf_name = "server-2003-sp2",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2003_R2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F16
#include "flavor-link.inl"
       },
       .nf_name = "server-2003-r2",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2003_R2_SP2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F17
#include "flavor-link.inl"
       },
       .nf_name = "server-2003-r2-sp2",
   },
   [NT_FLAVOR_WINDOWS_VISTA_SP0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F18
#include "flavor-link.inl"
       },
       .nf_name = "vista-sp0",
   },
   [NT_FLAVOR_WINDOWS_VISTA_SP1] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F19
#include "flavor-link.inl"
       },
       .nf_name = "vista-sp1",
   },
   [NT_FLAVOR_WINDOWS_VISTA_SP2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F20
#include "flavor-link.inl"
       },
       .nf_name = "vista-sp2",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2008_SP0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F21
#include "flavor-link.inl"
       },
       .nf_name = "server-2008-sp0",
   },
   [NT_FLAVOR_WINDOWS_SERVER_2008_SP2] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F22
#include "flavor-link.inl"
       },
       .nf_name = "server-2008-sp2",
   },
   [NT_FLAVOR_WINDOWS_7_SP0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F23
#include "flavor-link.inl"
       },
       .nf_name = "7-sp0",
   },
   [NT_FLAVOR_WINDOWS_7_SP1] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F24
#include "flavor-link.inl"
       },
       .nf_name = "7-sp1",
   },
   [NT_FLAVOR_WINDOWS_8_0] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F25
#include "flavor-link.inl"
       },
       .nf_name = "8-0",
   },
   [NT_FLAVOR_WINDOWS_8_1] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F26
#include "flavor-link.inl"
       },
       .nf_name = "8-1",
   },
   [NT_FLAVOR_WINDOWS_10_1507] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F27
#include "flavor-link.inl"
       },
       .nf_name = "10-1507",
   },
   [NT_FLAVOR_WINDOWS_10_1511] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F28
#include "flavor-link.inl"
       },
       .nf_name = "10-1511",
   },
   [NT_FLAVOR_WINDOWS_10_1607] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F29
#include "flavor-link.inl"
       },
       .nf_name = "10-1607",
   },
   [NT_FLAVOR_WINDOWS_10_1703] = {
       .nf_call = {
           [0 ... NT_SYSCALL_COUNT] = (nt_syscall)&NtBadSysCall,
#define NT NT_F30
#include "flavor-link.inl"
       },
       .nf_name = "10-1703",
   },
//[[[end]]]
};
#undef NOTSUP
#endif

#pragma GCC diagnostic pop


DECL_END

#endif /* !GUARD_MODULES_NT_FLAVOR_C */
