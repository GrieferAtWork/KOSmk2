#ifndef _ASM_X86_POSIX_TYPES_64_H
#define _ASM_X86_POSIX_TYPES_64_H

/* NOTE: Types in here are not actually used by the kernel!
 *       This header only mirrors what GLibC does for API compatibility. */

#include <hybrid/typecore.h>

typedef __UINT16_TYPE__    __kernel_old_uid_t;
#define __kernel_old_uid_t __kernel_old_uid_t

typedef __UINT16_TYPE__    __kernel_old_gid_t;
#define __kernel_old_gid_t __kernel_old_gid_t

typedef __UINT64_TYPE__    __kernel_old_dev_t;
#define __kernel_old_dev_t __kernel_old_dev_t

#include <asm-generic/posix_types.h>

#endif /* _ASM_X86_POSIX_TYPES_64_H */
