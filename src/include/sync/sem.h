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
#ifndef GUARD_INCLUDE_SYNC_SEM_H
#define GUARD_INCLUDE_SYNC_SEM_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <sched/cpu.h>
#include <sync/sig.h>

DECL_BEGIN

#ifdef __CC__
typedef size_t semcount_t;

typedef struct sem {
  struct sig s_signal; /*< Semaphore signal. */
  semcount_t s_ticket; /*< [lock(s_signal)] Semaphore ticket count. */
} sem_t;

#define SEM_INIT(n)       {SIG_INIT,n}
#define sem_init(self,n)  (void)(sig_init(&(self)->s_signal),(self)->s_ticket = (n))
#define sem_cinit(self,n) (sig_cinit(&(self)->s_signal),\
                          (__builtin_constant_p(n) && (n) == 0) ? \
                           assert((self)->s_ticket == 0) : \
                          (void) ((self)->s_ticket = (n)))

LOCAL semcount_t KCALL sem_release(sem_t *__restrict self, semcount_t num_tickets);
LOCAL errno_t KCALL sem_trywait(sem_t *__restrict self);
LOCAL errno_t KCALL sem_timedwait(sem_t *__restrict self, struct timespec const *abstime);
#ifdef __INTELLISENSE__
LOCAL errno_t KCALL sem_wait(sem_t *__restrict self);
#else
#define sem_wait(self)  sem_timedwait(self,NULL)
#endif


#if !defined(__INTELLISENSE__)
LOCAL semcount_t KCALL
sem_release(sem_t *__restrict self, semcount_t num_tickets) {
 semcount_t result; pflag_t was;
 was = PREEMPTION_PUSH();
 sig_write(&self->s_signal);
 result = ATOMIC_FETCHADD(self->s_ticket,num_tickets);
 sig_send_unlocked(&self->s_signal,num_tickets);
 sig_endwrite(&self->s_signal);
 PREEMPTION_POP(was);
 return result;
}
LOCAL errno_t KCALL
sem_trywait(sem_t *__restrict self) {
 semcount_t cnt;
 do {
  cnt = ATOMIC_READ(self->s_ticket);
  if (!cnt) return -EAGAIN;
 } while (!ATOMIC_CMPXCH_WEAK(self->s_ticket,cnt,cnt-1));
 return -EOK;
}
LOCAL errno_t KCALL
sem_timedwait(sem_t *__restrict self, struct timespec const *abstime) {
 errno_t error; pflag_t was;
 for (;;) {
  error = sem_trywait(self);
  if (E_ISOK(error)) break;
  was = PREEMPTION_PUSH();
  sig_write(&self->s_signal);
  COMPILER_READ_BARRIER();
  error = sem_trywait(self);
  if (E_ISOK(error)) {
   sig_endwrite(&self->s_signal);
   PREEMPTION_POP(was);
   break;
  }
  error = sig_timedrecv_endwrite(&self->s_signal,abstime);
  PREEMPTION_POP(was);
  if (E_ISERR(error)) break;
 }
 return error;
}
#endif /* !__INTELLISENSE__ */


#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_SYNC_SEM_H */
