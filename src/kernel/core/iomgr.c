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
#ifndef GUARD_KERNEL_CORE_IOMGR_C
#define GUARD_KERNEL_CORE_IOMGR_C 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/atree.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <syslog.h>
#include <errno.h>
#include <kernel/iomgr.h>
#include <linker/module.h>

DECL_BEGIN

PUBLIC struct iomgr io_mgr = {
};

FUNDEF ioaddr_t KCALL
io_malloc(iosize_t size, ioport_t max,
          struct instance *__restrict owner) {
 syslog(LOG_WARN,"TODO: io_malloc()\n");
 return -ENOMEM;
}
FUNDEF void KCALL io_free(ioport_t port, iosize_t size) {
 syslog(LOG_WARN,"TODO: io_free()\n");
}
FUNDEF ioaddr_t KCALL
io_malloc_at(ioport_t addr, iosize_t size,
             struct instance *__restrict owner) {
 /* TODO */
 return addr;
}


FUNDEF ioaddr_t KCALL
io_malloc_at_warn(ioport_t addr, iosize_t size,
                  struct instance *__restrict owner) {
 ioaddr_t result;
 result = io_malloc_at(addr,size,owner);
 /* Log an error message if the reservation failed. */
 if (E_ISERR(result))
     syslog(LOG_ERR,"[IO] Failed to reserve I/O range %.4I16x...%.4I16x for module %[file]: %[errno]\n",
            addr,addr+size-1,owner->i_module->m_file,-result);
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_CORE_IOMGR_C */
