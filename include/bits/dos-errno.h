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
#ifndef _BITS_DOS_ERRNO_H
#define _BITS_DOS_ERRNO_H 1

#include <__stdinc.h>
#include <features.h>

__DECL_BEGIN

/* DOS errno values */
#define __DOS_EPERM           1
#define __DOS_ENOENT          2
#define __DOS_ESRCH           3
#define __DOS_EINTR           4
#define __DOS_EIO             5
#define __DOS_ENXIO           6
#define __DOS_E2BIG           7
#define __DOS_ENOEXEC         8
#define __DOS_EBADF           9
#define __DOS_ECHILD          10
#define __DOS_EAGAIN          11
#define __DOS_ENOMEM          12
#define __DOS_EACCES          13
#define __DOS_EFAULT          14
#define __DOS_EBUSY           16
#define __DOS_EEXIST          17
#define __DOS_EXDEV           18
#define __DOS_ENODEV          19
#define __DOS_ENOTDIR         20
#define __DOS_EISDIR          21
#define __DOS_ENFILE          23
#define __DOS_EMFILE          24
#define __DOS_ENOTTY          25
#define __DOS_EFBIG           27
#define __DOS_ENOSPC          28
#define __DOS_ESPIPE          29
#define __DOS_EROFS           30
#define __DOS_EMLINK          31
#define __DOS_EPIPE           32
#define __DOS_EDOM            33
#define __DOS_EDEADLK         36
#define __DOS_ENAMETOOLONG    38
#define __DOS_ENOLCK          39
#define __DOS_ENOSYS          40
#define __DOS_ENOTEMPTY       41
#define __DOS_EINVAL          22
#define __DOS_ERANGE          34
#define __DOS_EILSEQ          42
#define __DOS_STRUNCATE       80
#define __DOS_EDEADLOCK       __DOS_EDEADLK
#define __DOS_EADDRINUSE      100
#define __DOS_EADDRNOTAVAIL   101
#define __DOS_EAFNOSUPPORT    102
#define __DOS_EALREADY        103
#define __DOS_EBADMSG         104
#define __DOS_ECANCELED       105
#define __DOS_ECONNABORTED    106
#define __DOS_ECONNREFUSED    107
#define __DOS_ECONNRESET      108
#define __DOS_EDESTADDRREQ    109
#define __DOS_EHOSTUNREACH    110
#define __DOS_EIDRM           111
#define __DOS_EINPROGRESS     112
#define __DOS_EISCONN         113
#define __DOS_ELOOP           114
#define __DOS_EMSGSIZE        115
#define __DOS_ENETDOWN        116
#define __DOS_ENETRESET       117
#define __DOS_ENETUNREACH     118
#define __DOS_ENOBUFS         119
#define __DOS_ENODATA         120
#define __DOS_ENOLINK         121
#define __DOS_ENOMSG          122
#define __DOS_ENOPROTOOPT     123
#define __DOS_ENOSR           124
#define __DOS_ENOSTR          125
#define __DOS_ENOTCONN        126
#define __DOS_ENOTRECOVERABLE 127
#define __DOS_ENOTSOCK        128
#define __DOS_ENOTSUP         129
#define __DOS_EOPNOTSUPP      130
#define __DOS_EOTHER          131
#define __DOS_EOVERFLOW       132
#define __DOS_EOWNERDEAD      133
#define __DOS_EPROTO          134
#define __DOS_EPROTONOSUPPORT 135
#define __DOS_EPROTOTYPE      136
#define __DOS_ETIME           137
#define __DOS_ETIMEDOUT       138
#define __DOS_ETXTBSY         139
#define __DOS_EWOULDBLOCK     140
#define __DOS_EMAX            140

#ifdef __USE_DOS
#define EPERM           __DOS_EPERM          
#define ENOENT          __DOS_ENOENT         
#define ESRCH           __DOS_ESRCH          
#define EINTR           __DOS_EINTR          
#define EIO             __DOS_EIO            
#define ENXIO           __DOS_ENXIO          
#define E2BIG           __DOS_E2BIG          
#define ENOEXEC         __DOS_ENOEXEC        
#define EBADF           __DOS_EBADF          
#define ECHILD          __DOS_ECHILD         
#define EAGAIN          __DOS_EAGAIN         
#define ENOMEM          __DOS_ENOMEM         
#define EACCES          __DOS_EACCES         
#define EFAULT          __DOS_EFAULT         
#define EBUSY           __DOS_EBUSY          
#define EEXIST          __DOS_EEXIST         
#define EXDEV           __DOS_EXDEV          
#define ENODEV          __DOS_ENODEV         
#define ENOTDIR         __DOS_ENOTDIR        
#define EISDIR          __DOS_EISDIR         
#define ENFILE          __DOS_ENFILE         
#define EMFILE          __DOS_EMFILE         
#define ENOTTY          __DOS_ENOTTY         
#define EFBIG           __DOS_EFBIG          
#define ENOSPC          __DOS_ENOSPC         
#define ESPIPE          __DOS_ESPIPE         
#define EROFS           __DOS_EROFS          
#define EMLINK          __DOS_EMLINK         
#define EPIPE           __DOS_EPIPE          
#define EDOM            __DOS_EDOM           
#define EDEADLK         __DOS_EDEADLK        
#define ENAMETOOLONG    __DOS_ENAMETOOLONG   
#define ENOLCK          __DOS_ENOLCK         
#define ENOSYS          __DOS_ENOSYS         
#define ENOTEMPTY       __DOS_ENOTEMPTY      
#define EINVAL          __DOS_EINVAL         
#define ERANGE          __DOS_ERANGE         
#define EILSEQ          __DOS_EILSEQ         
#define STRUNCATE       __DOS_STRUNCATE      
#define EDEADLOCK       __DOS_EDEADLOCK      
#define EADDRINUSE      __DOS_EADDRINUSE     
#define EADDRNOTAVAIL   __DOS_EADDRNOTAVAIL  
#define EAFNOSUPPORT    __DOS_EAFNOSUPPORT   
#define EALREADY        __DOS_EALREADY       
#define EBADMSG         __DOS_EBADMSG        
#define ECANCELED       __DOS_ECANCELED      
#define ECONNABORTED    __DOS_ECONNABORTED   
#define ECONNREFUSED    __DOS_ECONNREFUSED   
#define ECONNRESET      __DOS_ECONNRESET     
#define EDESTADDRREQ    __DOS_EDESTADDRREQ   
#define EHOSTUNREACH    __DOS_EHOSTUNREACH   
#define EIDRM           __DOS_EIDRM          
#define EINPROGRESS     __DOS_EINPROGRESS    
#define EISCONN         __DOS_EISCONN        
#define ELOOP           __DOS_ELOOP          
#define EMSGSIZE        __DOS_EMSGSIZE       
#define ENETDOWN        __DOS_ENETDOWN       
#define ENETRESET       __DOS_ENETRESET      
#define ENETUNREACH     __DOS_ENETUNREACH    
#define ENOBUFS         __DOS_ENOBUFS        
#define ENODATA         __DOS_ENODATA        
#define ENOLINK         __DOS_ENOLINK        
#define ENOMSG          __DOS_ENOMSG         
#define ENOPROTOOPT     __DOS_ENOPROTOOPT    
#define ENOSR           __DOS_ENOSR          
#define ENOSTR          __DOS_ENOSTR         
#define ENOTCONN        __DOS_ENOTCONN       
#define ENOTRECOVERABLE __DOS_ENOTRECOVERABLE
#define ENOTSOCK        __DOS_ENOTSOCK       
#define ENOTSUP         __DOS_ENOTSUP        
#define EOPNOTSUPP      __DOS_EOPNOTSUPP     
#define EOTHER          __DOS_EOTHER         
#define EOVERFLOW       __DOS_EOVERFLOW      
#define EOWNERDEAD      __DOS_EOWNERDEAD     
#define EPROTO          __DOS_EPROTO         
#define EPROTONOSUPPORT __DOS_EPROTONOSUPPORT
#define EPROTOTYPE      __DOS_EPROTOTYPE     
#define ETIME           __DOS_ETIME          
#define ETIMEDOUT       __DOS_ETIMEDOUT      
#define ETXTBSY         __DOS_ETXTBSY        
#define EWOULDBLOCK     __DOS_EWOULDBLOCK    
#endif

__DECL_END

#endif /* !_BITS_DOS_ERRNO_H */
