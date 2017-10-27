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
#ifndef GUARD_INCLUDE_SYNC_SIG_H
#define GUARD_INCLUDE_SYNC_SIG_H 1

/* When enabled, signals are only the size of a single pointer!
 * (Why would you ever disable this? - The scheduler gets a bit more complicated; that's why.) */
#ifndef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
#define CONFIG_SIGNAL_USING_ATOMIC_RWPTR 1
#endif

#include <errno.h>
#include <hybrid/list/list.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
#include <hybrid/sync/atomic-rwptr.h>
#else
#include <hybrid/sync/atomic-rwlock.h>
#endif
#ifndef __INTELLISENSE__
#include <stddef.h>
#endif

DECL_BEGIN

#ifdef __CC__
struct sig;
struct task;
struct sigset;
#endif /* !__CC__ */

#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
#define SIG_OFFSETOF_TASK  0
#define SIG_SIZE           __SIZEOF_POINTER__
#else
#define SIG_OFFSETOF_LOCK  0
#define SIG_OFFSETOF_TASK  ATOMIC_RWLOCK_SIZE
#define SIG_SIZE          (SIG_OFFSETOF_TASK+__SIZEOF_POINTER__)
#endif

#ifdef __CC__

/*
 * === The Signal (Invented by Griefer@Work; Yes I claim this one for myself) ===
 *
 * In KOS, a signal is _the_ lowest-level existing synchronization primitive,
 * in theory making it the only ~true~ primitive, as everything else is
 * merely a derivative of it, usually adding some functionality or
 * re-interpreting its existing features.
 *
 * In layman's terms, or using existing terminology, a signal is somewhat
 * of a hybrid between a monitor (at least I think it is. - I've never
 * actually used a monitor even once in my lifetime thus far), and a
 * condition variable (though still allowing you to go further and run
 * any kind of code at that critical point at which a regular condition
 * variable would unlock the mutex it's been given, essentially allowing
 * you to use any kind of other locking mechanism as the lock of a
 * condition variable)
 *
 * A most detailed explanation on the semantics of how signals operate
 * is, though truly primitive, somewhat hard to explain, with there not
 * really being something that is exactly what me and KOS calls a signal.
 * But the first thing I should mention, is that a signal, as versatile
 * and powerful as it is, isn't suitable for exposure to anyone.
 * 
 * While the APIs provided within this header tries to prevent and/or
 * detect invalid signal states that would result in the scheduler
 * deadlocking, or even worse: breaking all-together, it is very much
 * possible to cause a lot of damage by locking a signal at the
 * right (or rather wrong) time.
 *
 * With that in mind, you must never expose the true, raw nature of a
 * pure signals to a user (unless your interface doesn't allow users
 * to run arbitrary, or dangerous code while holding a signal lock).
 *
 * === Enough Theory. Explain how they work! ===
 *
 * The way a signal functions in quite basic. - Instead of defining some
 * value (such as used by a semaphore) or a binary state (like a mutex has),
 * a signal does no such thing. While KOS signals still allow you to
 * lock them, those locks are only required for an internal insurance
 * that can be used to keep a signal from being send while a thread
 * is being scheduled for receiving said signal.
 * 
 * === Receive, Send? I though these were about multitasking, not networking! ===
 *
 * You are absolutely correct. I think it is time for me to pull out
 * the most important bits of what a signal does before getting back
 * to the semantics of signal locking later:
 * 
 * >> void sig_recv(struct sig *s);
 * >> void sig_send(struct sig *s);
 *
 * This is a simplified version of a signal's most basic functionality.
 * The behavior is as follows.
 * signal_recv:
 *   The calling thread is suspended (unscheduled) until it is
 *   re-awakened by another thread that is sending a signal.
 * signal_send:
 *   Send a signal to some unspecified set of receiving threads
 *  (e.g.: all of them, one of them, etc.), waking them and causing
 *   them to resume execution by causing `task_waitfor()' to return
 *   a pointer to the signal that was send.
 *
 * === OH! I get it. But I'm spotting a gaping design flaw.
 *     You get a race condition because you can't synchronize
 *     when a signal is send vs. when it is received. ===
 *
 * You are right again. When I originally came up with signals, I too
 * though this was an obstacle impossible to overcome. To be honest,
 * I came up with the idea of a primitive as simple as a signal back
 * around 2010, but scrapped it because of this exact problem.
 * The solution has to do with the aforementioned locking of signal,
 * and is as follow (written in code to better illustrate the flow):
 *
 * >> void sig_recv(struct sig *s) {
 * >>     // Lock the signal for writing, implemented as an
 * >>     // atomic read-write-lock, using atomics
 * >>     sig_write(s);
 * >>     
 * >>     // Do some synchronization stuff, like higher-level primitive
 * >>     // checks. e.g.: If 's' is part of a mutex, can that mutex
 * >>     // be locked, or must we wait for some other thread to
 * >>     // unlock it first.
 * >>     
 * >>     // Unschedule the current thread and unlock the signal
 * >>     // NOTE: The magical part of this is that the signal will only be
 * >>     //       unlocked _AFTER_ the calling thread has already reached the
 * >>     //       point at which is is also ready to receive the same signal.
 * >>     MAGICALLY_UNSCHEDULE_CURRENT_THREAD_AND_UNLOCK_SIGNAL(s);
 * >>     // Once we're here, the signal was send, at which point we can
 * >>     // run more higher-level primitive-specific code.
 * >>     // e.g.: If 's' is part of a mutex, start over and
 * >>     //       check if we are able to lock it now.
 * >> }
 *
 * >> void sig_send(struct sig *s) {
 * >>     // Lock the signal for writing, thus preventing any
 * >>     // new threads from being added to the receiver queue
 * >>     // modifying any data that is protected by the signal.
 * >>     sig_write(s);
 * >>     
 * >>     // At this point do some more higher-level primitive task.
 * >>     // e.g.: A mutex would mark itself as unlocked, allowing
 * >>     //       other threads to acquire things like unique locks.
 * >>     
 * >>     // Reschedule a set of waiting tasks.
 * >>     RESCHEDULE_WAITING_TASKS(s);
 * >>     
 * >>     // unlock the signal.
 * >>     sig_endwrite(s);
 * >> }
 * 
 * === OK! That makes sense. You're system appears to work.
 *     But what about fairness? From what I understand, the
 *     way your mutex works still relies on try-and-failure. ===
 * 
 * First and foremost: I'd like to mention that when it comes to
 * multi-threading I've got quite some experience under my belt.
 * And let me promise you: fairness is completely over-rated and
 * should be the least of your concerns when trying to implement
 * an air-tight, multi-threaded system...
 * 
 * === So your saying that your system isn't fair? ===
 * 
 * Don't interrupt me... No that's not what I'm saying at all, but
 * it's like almost midnight and I have no idea why I'm even doing
 * this whole bi-polar dialog with myself within the documentation
 * of this thing I came up with...
 * 
 * The system remains fair because you can control how many threads
 * a signal should be send to when actually sending it.
 * In the case of a mutex, you can easily achieve fairness by only
 * sending (and therefor waking) up to one thread, where signals are
 * intentionally designed to track the order in which threads originally
 * started receiving them, thus allowing you to simply state:
 * >> void mutex_unlock(struct mutex *m) {
 * >>     sig_write(&m->signal);
 * >>     MARK_MUTEX_AS_NOT_ACQUIRED(m);
 * >>     RESCHEDULE_THREAD_NEXT_IN_LINE_BASED_ON_ORDER_OF_PRIO_UNSCHEDULING(&m->signal);
 * >>     sig_endwrite(&m->signal);
 * >> }
 *
 * === Awesome sauce! - And don't worry. You're not bi-polar.
 *     I know I'm just a figment of your imagination, meant to
 *     keep our deer reader interested while hopefully getting
 *     them to learn something along the way ===
 * 
 * See you `round!
 */


/*
 * === Advanced Signal functionality ===
 *
 * Assuming you've already read and understood my 'The Signal'
 * article above, the following will go further in-depth on
 * advanced signal functionality. Most of this is already partially
 * explained within the documentations of individual API function,
 * though this article will try to explain 'Why Choose Signal?'
 * 
 * A signal, though fully packed with functionality that can
 * be implemented without the need of extending its raw data
 * structure, only requires one (specially aligned) pointer.
 * >> On 32-bit platforms this evaluates to <4 bytes>
 * >> On 64-bit platforms this evaluates to <8 bytes>
 *
 * === Cool. But looking down there, I'm spotting those
 *     vrecv() and vsend() functions. - What their deal? ===
 *
 * One of the shortcoming of a signal is that it's nothing
 * but a meaningless Impulse. And while getting an Impulse
 * usually is all you need, what if you wanted to include
 * some special information to be told to who-ever is
 * getting hit by it.
 * 
 * You can use what I call vsend/vrecv (The 'v' stands for `value') to
 * include some Impulse-specific data to be transferred to whoever is
 * on the receiving end of said Impulse (Then often referred to as 'v-signal').
 *
 * An extension to signals, these functions allow tasks to wait
 * for a signal that broadcasts/generates a value alongside the Impulse.
 * HINT: To achieve this, KOS uses the `t_sigval' pointer with the ktask structure.
 * 
 * - Using vsend/vrecv, you can easily create a signal-subsystem that
 *   is capable of some interesting things, such as knowing which signal
 *   was used to wake a given task, and so on.
 *
 * == NOTE: All of the following text describes KOS-specific caveats
 *          and implementation details of v-signals.
 *
 * WARNING: In order to safely ensure that the recipient of a signal
 *          has actually received it, the recipient must receive it within
 *          a task-level critical block (task_crit()/task_endcrit()).
 *          Critical blocks based on nointerrupt sections will not suffice,
 *          as interrupts are re-enabled during preemption, essentially
 *          allowing a potential receiver to be terminated before they
 *          have a chance of processing data (in case processing of said
 *          data is required, such as when a reference to some object,
 *          or a newly allocated chunk of memory is passed using this
 *          mechanism)
 *
 * HINT: This mechanism can easily be used to implement
 *       unbuffered input from sources such as a keyboard:
 *       Simply setup an interrupt handler to vsend some signal,
 *       alongside data (such as a keyboard stroke) and whenever
 *       you want to start acting on such data, just vrecv the signal.
 *       Obviously you'd need to add additional protection to prevent
 *       the signal from being broadcast while not everyone is back
 *       at it and receiving, but even that is kind-of just a bonus
 *       feature if loss-less processing is also a requirement.
 *       Such functionality can be achieved by using an atomic I/O buffer.
 *      ("/src/include/fs/atomic-iobuffer.h")
 */
struct sig {
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
 atomic_rwptr_t                s_task; /*< [0..1][TYPE(LIST_HEAD(struct tasksigslot))]
                                        *   Chain of tasks receiving this signal. */
#else
 atomic_rwlock_t               s_lock; /*< Signal lock. */
 LIST_HEAD(struct tasksigslot) s_task; /*< [0..1][lock(s_lock)] Chain of tasks receiving this signal. */
#endif
};

#define DEFINE_SIG(name)       struct sig name = SIG_INIT
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
#define SIG_INIT              {ATOMIC_RWPTR_INIT(NULL)}
#define sig_cinit(x)           atomic_rwptr_cinit(&(x)->s_task)
#define sig_init(x)            atomic_rwptr_init(&(x)->s_task)
#define SIG_GETTASK(x)       ((LIST_HEAD(struct tasksigslot))ATOMIC_RWPTR_GET((x)->s_task))
#else
#define SIG_INIT              {ATOMIC_RWLOCK_INIT,NULL}
#define sig_cinit(x)          (atomic_rwlock_cinit(&(x)->s_lock),assert((x)->s_task == NULL))
#define sig_init(x)     (void)(atomic_rwlock_init(&(x)->s_lock),(x)->s_task = NULL)
#define SIG_GETTASK(x)        (x)->s_task
#endif

/* Signal locking control */
#ifdef CONFIG_SIGNAL_USING_ATOMIC_RWPTR
#define sig_reading(x)     atomic_rwptr_reading(&(x)->s_task)
#define sig_writing(x)     atomic_rwptr_writing(&(x)->s_task)
#define sig_tryread(x)     atomic_rwptr_tryread(&(x)->s_task)
#define sig_trywrite(x)    atomic_rwptr_trywrite(&(x)->s_task)
#define sig_tryupgrade(x)  atomic_rwptr_tryupgrade(&(x)->s_task)
#define sig_read(x)        atomic_rwptr_read(&(x)->s_task)
#define sig_write(x)       atomic_rwptr_write(&(x)->s_task)
#define sig_upgrade(x)     atomic_rwptr_upgrade(&(x)->s_task)
#define sig_downgrade(x)   atomic_rwptr_downgrade(&(x)->s_task)
#define sig_endread(x)     atomic_rwptr_endread(&(x)->s_task)
#define sig_endwrite(x)    atomic_rwptr_endwrite(&(x)->s_task)
#else
#define sig_reading(x)     atomic_rwlock_reading(&(x)->s_lock)
#define sig_writing(x)     atomic_rwlock_writing(&(x)->s_lock)
#define sig_tryread(x)     atomic_rwlock_tryread(&(x)->s_lock)
#define sig_trywrite(x)    atomic_rwlock_trywrite(&(x)->s_lock)
#define sig_tryupgrade(x)  atomic_rwlock_tryupgrade(&(x)->s_lock)
#define sig_read(x)        atomic_rwlock_read(&(x)->s_lock)
#define sig_write(x)       atomic_rwlock_write(&(x)->s_lock)
#define sig_upgrade(x)     atomic_rwlock_upgrade(&(x)->s_lock)
#define sig_downgrade(x)   atomic_rwlock_downgrade(&(x)->s_lock)
#define sig_endread(x)     atomic_rwlock_endread(&(x)->s_lock)
#define sig_endwrite(x)    atomic_rwlock_endwrite(&(x)->s_lock)
#endif

/* Receive a a signal, only returning once it is send, or `abstime' expires.
 * NOTE: '*_endwrite' will unlock an exclusive (write) lock on the
 *       signal, while the other functions are simply wrappers that
 *       acquire said lock before calling the '*_endwrite' versions.
 * HINT: 'JTIME_INFINITE' may be passed for `abstime' to use an infinite timeout.
 * @return: -EOK:       The signal was successfully received.
 * @return: -ETIMEDOUT: The given timeout has expired.
 * @return: -EINTR:     The calling thread was interrupted. */
FUNDEF errno_t KCALL sig_timedrecv(struct sig *__restrict sig, jtime_t abstime);
FUNDEF errno_t KCALL sig_timedrecv_endwrite(struct sig *__restrict sig, jtime_t abstime);
FUNDEF errno_t KCALL sig_vtimedrecv(struct sig *__restrict sig, USER void *msg_buf, size_t bufsize, jtime_t abstime);
FUNDEF errno_t KCALL sig_vtimedrecv_endwrite(struct sig *__restrict sig, USER void *msg_buf, size_t bufsize, jtime_t abstime);
#ifdef __INTELLISENSE__
FUNDEF errno_t KCALL sig_recv(struct sig *__restrict sig);
FUNDEF errno_t KCALL sig_recv_endwrite(struct sig *__restrict sig);
FUNDEF errno_t KCALL sig_vrecv(struct sig *__restrict sig, USER void *msg_buf, size_t bufsize);
FUNDEF errno_t KCALL sig_vrecv_endwrite(struct sig *__restrict sig, USER void *msg_buf, size_t bufsize);
#else
#define sig_recv(sig)                           sig_timedrecv(sig,JTIME_INFINITE)
#define sig_recv_endwrite(sig)                  sig_timedrecv_endwrite(sig,JTIME_INFINITE)
#define sig_vrecv(sig,msg_buf,bufsize)          sig_vtimedrecv(sig,msg_buf,bufsize,JTIME_INFINITE)
#define sig_vrecv_endwrite(sig,msg_buf,bufsize) sig_vtimedrecv_endwrite(sig,msg_buf,bufsize,JTIME_INFINITE)
#endif

/* Send the specified signal to at most `max_threads' threads,
 * returning the amount of threads that received the signal. */
FUNDEF size_t KCALL sig_send(struct sig *__restrict sig, size_t max_threads);
FUNDEF size_t KCALL sig_send_unlocked(struct sig *__restrict sig, size_t max_threads);
FUNDEF size_t KCALL sig_vsend(struct sig *__restrict sig, void *msg, size_t msgsize, size_t max_threads);
FUNDEF size_t KCALL sig_vsend_unlocked(struct sig *__restrict sig, void *msg, size_t msgsize, size_t max_threads);
#ifdef __INTELLISENSE__
FUNDEF size_t KCALL sig_broadcast(struct sig *__restrict sig);
FUNDEF size_t KCALL sig_broadcast_unlocked(struct sig *__restrict sig);
FUNDEF size_t KCALL sig_vbroadcast(struct sig *__restrict sig, void *msg, size_t msgsize);
FUNDEF size_t KCALL sig_vbroadcast_unlocked(struct sig *__restrict sig, void *msg, size_t msgsize);
#else
#define sig_broadcast(sig)                       sig_send(sig,(size_t)-1)
#define sig_broadcast_unlocked(sig)              sig_send_unlocked(sig,(size_t)-1)
#define sig_vbroadcast(sig,msg,msgsize)          sig_vsend(sig,msg,msgsize,(size_t)-1)
#define sig_vbroadcast_unlocked(sig,msg,msgsize) sig_vsend_unlocked(sig,msg,msgsize,(size_t)-1)
#endif

/* Wake the first task from `sig'
 * NOTE: The caller is responsible for holding a write-lock on `sig'.
 * @return: true:  Successfully woken one task.
 * @return: false: No more tasks left to wake. */
FUNDEF bool KCALL
sig_vsendone_unlocked(struct sig *__restrict sig,
                      void const *data, size_t datasize);

#endif /* !__CC__ */


DECL_END

#endif /* !GUARD_INCLUDE_SYNC_SIG_H */
