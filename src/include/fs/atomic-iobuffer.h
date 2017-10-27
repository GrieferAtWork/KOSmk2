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
#ifndef GUARD_INCLUDE_FS_ATOMIC_IOBUFFER_H
#define GUARD_INCLUDE_FS_ATOMIC_IOBUFFER_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <sync/sig.h>
#ifdef CONFIG_SMP
#include <hybrid/sync/atomic-rwlock.h>
#endif /* CONFIG_SMP */

DECL_BEGIN

/* A special kind of I/O buffer that is specifically
 * designed to feature atomic (lock-less) writing,
 * thus allowing it to be used from IRQ handler.
 * NOTE: Technically, writing isn't fully atomic, but by disabling
 *       interrupts in strategic places, it effectively behaves as such.
 */
struct atomic_iobuffer {
 struct sig          aib_avail;  /*< Signal send when data becomes available. */
 size_t              aib_size;   /*< [const] Size of the buffer. */
 byte_t             *aib_buffer; /*< [const][1..ATOMIC_IOBUFFER_SIZE(self)] The fixed-address (likely statically allocated) buffer memory. */
 ATOMIC_DATA byte_t *aib_rpos;   /*< [1..1][in(aib_buffer)][cyclic:<=aib_apos] Current read-pointer. */
 ATOMIC_DATA byte_t *aib_wpos;   /*< [1..1][in(aib_buffer)][cyclic:>=aib_rpos] Current write-pointer. */
 ATOMIC_DATA byte_t *aib_apos;   /*< [1..1][in(aib_buffer)][cyclic:<=aib_wpos] Current avail-pointer (lazily updated as data is written). */
#ifdef CONFIG_SMP
 atomic_rwlock_t     aib_using;  /*< Lock used to reset the buffer into its empty state. */
#endif /* CONFIG_SMP */
};

#define ATOMIC_IOBUFFER_INIT(buffer)      ATOMIC_IOBUFFER_INIT_EX(buffer,sizeof(buffer))
#ifdef CONFIG_SMP
#define ATOMIC_IOBUFFER_INIT_EX(buf,siz) \
  {SIG_INIT,siz,(byte_t *)(buf),((byte_t *)(buf))+(siz),(byte_t *)(buf),(byte_t *)(buf),ATOMIC_RWLOCK_INIT}
#else
#define ATOMIC_IOBUFFER_INIT_EX(buf,siz) \
  {SIG_INIT,siz,(byte_t *)(buf),((byte_t *)(buf))+(siz),(byte_t *)(buf),(byte_t *)(buf)}
#endif
#define ATOMIC_IOBUFFER_SIZE(self)    (self)->aib_size
#define ATOMIC_IOBUFFER_MAXREAD(self,rpos,apos) \
 ((rpos) < (apos) ? (size_t)((apos)-(rpos)) : \
  (ATOMIC_IOBUFFER_SIZE(self)-(size_t)((rpos)-(apos))))
#define ATOMIC_IOBUFFER_MAXWRITE(self,rpos,wpos) \
 ((rpos) >= (wpos) ? (size_t)((rpos)-(wpos)) : \
  (ATOMIC_IOBUFFER_SIZE(self)-(size_t)((wpos)-(rpos))))

/* Read data from a given atomic I/O buffer.
 * @return: * :      The actual amount of bytes read. (<= bufsiz)
 * @return: -EFAULT: A faulty pointer was given. */
FUNDEF ssize_t KCALL atomic_iobuffer_read(struct atomic_iobuffer *__restrict self,
                                         USER void *buf, size_t bufsiz, bool blocking);

/* Write data to a given atomic I/O buffer.
 * NOTE: Unlike `atomic_iobuffer_read()', this function _only_ acts on kernel memory.
 * WARNING: Try not to write too much data at once.
 *          Any data that there was no space for isn't written...
 * WARNING: The caller is responsible for synchronizing calls to
 *          this function, as two interlocked calls will still
 *          complete successfully, though written data may have
 *          been corrupted.
 * @return: * : The actual amount of bytes written (<= bufsiz)
 */
FUNDEF size_t KCALL
atomic_iobuffer_kwrite(struct atomic_iobuffer *__restrict self,
                       HOST void const *__restrict buf, size_t bufsiz);


DECL_END

#endif /* !GUARD_INCLUDE_FS_ATOMIC_IOBUFFER_H */
