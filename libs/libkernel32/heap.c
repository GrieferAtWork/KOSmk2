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
#ifndef GUARD_LIBS_LIBKERNEL32_HEAP_C
#define GUARD_LIBS_LIBKERNEL32_HEAP_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* mremap() */

#include "k32.h"
#include "heap.h"
#include <hybrid/compiler.h>
#include <errno.h>
#include <sys/mman.h>


/* Don't confuse dlmalloc into thinking it's running on windows. */
#undef WIN32
#undef _WIN32
#undef _WIN32_WCE

/* Delete misc macros also defined by dlmalloc */
#undef FORCEINLINE
#undef NOINLINE

/* Pull in a new copy of dlmalloc(), using its
 * mspace feature to implement Kernel32's heaps. */
#define DLMALLOC_NO_ONETIME_INIT 1
#define NO_MALLOC_STATS          1
#define USE_DL_PREFIX            1
#define malloc_getpagesize       PAGESIZE
#define DLMALLOC_EXPORT          INTDEF
#define MSPACES                  1 /* This is what we use to emulate DOS's heaps. */
#define ONLY_MSPACES             0
#ifndef NDEBUG
#define DEBUG                    1
#define FOOTERS                  1
#endif
#define TRIM_RETURNS_RELEASED    1
#undef REALLOC_ZERO_BYTES_FREES

/* Re-use libc's dlmalloc source file. */
#pragma GCC visibility push(hidden)
#include "../libc/dlmalloc.c.inl"
#pragma GCC visibility pop
         
DECL_BEGIN


/* Heap API */
INTERN HANDLE WINAPI
K32_HeapCreate(DWORD UNUSED(flOptions), SIZE_T dwInitialSize,
               SIZE_T UNUSED(dwMaximumSize)) {
 return create_mspace((size_t)dwInitialSize,1);
}
INTERN WINBOOL WINAPI K32_HeapDestroy(HANDLE hHeap) {
 if (!hHeap || hHeap == INVALID_HANDLE_VALUE) return FALSE;
 destroy_mspace(hHeap);
 return TRUE;
}
INTERN LPVOID WINAPI
K32_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
 if (!hHeap || hHeap == INVALID_HANDLE_VALUE) return NULL;
 return dwFlags&HEAP_ZERO_MEMORY
      ? mspace_calloc(hHeap,1,dwBytes)
      : mspace_malloc(hHeap,dwBytes);
}
INTERN LPVOID WINAPI
K32_HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes) {
 void *result;
 if (!hHeap || hHeap == INVALID_HANDLE_VALUE) return NULL;
 if (dwFlags&HEAP_ZERO_MEMORY) {
  if (!lpMem)
   result = mspace_calloc(hHeap,1,dwBytes);
  else {
   size_t oldsize = dlmalloc_usable_size(lpMem);
   if (dwFlags&HEAP_REALLOC_IN_PLACE_ONLY)
    result = mspace_realloc_in_place(hHeap,lpMem,dwBytes);
   else {
    result = mspace_realloc(hHeap,lpMem,dwBytes);
   }
   /* ZERO-initialize new memory. */
   if (result && dwBytes > oldsize)
       memset((byte_t *)result+oldsize,0,dwBytes-oldsize);
  }
 } else if (dwFlags&HEAP_REALLOC_IN_PLACE_ONLY) {
  result = mspace_realloc_in_place(hHeap,lpMem,dwBytes);
 } else {
  result = mspace_realloc(hHeap,lpMem,dwBytes);
 }
 return result;
}
INTERN WINBOOL WINAPI
K32_HeapFree(HANDLE hHeap, DWORD UNUSED(dwFlags), LPVOID lpMem) {
 if (!hHeap || hHeap == INVALID_HANDLE_VALUE)
     return FALSE;
 mspace_free(hHeap,lpMem);
 return TRUE;
}
INTERN SIZE_T WINAPI
K32_HeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem) {
 if (!hHeap || hHeap == INVALID_HANDLE_VALUE/* || !lpMem*/)
     return (SIZE_T)-1;
 return dlmalloc_usable_size((void *)lpMem);
}
INTERN WINBOOL WINAPI
K32_HeapValidate(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem) {
 return hHeap && hHeap != INVALID_HANDLE_VALUE && lpMem != NULL;
}
INTERN SIZE_T WINAPI K32_HeapCompact(HANDLE hHeap, DWORD dwFlags) {
 if (!hHeap || hHeap == INVALID_HANDLE_VALUE/* || !lpMem*/)
     return 0;
 SET_ERRNO(EOK);
 return (SIZE_T)mspace_trim(hHeap,0);
}
INTERN HANDLE WINAPI K32_GetProcessHeap(void) { ensure_initialization(); return gm; }
INTERN DWORD WINAPI K32_GetProcessHeaps(DWORD NumberOfHeaps, PHANDLE ProcessHeaps) {
 if (NumberOfHeaps >= 1) ProcessHeaps[0] = gm;
 return 1;
}
INTERN WINBOOL WINAPI K32_HeapLock(HANDLE hHeap) { return hHeap && hHeap != INVALID_HANDLE_VALUE; }
INTERN WINBOOL WINAPI K32_HeapUnlock(HANDLE hHeap) { return hHeap && hHeap != INVALID_HANDLE_VALUE; }
INTERN WINBOOL WINAPI K32_HeapWalk(HANDLE hHeap, LPPROCESS_HEAP_ENTRY lpEntry) { SET_NT_ERRNO(ERROR_NO_MORE_ITEMS); return FALSE; }
INTERN WINBOOL WINAPI
K32_HeapSetInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass,
                       PVOID HeapInformation, SIZE_T HeapInformationLength) {
 /* TODO */
 return FALSE;
}
INTERN WINBOOL WINAPI
K32_HeapQueryInformation(HANDLE HeapHandle, HEAP_INFORMATION_CLASS HeapInformationClass,
                         PVOID HeapInformation, SIZE_T HeapInformationLength, PSIZE_T ReturnLength) {
 /* TODO */
 return FALSE;
}



/* Heap API */
DEFINE_PUBLIC_ALIAS(HeapCreate,K32_HeapCreate);
DEFINE_PUBLIC_ALIAS(HeapDestroy,K32_HeapDestroy);
DEFINE_PUBLIC_ALIAS(HeapAlloc,K32_HeapAlloc);
DEFINE_PUBLIC_ALIAS(HeapReAlloc,K32_HeapReAlloc);
DEFINE_PUBLIC_ALIAS(HeapFree,K32_HeapFree);
DEFINE_PUBLIC_ALIAS(HeapSize,K32_HeapSize);
DEFINE_PUBLIC_ALIAS(HeapValidate,K32_HeapValidate);
DEFINE_PUBLIC_ALIAS(HeapCompact,K32_HeapCompact);
DEFINE_PUBLIC_ALIAS(GetProcessHeap,K32_GetProcessHeap);
DEFINE_PUBLIC_ALIAS(GetProcessHeaps,K32_GetProcessHeaps);
DEFINE_PUBLIC_ALIAS(HeapLock,K32_HeapLock);
DEFINE_PUBLIC_ALIAS(HeapUnlock,K32_HeapUnlock);
DEFINE_PUBLIC_ALIAS(HeapWalk,K32_HeapWalk);
DEFINE_PUBLIC_ALIAS(HeapSetInformation,K32_HeapSetInformation);
DEFINE_PUBLIC_ALIAS(HeapQueryInformation,K32_HeapQueryInformation);

DECL_END

#endif /* !GUARD_LIBS_LIBKERNEL32_HEAP_C */
