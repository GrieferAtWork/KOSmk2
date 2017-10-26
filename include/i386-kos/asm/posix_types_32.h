#ifndef _ASM_X86_POSIX_TYPES_32_H
#define _ASM_X86_POSIX_TYPES_32_H

#include <hybrid/typecore.h>

/* NOTE: Types in here are not actually used by the kernel!
 *       This header only mirrors what GLibC does for API compatibility. */

typedef __UINT16_TYPE__    __kernel_mode_t;
#define __kernel_mode_t    __kernel_mode_t

typedef __UINT16_TYPE__    __kernel_ipc_pid_t;
#define __kernel_ipc_pid_t __kernel_ipc_pid_t

typedef __UINT16_TYPE__    __kernel_uid_t;
#define __kernel_uid_t     __kernel_uid_t

typedef __UINT16_TYPE__    __kernel_gid_t;
#define __kernel_gid_t     __kernel_gid_t

typedef __UINT16_TYPE__    __kernel_old_dev_t;
#define __kernel_old_dev_t __kernel_old_dev_t

#include <asm-generic/posix_types.h>

#endif /* _ASM_X86_POSIX_TYPES_32_H */
