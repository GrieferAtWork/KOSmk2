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
#ifndef GUARD_KERNEL_FS_PTY_C
#define GUARD_KERNEL_FS_PTY_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <bits/signum.h>
#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/pty.h>
#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/irq.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <limits.h>
#include <sched/cpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bits/poll.h>

DECL_BEGIN

/* The Minor device number used for allocating the next PTY device. */
PRIVATE ATOMIC_DATA minor_t next_pty_minor_id = 0;

#if 1 /* Full-blocking write (May cause slave apps to hang if the driver crashes) */
#define PTY_WRITE_BLOCKING_MODE   IO_BLOCKALL
#else
#define PTY_WRITE_BLOCKING_MODE   IO_BLOCKNONE
#endif

PRIVATE ssize_t KCALL /* Flush all stored canon data. */
pty_dump_canon(struct ptymaster *__restrict self) {
 ssize_t result; struct canon can;
 can = canonbuffer_capture(&self->pm_canon);
 HOSTMEMORY_BEGIN {
  result = iobuffer_write(&self->pm_m2s,can.c_data,can.c_size,IO_BLOCKNONE);
 }
 HOSTMEMORY_END;
 canonbuffer_release(&self->pm_canon,can);
 return result;
}

PRIVATE SAFE errno_t KCALL /* PTY standard IO-control. */
pty_ioctl(struct ptymaster *__restrict self, int name, USER void *arg) {
 errno_t result = -EOK;
 assert(TASK_ISSAFE());
 CHECK_HOST_DOBJ(self);
 switch (name) {

 case TCGETS: case TCGETA:
  atomic_rwlock_read(&self->pm_lock);
  if (copy_to_user(arg,&self->pm_ios,sizeof(struct termios)))
      result = -EFAULT;
  atomic_rwlock_endread(&self->pm_lock);
  break;

 { struct termios new_ios;
 case TCSETS:  case TCSETA:
 case TCSETSW: case TCSETAW:
 case TCSETSF: case TCSETAF:
  if (copy_from_user(&new_ios,arg,sizeof(struct termios)))
      return -EFAULT;
  atomic_rwlock_read(&self->pm_lock);

  if (!(new_ios.c_lflag&ICANON) && (self->pm_ios.c_lflag&ICANON)) {
   /* When switching out of line-buffered mode, first dump the canon. */
   result = pty_dump_canon(self);
  }
  if (E_ISOK(result)) {
   /* Re-check for a canon-dump if we've failed to upgrade the lock. */
   if (!atomic_rwlock_upgrade(&self->pm_lock) &&
       !(new_ios.c_lflag&ICANON) && (self->pm_ios.c_lflag&ICANON))
        result = pty_dump_canon(self);
   /* Copy the new IOs data structure into the PTY driver. */
   if (E_ISOK(result))
       memcpy(&self->pm_ios,&new_ios,sizeof(struct termios));
   atomic_rwlock_endwrite(&self->pm_lock);
  } else {
   atomic_rwlock_endread(&self->pm_lock);
  }
 } break;

 case TCFLSH:
  /* Discard data received, but not read (aka. what the driver send) */
  if ((uintptr_t)arg != TCOFLUSH) iobuffer_discard(&self->pm_m2s);
  /* Discard data written, but not send (aka. what the slave told to the driver). */
  if ((uintptr_t)arg != TCIFLUSH) iobuffer_discard(&self->pm_s2m);
  break;

 case TIOCGWINSZ:
  atomic_rwlock_read(&self->pm_lock);
  if (copy_to_user(arg,&self->pm_size,sizeof(struct winsize)))
      result = -EFAULT;
  atomic_rwlock_endread(&self->pm_lock);
  break;

 {
  struct task *t;
  struct winsize new_winsize;
 case TIOCSWINSZ:
  if (copy_from_user(&new_winsize,arg,sizeof(struct winsize)))
      return -EFAULT;
  atomic_rwlock_write(&self->pm_lock);
  /* Check if the window size actually changed. */
  if (memcmp(&new_winsize,&self->pm_size,sizeof(struct winsize)) != 0) {
   memcpy(&self->pm_size,&new_winsize,sizeof(struct winsize));
   atomic_rwlock_downgrade(&self->pm_lock);
   if ((t = self->pm_fproc) != NULL &&
       !TASK_TRYINCREF(t)) t = NULL;
   atomic_rwlock_endread(&self->pm_lock);
   if (t) {
    result = task_kill(self->pm_fproc,SIGWINCH);
    TASK_DECREF(t);
   }
  } else {
   atomic_rwlock_endwrite(&self->pm_lock);
  }
 } break;

 {
  pid_t pid;
  WEAK REF struct task *newproc;
  WEAK REF struct task *oldproc;
 case TIOCSPGRP:
  if (copy_from_user(&pid,arg,sizeof(pid_t)))
      return -EFAULT;
  if (pid == 0) newproc = THIS_TASK,TASK_WEAK_INCREF(newproc);
  else newproc = pid_namespace_lookup_weak(THIS_NAMESPACE,pid);
  if unlikely(!newproc) return -ESRCH;

  /* Use the group leader as foreground process target. */
  oldproc = newproc->t_pid.tp_leader;
  TASK_WEAK_INCREF(oldproc);
  TASK_WEAK_DECREF(newproc);
  newproc = oldproc;

  atomic_rwlock_write(&self->pm_lock);
  syslog(LOG_INFO,"[PTY] Set group %I32d (%I32d) (%p)\n",
         TASK_GETPID(newproc),pid,newproc);
  oldproc = self->pm_fproc;
  self->pm_fproc = newproc; /* Inherit reference. */
  atomic_rwlock_endwrite(&self->pm_lock);

  if (oldproc)
      TASK_WEAK_DECREF(oldproc);
  break;
 }

 {
  pid_t respid;
 case TIOCGPGRP:
  respid = -ESRCH;
  atomic_rwlock_read(&self->pm_lock);
  if (self->pm_fproc) {
   respid = self->pm_fproc->t_pid.tp_ids[PIDTYPE_PID].tl_ns == THIS_NAMESPACE
          ? self->pm_fproc->t_pid.tp_ids[PIDTYPE_PID].tl_pid : 0;
  }
  atomic_rwlock_endread(&self->pm_lock);
  if (copy_to_user(arg,&respid,sizeof(pid_t)))
      result = -EFAULT;
  break;
 }

 /* XXX: All the other ioctls? */

 default:
  result = -EINVAL;
  break;
 }
 return result;
}

PRIVATE ATTR_USED ssize_t KCALL
master_s2m_write_crlf(struct ptymaster *__restrict self,
                      USER void const *buf, size_t bufsize) {
 char const *iter,*end,*flush_start;
 ssize_t result = 0,temp; size_t partsize;
 if (!addr_isuser(buf,bufsize)) return -EFAULT;
 /* Must convert '\n' to '\r\n' (LF -> CRLF; 10 -> 13,10) */
 end = (flush_start = iter = (char const *)buf)+(bufsize/sizeof(char));
 for (;;) {
  if (iter == end || *iter == '\n') {
   PRIVATE char const crlf[] = {'\r','\n'};
   partsize = (size_t)(iter-flush_start)*sizeof(char);
   temp = iobuffer_write(&self->pm_s2m,flush_start,partsize,PTY_WRITE_BLOCKING_MODE);
   if (E_ISERR(temp)) return temp;
   result += temp;
   if (iter == end || (size_t)temp != partsize) break;
   HOSTMEMORY_BEGIN {
    temp = iobuffer_write(&self->pm_s2m,crlf,sizeof(crlf),PTY_WRITE_BLOCKING_MODE);
   }
   HOSTMEMORY_END;
   if (E_ISERR(temp)) return temp;
   result += temp;
   if (iter == end || (size_t)temp != partsize) break;
   if (partsize != sizeof(sizeof(crlf))) break;
   flush_start = iter+1;
  }
  ++iter;
 }
 return result;
}

PRIVATE ATTR_USED ssize_t KCALL
master_s2m_write(struct ptymaster *__restrict self,
                 USER void const *buf, size_t bufsize) {
 if (!(ATOMIC_READ(self->pm_ios.c_oflag)&ONLCR))
     return iobuffer_write(&self->pm_s2m,buf,bufsize,PTY_WRITE_BLOCKING_MODE);
 return call_user_worker(&master_s2m_write_crlf,3,self,buf,bufsize);
}

#define PRINT(p,s) \
{ size_t canon_size = (s); \
  temp = canonbuffer_write(&self->pm_canon,p,canon_size); \
  if (E_ISERR(temp)) return result; \
  result += temp; \
  if (lflags&ECHO) { \
   temp = master_s2m_write(self,p,canon_size); \
   if (E_ISERR(temp)) return temp; \
  } \
}


PRIVATE char const erase[] = {'\b',' ','\b'};
PRIVATE ssize_t
master_canon_write_impl(struct ptymaster *__restrict self,
                        USER void const *buf, size_t bufsize) {
 ssize_t result = 0,temp; size_t n_clear;
 cc_t const *iter,*end,*canon_start;
 tcflag_t lflags = ATOMIC_READ(self->pm_ios.c_lflag);
 if (!addr_isuser(buf,bufsize)) return -EFAULT;
 /* Must use the line buffer, as well as check for control characters */
 end = (canon_start = iter = (cc_t const *)buf)+bufsize;
 while (iter != end) {
  cc_t ch = (cc_t)*iter++;
  if (ch == (cc_t)'\n' ||
      ch == self->pm_ios.c_cc[VEOL] ||
      ch == self->pm_ios.c_cc[VEOL2]) {
   /* End-of-line. */
   PRINT(canon_start,(size_t)(iter-canon_start)*sizeof(cc_t));
   canon_start = iter;
   /* Dump the canon */
   temp = pty_dump_canon(self);
   if (E_ISERR(temp)) return temp;
  } else if (ch == self->pm_ios.c_cc[VKILL]) {
   /* Clear input line. */
   n_clear = canonbuffer_clear(&self->pm_canon);
handle_clear:
   if (n_clear && lflags & ECHO) {
    /* Mirror deleted data in the master terminal. */
    HOSTMEMORY_BEGIN {
     do temp = master_s2m_write(self,erase,sizeof(erase));
     while (E_ISOK(temp) && --n_clear);
    }
    HOSTMEMORY_END;
    if (E_ISERR(temp)) return temp;
   }
   canon_start = iter;
  } else if (ch == self->pm_ios.c_cc[VERASE]) {
   /* Flush input data, excluding the ERASE character itself. */
   if (iter-1 > canon_start)
       PRINT(canon_start,(size_t)((iter-1)-canon_start)*sizeof(cc_t));
   n_clear = canonbuffer_erase(&self->pm_canon,1);
   goto handle_clear;
  } else if (ch == self->pm_ios.c_cc[VINTR] ||
             ch == self->pm_ios.c_cc[VQUIT]) {
   int signum = ch == self->pm_ios.c_cc[VINTR] ? SIGINT : SIGKILL;
   REF struct task *fproc;
   if (iter-1 > canon_start)
       PRINT(canon_start,(size_t)((iter-1)-canon_start)*sizeof(cc_t));
   if (lflags & ECHO) {
    char out_text[2] = {'^','@'+ch};
    HOSTMEMORY_BEGIN {
#if 1 /* NOTE: We can simply use 'iobuffer_write()' here! */
     temp = iobuffer_write(&self->pm_s2m,out_text,sizeof(out_text),
                            PTY_WRITE_BLOCKING_MODE);
#else
     temp = master_s2m_write(self,out_text,sizeof(out_text));
#endif
    }
    HOSTMEMORY_END;
    if (E_ISERR(temp)) return temp;
   }
   /* Send a signal to the foreground process. */
   atomic_rwlock_read(&self->pm_lock);
   if ((fproc = self->pm_fproc) != NULL &&
       !TASK_TRYINCREF(fproc)) fproc = NULL;
   atomic_rwlock_endread(&self->pm_lock);
   if (fproc) {
    temp = task_kill(fproc,signum);
    TASK_DECREF(fproc);
    if (E_ISERR(temp)) return temp;
   }
   canon_start = iter;
  } else if (ch == self->pm_ios.c_cc[VEOF]) {
   struct canon can;
   /* Flush the canon, or interrupt a block-first read
    * operation originating from the pty slave process. */
   if (iter-1 > canon_start)
       PRINT(canon_start,(size_t)((iter-1)-canon_start)*sizeof(cc_t));
   can = canonbuffer_capture(&self->pm_canon);
   if (can.c_size)
    temp = iobuffer_interrupt(&self->pm_m2s);
   else {
    HOSTMEMORY_BEGIN {
     temp = iobuffer_write(&self->pm_m2s,can.c_data,can.c_size,IO_BLOCKNONE);
    }
    HOSTMEMORY_END;
   }
   canonbuffer_release(&self->pm_canon,can);
   if (E_ISERR(temp)) return temp;
   canon_start = iter;
  }
  /* XXX: Implement the following special characters.
   *     (s.a.: 'https://linux.die.net/man/3/termios') */
//#define VTIME    5
//#define VMIN     6
//#define VSWTC    7
//#define VSTART   8
//#define VSTOP    9
//#define VSUSP    10
//#define VREPRINT 12
//#define VDISCARD 13
//#define VWERASE  14
//#define VLNEXT   15

 }

 /* Flush all unwritten data. */
 if (iter != canon_start)
     PRINT(canon_start,(size_t)(iter-canon_start)*sizeof(cc_t));
 return result;
}
#undef PRINT



#define M ((struct ptymaster *)ino)
PRIVATE void KCALL master_fini(struct inode *__restrict ino) {
 syslog(LOG_DEBUG,"[PTY] Finalize PTY master controller\n");
 if (ino->i_ops == &ptymaster_ops) {
  /* Try to re-use this pty's id for the next call to 'openpty()' */
  minor_t min = MINOR(DEVICE_ID(&M->pm_chr.cd_device));
  ATOMIC_CMPXCH(next_pty_minor_id,min+1,min);
 }
 /* Cleanup PTY data members. */
 iobuffer_fini(&M->pm_s2m);
 iobuffer_fini(&M->pm_m2s);
 canonbuffer_fini(&M->pm_canon);
 if (M->pm_cproc) TASK_WEAK_DECREF(M->pm_cproc);
 if (M->pm_fproc) TASK_WEAK_DECREF(M->pm_fproc);
 chrdev_fini(&M->pm_chr);
}
PRIVATE errno_t KCALL
master_stat(struct inode *__restrict ino,
            struct stat64 *__restrict statbuf) {
 ssize_t read_size;
 CHECK_HOST_DOBJ(ino);
 CHECK_HOST_DOBJ(statbuf);
 read_size = iobuffer_get_read_size(&M->pm_s2m);
 if (E_ISERR(read_size)) return (errno_t)read_size;
 statbuf->st_size64   = (pos64_t)read_size;
 statbuf->st_blocks64 = CEILDIV((blkcnt64_t)read_size,S_BLKSIZE);
 return -EOK;
}
#undef M





#define M ((struct ptymaster *)fp->f_node)
PRIVATE ssize_t KCALL
master_read(struct file *__restrict fp, USER void *buf, size_t bufsize) {
 ssize_t result;
 struct ptymaster *master;
 SUPPRESS_WAITLOGS_BEGIN();
 master = M;
 if (master->pm_chr.cd_device.d_node.i_state&INODE_STATE_CLOSING)
  result = 0; /* Don't read any more data if the slave was closed. */
 else {
  result = iobuffer_read(&M->pm_s2m,buf,bufsize,IO_BLOCKFIRST);
  assertf(result != 0 ||
         (ATOMIC_READ(master->pm_chr.cd_device.d_node.i_state)&INODE_STATE_CLOSING),
          "The master PTY must only indicate EOF once being closed!");
 }
 SUPPRESS_WAITLOGS_END();
 return result;
}
PRIVATE ssize_t KCALL
master_write(struct file *__restrict fp, USER void const *buf, size_t bufsize) {
 struct ptymaster *self = M; tcflag_t flags;
 flags = ATOMIC_READ(self->pm_ios.c_lflag);
 if (!(flags&ICANON)) {
  if (flags&ECHO) {
   ssize_t temp = master_s2m_write(self,buf,bufsize);
   if (E_ISERR(temp)) return temp;
  }
  return iobuffer_write(&self->pm_m2s,buf,bufsize,PTY_WRITE_BLOCKING_MODE);
 }
 return call_user_worker(&master_canon_write_impl,3,self,buf,bufsize);
}
PRIVATE errno_t KCALL
master_ioctl(struct file *__restrict fp, int name, USER void *arg) {
 return pty_ioctl(M,name,arg);
}
PRIVATE pollmode_t KCALL
master_poll(struct file *__restrict fp, pollmode_t mode) {
 pollmode_t result = 0;
 struct ptymaster *master = M;
 /* Poll input data. */
 if (mode&POLLIN) {
  errno_t temp = -EOK;
  sig_write(&master->pm_s2m.ib_avail);
  if (master->pm_chr.cd_device.d_node.i_state&INODE_STATE_CLOSING)
   result |= POLLIN; /* Technically correct, but POLLIN must return once read() no longer blocks...
                      * Plus: This prevents a soft-lock when the master poll()s after the slave was close()d. */
  else if (IOBUFFER_MAXREAD(&master->pm_s2m,master->pm_s2m.ib_rpos))
   result |= POLLIN;
  else {
   temp = task_addwait(&master->pm_s2m.ib_avail,NULL,0);
  }
  sig_endwrite(&master->pm_s2m.ib_avail);
  if (E_ISERR(temp)) return temp;
 }
 /* Poll output data. */
 if (mode&POLLOUT) {
  errno_t temp = -EOK;
  sig_write(&master->pm_m2s.ib_nfull);
  if (IOBUFFER_MAXWRITE(&master->pm_m2s,master->pm_m2s.ib_rpos))
      result |= POLLOUT;
  else {
   temp = task_addwait(&master->pm_m2s.ib_nfull,NULL,0);
  }
  sig_endwrite(&master->pm_m2s.ib_nfull);
  if (E_ISERR(temp)) return temp;
 }

 if (!result && mode&(POLLERR|POLLPRI))
      result = -EWOULDBLOCK;
 return result;
}
#undef M

#define M (S->ps_master)
#define S  ((struct ptyslave *)fp->f_node)
PRIVATE ssize_t KCALL
slave_read(struct file *__restrict fp, USER void *buf, size_t bufsize) {
 ssize_t result;
 SUPPRESS_WAITLOGS_BEGIN();
 result = iobuffer_read(&M->pm_m2s,buf,bufsize,IO_BLOCKFIRST);
 SUPPRESS_WAITLOGS_END();
 return result;
}
PRIVATE ssize_t KCALL
slave_write(struct file *__restrict fp, USER void const *buf, size_t bufsize) {
 return master_s2m_write(M,buf,bufsize);
}
PRIVATE errno_t KCALL
slave_ioctl(struct file *__restrict fp, int name, USER void *arg) {
 return pty_ioctl(M,name,arg);
}
PRIVATE pollmode_t KCALL
slave_poll(struct file *__restrict fp, pollmode_t mode) {
 pollmode_t result = 0;
 struct ptymaster *master = M;
 /* Poll input data. */
 if (mode&POLLIN) {
  errno_t temp = -EOK;
  sig_write(&master->pm_m2s.ib_avail);
  if (IOBUFFER_MAXREAD(&master->pm_m2s,master->pm_m2s.ib_rpos))
      result |= POLLIN;
  else {
   temp = task_addwait(&master->pm_m2s.ib_avail,NULL,0);
  }
  sig_endwrite(&master->pm_m2s.ib_avail);
  if (E_ISERR(temp)) return temp;
 }
 /* Poll output data. */
 if (mode&POLLOUT) {
  errno_t temp = -EOK;
  sig_write(&master->pm_s2m.ib_nfull);
  if (IOBUFFER_MAXWRITE(&master->pm_s2m,master->pm_s2m.ib_rpos))
      result |= POLLOUT;
  else {
   temp = task_addwait(&master->pm_s2m.ib_nfull,NULL,0);
  }
  sig_endwrite(&master->pm_s2m.ib_nfull);
  if (E_ISERR(temp)) return temp;
 }

 if (!result && mode&(POLLERR|POLLPRI))
      result = -EWOULDBLOCK;
 return result;
}
#undef S
#undef M



PRIVATE void KCALL
pty_fclose(struct inode *__restrict UNUSED(ino),
           struct file *__restrict fp) {
 errno_t error; struct fsaccess ac; FSACCESS_SETHOST(ac);
 /* Delete the main associated directory entry. */
 error = dentry_remove(fp->f_dent,&ac,DENTRY_REMOVE_REG);
 if (E_ISOK(error))
  syslog(LOG_INFO,"[PTY] Deleted PTY device file '%[dentry]'\n",fp->f_dent);
 else {
  syslog(LOG_WARN,
         "[PTY] Failed to delete PTY device file '%[dentry]': %[errno]\n",
         fp->f_dent,-error);
 }
}

PRIVATE void KCALL slave_fini(struct inode *__restrict ino) {
 struct ptymaster *master;
 master = container_of(ino,struct ptyslave,ps_chr.cd_device.d_node)->ps_master;
 /* Set the closing bit in the master, thus preventing any further read()s. */
 ATOMIC_FETCHOR(master->pm_chr.cd_device.d_node.i_state,INODE_STATE_CLOSING);
 task_nointr();
 /* Interrupt a potential running read() operation in the master. */
 iobuffer_interrupt(&master->pm_s2m);
 task_endnointr();

 CHRDEV_DECREF(&master->pm_chr);
 chrdev_fini(&container_of(ino,struct ptyslave,ps_chr.cd_device.d_node)->ps_chr);
}
PRIVATE errno_t KCALL
slave_stat(struct inode *__restrict ino,
           struct stat64 *__restrict statbuf) {
 ssize_t read_size;
 struct ptymaster *master;
 CHECK_HOST_DOBJ(ino);
 CHECK_HOST_DOBJ(statbuf);
 master = container_of(ino,struct ptyslave,ps_chr.cd_device.d_node)->ps_master;
 CHECK_HOST_DOBJ(master);
 read_size = iobuffer_get_read_size(&master->pm_m2s);
 if (E_ISERR(read_size)) return (errno_t)read_size;
 statbuf->st_size64   = (pos64_t)read_size;
 statbuf->st_blocks64 = CEILDIV((blkcnt64_t)read_size,S_BLKSIZE);
 return -EOK;
}


/* PTY master/slave INode operations. */
PUBLIC struct inodeops const ptymaster_ops = {
    .ino_fopen  = &inode_fopen_default,
    .ino_fclose = &pty_fclose,
    .ino_fini   = &master_fini,
    .ino_stat   = &master_stat,
    .f_flags    = INODE_FILE_LOCKLESS,
    .f_read     = &master_read,
    .f_write    = &master_write,
    .f_ioctl    = &master_ioctl,
    .f_poll     = &master_poll,
};

PUBLIC struct inodeops const ptyslave_ops = {
    .ino_fopen  = &inode_fopen_default,
    .ino_fclose = &pty_fclose,
    .ino_fini   = &slave_fini,
    .ino_stat   = &slave_stat,
    .f_flags    = INODE_FILE_LOCKLESS,
    .f_read     = &slave_read,
    .f_write    = &slave_write,
    .f_ioctl    = &slave_ioctl,
    .f_poll     = &slave_poll,
};




PUBLIC struct ptymaster *KCALL
ptymaster_cinit(struct ptymaster *master) {
 if (master) {
  CHECK_HOST_DOBJ(master);
  chrdev_cinit(&master->pm_chr);
  master->pm_chr.cd_device.d_node.i_ops = &ptymaster_ops;
  iobuffer_cinit(&master->pm_s2m);
  iobuffer_cinit(&master->pm_m2s);
  /* NOTE: Use 'LINE_MAX' as default limit for the
   *       canon, because that's literally its purpose. */
  canonbuffer_cinit(&master->pm_canon,LINE_MAX);
  atomic_rwlock_cinit(&master->pm_lock);
  assert(master->pm_cproc == NULL);
  assert(master->pm_fproc == NULL);
  //master->pm_size = ...; /* Initialized by the caller */
  //master->pm_ios = ...; /* Initialized by the caller */
 }
 return master;
}


/* Max name sizes (including NUL-characters) for master/slave files under '/dev'. */
#define PTY_DEVICENAME_MAX 10
LOCAL size_t KCALL pty_drivername(char prefix, char *buf, minor_t id) {
 size_t result;
 if (id >= PTY_DEVCNT) {
  /* KOS Extension: Since we allocate '20' bits for minor device numbers,
   *                it's actually possible to get _much_ more than 256
   *                pty devices in KOS (1048576 to be exact).
   *             >> But for all those additional nodes, we need a new
   *                naming scheme that isn't standardized due to its
   *                nature of being an extension:
   * MASTER: /dev/ptyX12345
   * SLAVE:  /dev/ttyX12345
   * NOTE: The 12345 is the hex value of the minor device number, using lower-case letters.
   */
  STATIC_ASSERT(MINORBITS/4 == 5);
#define PTY_EXT_NDIGITS  5
  result = 4+PTY_EXT_NDIGITS;
  sprintf(buf,"%ctyX%.5I16x",prefix,(u16)id);
 } else {
  /* Old (BSD-style) PTY master/slave device names. */
  result = 5;
  buf[0] = prefix;
  buf[1] = 't';
  buf[2] = 'y';
  { char temp = 'p'+(id/16);
    if (temp > 'z') temp = 'a'+(temp-'z');
    buf[3] = temp;
  }
  id %= 16;
  buf[4] = id >= 10 ? 'a'+(id-10) : '0'+id;
  buf[5] = '\0';
 }
 return result;
}

PRIVATE REF struct dentry *KCALL
pty_add2dev(char prefix, struct device *__restrict self) {
 char buf[PTY_DEVICENAME_MAX];
 struct dentryname name; struct fsaccess ac;
 name.dn_name = buf;
 name.dn_size = pty_drivername(prefix,buf,MINOR(DEVICE_ID(self)));
 dentryname_loadhash(&name);
 FSACCESS_SETHOST(ac);
 return dentry_insnod(devfs_root,&name,&ac,self,NULL);
}

PUBLIC REF struct dentry *KCALL
ptymaster_add2dev(struct ptymaster *__restrict self) {
 return pty_add2dev('p',&self->pm_chr.cd_device);
}
PUBLIC REF struct dentry *KCALL
ptyslave_add2dev(struct ptyslave *__restrict self) {
 return pty_add2dev('t',&self->ps_chr.cd_device);
}


PUBLIC errno_t KCALL
pty_register(struct ptymaster *__restrict master,
             struct ptyslave *__restrict slave) {
 minor_t first_minor,pty_minor; errno_t error;
 CHECK_HOST_DOBJ(master);
 CHECK_HOST_DOBJ(slave);
 assert(slave->ps_master == master);
 first_minor = (minor_t)-1;
 for (;;) {
  /* Select */
  do {
   pty_minor = ATOMIC_READ(next_pty_minor_id);
   /* Return an error for the _absolutely_ unlikely case
    * that _ALL_ pty ids are taken. (This'll never happen...) */
   if unlikely(pty_minor == first_minor) return -ENOMEM;
  } while (!ATOMIC_CMPXCH_WEAK(next_pty_minor_id,pty_minor,
                               pty_minor >= (PTY_EXTCNT-1) ?
                               0 : pty_minor+1));
  error = CHRDEV_REGISTER(master,PTY_MASTER(pty_minor));
  if (E_ISERR(error)) {
   /* Stop on anything that isn't 'EEXIST' */
   if (error != -EEXIST) break;
  } else {
   error = CHRDEV_REGISTER(slave,PTY_SLAVE(pty_minor));
   if (E_ISOK(error)) break;
   /* Remove the master device after we've failed to add the slave. */
   devns_remove(&ns_chrdev,&master->pm_chr.cd_device,true);
   if (error != -EEXIST) break;
  }
  if (first_minor == (minor_t)-1)
      first_minor = pty_minor;
 }
 return error;
}



SYSCALL_LDEFINE3(xopenpty,USER char *,name,
                 USER struct termios const *,termp,
                 USER struct winsize const *,winp) {
 int fd_master,fd_slave = 0;
 REF struct ptymaster *master;
 REF struct ptyslave *slave;
 REF struct dentry *master_ent;
 REF struct dentry *slave_ent;
 struct fd fd_m,fd_s;
 task_crit();
 fd_master = -ENOMEM;
 master = ptymaster_new(sizeof(struct ptymaster));
 if unlikely(!master) goto end;
 slave = ptyslave_new(sizeof(struct ptyslave));
 if unlikely(!slave) goto err_free_master;
 assert(master->pm_chr.cd_device.d_node.i_refcnt == 1);
 master->pm_chr.cd_device.d_node.i_refcnt = 2; /* +1 for the slave->master pointer. */
 slave->ps_master = master;

 fd_master = -EFAULT;
 /* Copy PTY setup information (if provided). */
 if (termp) {
  if (copy_from_user(&master->pm_ios,termp,sizeof(struct termios)))
      goto err_free_master;
 } else {
  memset(&master->pm_ios,0,sizeof(struct termios));
  master->pm_ios.c_iflag      = (ICRNL|BRKINT);
  master->pm_ios.c_oflag      = (/*ONLCR|*/OPOST);
  master->pm_ios.c_lflag      = (ECHO|ECHOE|ECHOK|ICANON|ISIG|IEXTEN);
  master->pm_ios.c_cflag      = (CREAD);
  master->pm_ios.c_cc[VMIN]   =  1; /* Just use KIO_BLOCKFIRST-style behavior by default. */
  master->pm_ios.c_cc[VEOF]   =  4; /* ^D. */
  master->pm_ios.c_cc[VEOL]   =  0; /* Not set. */
  master->pm_ios.c_cc[VERASE] = '\b';
  master->pm_ios.c_cc[VINTR]  =  3; /* ^C. */
  master->pm_ios.c_cc[VKILL]  = 21; /* ^U. */
  master->pm_ios.c_cc[VQUIT]  = 28; /* ^\. */
  master->pm_ios.c_cc[VSTART] = 17; /* ^Q. */
  master->pm_ios.c_cc[VSTOP]  = 19; /* ^S. */
  master->pm_ios.c_cc[VSUSP]  = 26; /* ^Z. */
  master->pm_ios.c_cc[VTIME]  =  0;
 }
 if (winp) {
  if (copy_from_user(&master->pm_size,winp,sizeof(struct winsize)))
      goto err_free_master;
 } else {
  master->pm_size.ws_xpixel = master->pm_size.ws_col = 80;
  master->pm_size.ws_ypixel = master->pm_size.ws_row = 25;
 }

 /* Mark both the master & slave devices as weakly linked (Required for lazy finalization). */
 DEVICE_SETWEAK(&master->pm_chr.cd_device);
 DEVICE_SETWEAK(&slave->ps_chr.cd_device);

 /* Setup both the mast and slave device drivers.
  * >> Following this point, both devices are publically available. */
 asserte(E_ISOK(device_setup(&master->pm_chr.cd_device,THIS_INSTANCE)));
 asserte(E_ISOK(device_setup(&slave->ps_chr.cd_device,THIS_INSTANCE)));

 fd_master = pty_register(master,slave);
 if (E_ISERR(fd_master)) goto err;

 master_ent = ptymaster_add2dev(master);
 if (E_ISERR(master_ent)) { fd_master = E_GTERR(master_ent); goto err; }
 slave_ent = ptyslave_add2dev(slave);
 if (E_ISERR(slave_ent)) { fd_master = E_GTERR(slave_ent); goto err2; }

 if (name) {
  char buf[32]; size_t buflen; /* XXX: sprintf_user()? */
  buflen = snprintf(buf,sizeof(buf),"%[dentry]",master_ent);
  if (copy_to_user(name,buf,(buflen+1)*sizeof(char)))
  { fd_master = -EFAULT; goto err3; }
 }

 /* The devices are now registered and should be accessible through '/dev/pts' */

 /* Create file streams for the master and slave drivers. */
 fd_m.fo_obj.fo_file = inode_fopen_default(&master->pm_chr.cd_device.d_node,master_ent,O_RDWR);
 if (E_ISERR(fd_m.fo_obj.fo_file)) { fd_master = E_GTERR(fd_m.fo_obj.fo_file); goto err3; }
 fd_s.fo_obj.fo_file = inode_fopen_default(&slave->ps_chr.cd_device.d_node,slave_ent,O_RDWR);
 if (E_ISERR(fd_s.fo_obj.fo_file)) { fd_master = E_GTERR(fd_s.fo_obj.fo_file); goto err4; }
 fd_m.fo_ops = &fd_ops[FD_TYPE_FILE];
 fd_s.fo_ops = &fd_ops[FD_TYPE_FILE];

 /* Register the master and slave file descriptors. */
 { struct fdman *fdm = THIS_FDMAN;
   fd_master = fdman_write(fdm);
   if (E_ISERR(fd_master)) goto err5;
   fd_master = fdman_put_unlocked(fdm,fd_m);
   if (E_ISOK(fd_master)) {
    fd_slave = fdman_put_unlocked(fdm,fd_s);
    if (E_ISERR(fd_slave)) {
     struct fd temp;
     temp = fdman_del_unlocked(fdm,fd_master);
     assert(temp.fo_ptr == fd_m.fo_obj.fo_file);
     asserte(ATOMIC_DECFETCH(fd_m.fo_obj.fo_file->f_refcnt) >= 1);
     fd_master = fd_slave;
    }
   }
   fdman_endwrite(fdm);
   if (E_ISERR(fd_master)) goto err5;
 }
 /* All right! - Everything's registered and initialized. - We're done here. */
 FILE_DECREF(fd_s.fo_obj.fo_file);
 FILE_DECREF(fd_m.fo_obj.fo_file);
 DENTRY_DECREF(slave_ent);
 DENTRY_DECREF(master_ent);
end2:
 CHRDEV_DECREF(&slave->ps_chr);
 CHRDEV_DECREF(&master->pm_chr);
end:
 task_endcrit();
#if 0
 syslog(LOG_DEBUG,"[PTY] Created new drier %d, %d\n",fd_master,fd_slave);
#endif
 return (s64)fd_master | (s64)fd_slave << 32;
err5: FILE_DECREF(fd_s.fo_obj.fo_file);
err4: FILE_DECREF(fd_m.fo_obj.fo_file);
err3: DENTRY_DECREF(slave_ent);
err2: DENTRY_DECREF(master_ent);
err:  fd_slave = 0; goto end2;
/*err_free_slave:  free(slave);*/
err_free_master: free(master); goto end;
}



DECL_END

#endif /* !GUARD_KERNEL_FS_PTY_C */
