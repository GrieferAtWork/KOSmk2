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
#ifndef GUARD_LIBS_LIBC_SYSTEM_H
#define GUARD_LIBS_LIBC_SYSTEM_H 1

#include <bits/siginfo.h>
#include <bits/sigset.h>
#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <hybrid/types.h>
#include <linux/unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>

#define TRACE_SYSTEM_CALLS 0

DECL_BEGIN

struct mmap_info;
struct stat64;
struct dirent;
struct termios;
struct winsize;
struct sigaction;
struct rusage;
struct ucontext;
struct pollfd;
struct timezone;

#ifndef __errno_t_defined
#define __errno_t_defined 1
typedef int errno_t;
#endif /* !__errno_t_defined */


#ifdef CONFIG_32BIT_TIME
#define kernel_timespec __timespec32
#define kernel_timeval  __timeval32
#else
#define kernel_timespec __timespec64
#define kernel_timeval  __timeval64
#endif


#define SET_SYSTEM_ERROR(expr) \
 XBLOCK({ SET_ERRNO(-(expr)); XRETURN -1; })
#define FORWARD_SYSTEM_ERROR(expr) \
 XBLOCK({ register errno_t _e = (expr); \
          XRETURN E_ISERR(_e) ? (SET_ERRNO(-(_e)),-1) : 0; })
#define FORWARD_SYSTEM_VALUE(expr) \
 XBLOCK({ register __typeof__(expr) _e = (expr); \
          XRETURN E_ISERR(_e) ? (SET_ERRNO((errno_t)-(_e)),-1) : _e; })



#if TRACE_SYSTEM_CALLS
#if 1
#define TRACE0()   \
 (void)(libc_syslog(LOG_DEBUG,"%s\n",__FUNCTION__))
#else
#define TRACE0()   \
 (void)(libc_syslog(LOG_DEBUG,"%s(%d) : %s()\n",__FILE__,__LINE__,__FUNCTION__))
#endif
#undef __SYSCALL_TRACE
#define __SYSCALL_TRACE(id) { if (id != __NR_xsyslog) TRACE0(); }
#endif /* TRACE_SYSTEM_CALLS */

#define SYSCALL0(type,name,decl) __SYSCALL_FUN(0,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)
#define SYSCALL1(type,name,decl) __SYSCALL_FUN(1,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)
#define SYSCALL2(type,name,decl) __SYSCALL_FUN(2,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)
#define SYSCALL3(type,name,decl) __SYSCALL_FUN(3,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)
#define SYSCALL4(type,name,decl) __SYSCALL_FUN(4,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)
#define SYSCALL5(type,name,decl) __SYSCALL_FUN(5,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)
#define SYSCALL6(type,name,decl) __SYSCALL_FUN(6,LIBCCALL,,type,__NR_##name,SYSCALL_NAME(name),decl)

/* Define all system calls as inline functions for later use. */
#define SYSCALL_NAME(x) sys_##x
LOCAL SYSCALL4(int,openat,(int,dfd,char const *,filename,oflag_t,flags,mode_t,mode));
LOCAL SYSCALL1(errno_t,close,(int,fd));
LOCAL SYSCALL1(void,exit,(int,exitcode));
LOCAL SYSCALL1(void,exit_group,(int,exitcode));
LOCAL SYSCALL1(errno_t,unshare,(int,flags));
LOCAL SYSCALL2(ssize_t,munmap,(void *,addr,size_t,len));
LOCAL SYSCALL5(void *,mremap,(VIRT void *,addr,size_t,old_len,size_t,new_len,int,flags,VIRT void *,new_addr));
LOCAL SYSCALL6(void *,mmap,(void *,addr,size_t,len,int,prot,int,flags,int,fd,syscall_ulong_t,off));
LOCAL SYSCALL3(ssize_t,read,(int,fd,USER void *,buf,size_t,bufsize));
LOCAL SYSCALL3(ssize_t,write,(int,fd,USER void const *,buf,size_t,bufsize));
LOCAL SYSCALL4(errno_t,faccessat,(int,dfd,USER char const *,filename,int,mode,int,flag));
LOCAL SYSCALL1(errno_t,chdir,(USER char const *,path));
LOCAL SYSCALL1(errno_t,chroot,(USER char const *,path));
LOCAL SYSCALL1(errno_t,fchdir,(int,fd));
LOCAL SYSCALL2(errno_t,fchmod,(int,fd,mode_t,mode));
LOCAL SYSCALL3(errno_t,fchown,(int,fd,uid_t,user,gid_t,group));
LOCAL SYSCALL3(errno_t,fchmodat,(int,dfd,USER char const *,filename,mode_t,mode));
LOCAL SYSCALL5(errno_t,fchownat,(int,dfd,USER char const *,filename,uid_t,user,gid_t,group,int,flag));
LOCAL SYSCALL1(int,dup,(int,fd));
LOCAL SYSCALL3(int,dup3,(int,newfd,int,oldfd,oflag_t,flags));
LOCAL SYSCALL3(syscall_slong_t,fcntl,(int,fd,int,cmd,USER void *,arg));
LOCAL SYSCALL3(syscall_slong_t,ioctl,(int,fd,int,cmd,USER void *,arg));
LOCAL SYSCALL3(errno_t,execve,(char const *,filename,char const *const *,argv,char const *const *,envp));
#undef sched_yield
LOCAL SYSCALL0(int,sched_yield,(void));
#define sched_yield libc_sched_yield
LOCAL SYSCALL0(pid_t,fork,(void));
LOCAL SYSCALL0(ssize_t,sync,(void));
LOCAL SYSCALL1(errno_t,fsync,(int,fd));
LOCAL SYSCALL1(errno_t,fdatasync,(int,fd));
LOCAL SYSCALL1(ssize_t,syncfs,(int,fs_fd));
LOCAL SYSCALL2(errno_t,fstat64,(int,fd,USER struct stat64 *,statbuf));
LOCAL SYSCALL4(errno_t,fstatat64,(int,dfd,USER char const *,filename,USER struct stat64 *,statbuf,int,flag));
LOCAL SYSCALL4(ssize_t,readlinkat,(int,fd,USER char const *,path,USER char *,buf,size_t,len));
LOCAL SYSCALL4(errno_t,mknodat,(int,dfd,USER char const *,filename,mode_t,mode,dev_t,dev));
LOCAL SYSCALL3(errno_t,mkdirat,(int,dfd,USER char const *,pathname,mode_t,mode));
LOCAL SYSCALL3(errno_t,unlinkat,(int,dfd,USER char const *,pathname,int,flag));
LOCAL SYSCALL3(errno_t,symlinkat,(USER char const *,oldname,int,newdfd,USER char const *,newname));
LOCAL SYSCALL5(errno_t,linkat,(int,olddfd,USER char const *,oldname,int,newdfd,USER char const *,newname,int,flags));
LOCAL SYSCALL4(errno_t,renameat,(int,olddfd,USER char const *,oldname,int,newdfd,USER char const *,newname));
LOCAL SYSCALL4(errno_t,utimensat,(int,dfd,USER char const *,filename,USER struct kernel_timespec const *,utimes,int,flags));
LOCAL SYSCALL4(ssize_t,xreaddir,(int,fd,USER struct dirent *,buf,size_t,bufsize,int,mode));
LOCAL SYSCALL3(s64,xopenpty,(USER char *,name,USER struct termios const *,termp,USER struct winsize const *,winp));
LOCAL SYSCALL0(pid_t,getpid,(void));
LOCAL SYSCALL0(pid_t,getppid,(void));
LOCAL SYSCALL0(pid_t,gettid,(void));
LOCAL SYSCALL1(pid_t,getpgid,(pid_t,pid));
LOCAL SYSCALL2(int,setpgid,(pid_t,pid,pid_t,pgid));
LOCAL SYSCALL1(void,sigreturn,(struct ucontext const *,scp));
LOCAL SYSCALL4(errno_t,sigaction,(int,sig,USER struct sigaction const *,act,USER struct sigaction *,oact,size_t,sigsetsize));
LOCAL SYSCALL4(errno_t,sigprocmask,(int,how,USER __sigset_t const *,set,USER __sigset_t *,oldset,size_t,sigsetsize));
LOCAL SYSCALL4(errno_t,sigtimedwait,(USER __sigset_t const *,uthese,USER siginfo_t *,uinfo,USER struct kernel_timespec const *,uts,size_t,sigsetsize));
LOCAL SYSCALL2(errno_t,sigpending,(USER __sigset_t *,uset,size_t,sigsetsize));
LOCAL SYSCALL2(errno_t,sigsuspend,(USER __sigset_t const *,unewset,size_t,sigsetsize));
LOCAL SYSCALL2(errno_t,kill,(pid_t,pid,int,sig));
LOCAL SYSCALL2(errno_t,tkill,(pid_t,pid,int,sig));
LOCAL SYSCALL3(errno_t,tgkill,(pid_t,tgid,pid_t,pid,int,sig));
LOCAL SYSCALL4(ssize_t,xfdname,(int,fd,int,type,USER char *,buf,size_t,bufsize));
LOCAL SYSCALL2(ssize_t,getcwd,(USER char *,buf,size_t,bufsize));
LOCAL SYSCALL5(int,waitid,(int,which,pid_t,upid,USER siginfo_t *,infop,int,options,USER struct rusage *,ru));
LOCAL SYSCALL4(pid_t,wait4,(pid_t,upid,USER int *,stat_addr,int,options,USER struct rusage *,ru));
LOCAL SYSCALL6(ssize_t,pselect6,(size_t,n,USER fd_set *,inp,USER fd_set *,outp,USER fd_set *,exp,USER struct kernel_timespec const *,tsp,USER void *,sig));
LOCAL SYSCALL5(ssize_t,ppoll,(USER struct pollfd *,ufds,size_t,nfds,USER struct kernel_timespec const *,tsp,USER sigset_t const *,sigmask,size_t,sigsetsize));
LOCAL SYSCALL2(errno_t,pipe2,(USER int *,pfd,int,flags));
LOCAL SYSCALL1(s64,xpipe,(int,flags));
LOCAL SYSCALL5(errno_t,mount,(USER char const *,dev_name,USER char const *,dir_name,USER char const *,type,unsigned long,flags,USER void const *,data));
LOCAL SYSCALL2(errno_t,umount2,(USER char const *,name,int,flags));
LOCAL SYSCALL2(errno_t,gettimeofday,(USER struct kernel_timeval *,tv,USER struct timezone *,tz));
LOCAL SYSCALL2(errno_t,settimeofday,(USER struct kernel_timeval const *,tv,USER struct timezone const *,tz));
LOCAL SYSCALL2(errno_t,nanosleep,(USER struct kernel_timespec const *,rqtp,USER struct kernel_timespec *,rmtp));
LOCAL SYSCALL1(mode_t,umask,(mode_t,mask));
LOCAL SYSCALL3(errno_t,mprotect,(USER void *,start,size_t,len,u32,prot));
LOCAL SYSCALL2(errno_t,swapon,(char const *,specialfile,int,flags));
LOCAL SYSCALL1(errno_t,swapoff,(char const *,specialfile));
LOCAL SYSCALL6(syscall_slong_t,futex,(USER u32 *,uaddr,int,op,u32,val,USER struct timespec *,utime,USER u32 *,uaddr2,u32,val3));
LOCAL SYSCALL5(pid_t,clone,(syscall_ulong_t,flags,USER void *,newsp,USER pid_t *,parent_tidptr,USER pid_t *,child_tidptr,USER void *,tls_val));

/* KOS system-call extensions. */
LOCAL SYSCALL3(ssize_t,xsyslog,(int,type,char const *,p,size_t,len));
LOCAL SYSCALL2(void *,xmmap,(int,version,struct mmap_info const *,data));
LOCAL SYSCALL4(ssize_t,xmunmap,(void *,addr,size_t,len,int,flags,void *,tag));
LOCAL SYSCALL1(void *,xsharesym,(USER char const *,name));
LOCAL SYSCALL1(errno_t,xpaused,(USER char const *,message));
LOCAL SYSCALL3(errno_t,xfexecve,(int,fd,char const *const *,argv,char const *const *,envp));
LOCAL SYSCALL3(errno_t,xfchdirat,(int,dfd,USER char const *,path,int,flags));
LOCAL SYSCALL5(errno_t,xrenameat,(int,olddfd,USER char const *,oldname,int,newdfd,USER char const *,newname,int,flags));
LOCAL SYSCALL2(void *,xdlopen,(char const *,filename,int,flags));
LOCAL SYSCALL2(void *,xfdlopen,(int,fd,int,flags));
LOCAL SYSCALL2(void *,xdlsym,(void *,handle,char const *,symbol));
LOCAL SYSCALL1(int,xdlclose,(void *,handle));
LOCAL SYSCALL4(errno_t,xsymlinkat,(USER char const *,oldname,int,newdfd,USER char const *,newname,int,flags));

#ifdef __ARCH_EXTENDED_FS_SYSCALLS
#undef SYSCALL_NAME
#define SYSCALL_NAME(name) __sys_##name
LOCAL SYSCALL4(s64,lseek,(int,fd,syscall_slong_t,off_hi,syscall_slong_t,off_lo,int,whence));
LOCAL SYSCALL5(ssize_t,pread64,(int,fd,USER void *,buf,size_t,bufsize,syscall_ulong_t,pos_hi,syscall_ulong_t,pos_lo));
LOCAL SYSCALL5(ssize_t,pwrite64,(int,fd,USER void const *,buf,size_t,bufsize,syscall_ulong_t,pos_hi,syscall_ulong_t,pos_lo));
LOCAL SYSCALL3(errno_t,truncate,(USER char const *,path,syscall_ulong_t,len_hi,syscall_ulong_t,len_lo));
LOCAL SYSCALL3(errno_t,ftruncate,(int,fd,syscall_ulong_t,len_hi,syscall_ulong_t,len_lo));
LOCAL SYSCALL6(errno_t,fallocate,(int,fd,int,mode,syscall_ulong_t,off_hi,syscall_ulong_t,off_lo,syscall_ulong_t,len_hi,syscall_ulong_t,len_lo));
#define sys_lseek32(fd,offset,whence)          __sys_lseek(fd,0,(syscall_slong_t)(offset),whence)
#define sys_pread32(fd,buf,bufsize,pos)        __sys_pread64(fd,buf,bufsize,0,(syscall_ulong_t)(pos))
#define sys_pwrite32(fd,buf,bufsize,pos)       __sys_pwrite64(fd,buf,bufsize,0,(syscall_ulong_t)(pos))
#define sys_truncate32(path,length)            __sys_truncate(path,0,(syscall_ulong_t)(length))
#define sys_ftruncate32(fd,length)             __sys_ftruncate(fd,0,(syscall_ulong_t)(length))
#define sys_fallocate32(fd,mode,offset,length) __sys_fallocate(fd,mode,0,(syscall_ulong_t)(offset)),0,(syscall_ulong_t)(length))
#define sys_lseek(fd,offset,whence)            __sys_lseek(fd,(syscall_slong_t)((offset) >> 32),(syscall_slong_t)(offset),whence)
#define sys_pread64(fd,buf,bufsize,pos)        __sys_pread64(fd,buf,bufsize,(syscall_ulong_t)((pos) >> 32),(syscall_ulong_t)(pos))
#define sys_pwrite64(fd,buf,bufsize,pos)       __sys_pwrite64(fd,buf,bufsize,(syscall_ulong_t)((pos) >> 32),(syscall_ulong_t)(pos))
#define sys_truncate(path,length)              __sys_truncate(path,(syscall_ulong_t)((length) >> 32),(syscall_ulong_t)(length))
#define sys_ftruncate(fd,length)               __sys_ftruncate(fd,(syscall_ulong_t)((length) >> 32),(syscall_ulong_t)(length))
#define sys_fallocate(fd,mode,offset,length)   __sys_fallocate(fd,mode,(syscall_ulong_t)((offset) >> 32),(syscall_ulong_t)(offset)),(syscall_ulong_t)((length) >> 32),(syscall_ulong_t)(length))
#else
LOCAL SYSCALL3(s64,lseek,(int,fd,loff_t,offset,int,whence));
LOCAL SYSCALL4(ssize_t,pread64,(int,fd,USER void *,buf,size_t,bufsize,lpos_t,pos));
LOCAL SYSCALL4(ssize_t,pwrite64,(int,fd,USER void const *,buf,size_t,bufsize,lpos_t,pos));
LOCAL SYSCALL2(errno_t,truncate,(USER char const *,path,lpos_t,length));
LOCAL SYSCALL2(errno_t,ftruncate,(int,fd,lpos_t,length));
LOCAL SYSCALL4(errno_t,fallocate,(int,fd,int,mode,lpos_t,offset,lpos_t,length));
#define sys_lseek32(fd,offset,whence)          sys_lseek(fd,(syscall_slong_t)(offset),whence)
#define sys_pread32(fd,buf,bufsize,pos)        sys_pread64(fd,buf,bufsize,(syscall_ulong_t)(pos))
#define sys_pwrite32(fd,buf,bufsize,pos)       sys_pwrite64(fd,buf,bufsize,(syscall_ulong_t)(pos))
#define sys_truncate32(path,length)            sys_truncate(path,(syscall_ulong_t)(length))
#define sys_ftruncate32(fd,length)             sys_ftruncate(fd,(syscall_ulong_t)(length))
#define sys_fallocate32(fd,mode,offset,length) sys_fallocate(fd,mode,(syscall_ulong_t)(offset)),(syscall_ulong_t)(length))
#endif
#undef SYSCALL_NAME

DECL_END

#endif /* !GUARD_LIBS_LIBC_SYSTEM_H */
