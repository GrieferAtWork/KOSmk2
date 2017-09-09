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
#ifndef GUARD_KERNEL_TEST_OWNER_RWLOCK_C
#define GUARD_KERNEL_TEST_OWNER_RWLOCK_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <kernel/test.h>
#include <sched/task.h>
#include <sync/owner-rwlock.h>

DECL_BEGIN

TEST(owner_rwlock) {
 DEFINE_OWNER_RWLOCK(lock);

 task_crit();
 asserte(!owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));

 /* Simple read. */
 asserte(owner_rwlock_tryread(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));
 asserte(owner_rwlock_tryread(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));
 owner_rwlock_endread(&lock);
 asserte(owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));
 owner_rwlock_endread(&lock);
 asserte(!owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));

 /* Simple write. */
 asserte(owner_rwlock_trywrite(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(owner_rwlock_writing(&lock));
 asserte(owner_rwlock_trywrite(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(owner_rwlock_writing(&lock));
 owner_rwlock_endwrite(&lock);
 asserte(owner_rwlock_reading(&lock));
 asserte(owner_rwlock_writing(&lock));
 owner_rwlock_endwrite(&lock);
 asserte(!owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));

 /* Simple read -> write upgrade. */
 asserte(owner_rwlock_tryread(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));
 asserte(owner_rwlock_tryupgrade(&lock) == -EOK);
 owner_rwlock_endwrite(&lock);
 asserte(!owner_rwlock_writing(&lock));

#if 0 /* Can't work due to double-upgrade deadlock */
 /* Recursive write after read. */
 asserte(owner_rwlock_tryread(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));
 asserte(owner_rwlock_trywrite(&lock) == -EOK);
 asserte(owner_rwlock_reading(&lock));
 asserte(owner_rwlock_writing(&lock));
 owner_rwlock_endwrite(&lock);
 asserte(!owner_rwlock_writing(&lock));
 asserte(owner_rwlock_reading(&lock));
 owner_rwlock_endread(&lock);
 asserte(!owner_rwlock_writing(&lock));
 asserte(!owner_rwlock_reading(&lock));
#endif

 /* Recursive read after write. */
 asserte(owner_rwlock_trywrite(&lock) == -EOK);
 asserte(owner_rwlock_writing(&lock));
 asserte(owner_rwlock_tryread(&lock) == -EOK);
 asserte(owner_rwlock_writing(&lock));
 asserte(owner_rwlock_tryupgrade(&lock) == -EOK);
 asserte(owner_rwlock_writing(&lock));
 owner_rwlock_endwrite(&lock);
 owner_rwlock_endwrite(&lock);

 asserte(!owner_rwlock_reading(&lock));
 asserte(!owner_rwlock_writing(&lock));

 task_endcrit();
}

DECL_END

#endif /* !GUARD_KERNEL_TEST_OWNER_RWLOCK_C */
