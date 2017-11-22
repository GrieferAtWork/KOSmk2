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
#ifndef _SYS_GENERIC_REENT_H
#define _SYS_GENERIC_REENT_H 1
#define _SYS_REENT_H 1
#define _SYS_REENT_H_ 1

/* DISCLAIMER: This file is based off of cygwin's `/usr/include/sys/reent.h' */

#include <_ansi.h>
#include <__stdinc.h>
#include <features.h>
#include <hybrid/typecore.h>
#include <sys/config.h>
#include <sys/_types.h>
#include <bits/mbstate.h>

__SYSDECL_BEGIN

#ifndef __USE_CYG
#define __DEFINE_CYG_MEMBER(T,name)            T __cyg##name
#define __DEFINE_CYG_MEMBER_FUN(ret,name,args) ret (__LIBCCALL *__cyg##name) args
#else /* !__USE_CYG */
#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("_Bigint")
#pragma push_macro("_on_exit_args")
#pragma push_macro("_atexit")
#pragma push_macro("_glue")
#pragma push_macro("_rand48")
#pragma push_macro("_reent")
#pragma push_macro("_next")
#pragma push_macro("_k")
#pragma push_macro("_maxwds")
#pragma push_macro("_sign")
#pragma push_macro("_wds")
#pragma push_macro("_x")
#pragma push_macro("_fnargs")
#pragma push_macro("_dso_handle")
#pragma push_macro("_fntypes")
#pragma push_macro("_is_cxa")
#pragma push_macro("_ind")
#pragma push_macro("_fns")
#pragma push_macro("_on_exit_args_ptr")
#pragma push_macro("_base")
#pragma push_macro("_size")
#pragma push_macro("_p")
#pragma push_macro("_r")
#pragma push_macro("_w")
#pragma push_macro("_flags")
#pragma push_macro("_file")
#pragma push_macro("_bf")
#pragma push_macro("_lbfsize")
#pragma push_macro("_cookie")
#pragma push_macro("_read")
#pragma push_macro("_write")
#pragma push_macro("_seek")
#pragma push_macro("_close")
#pragma push_macro("_ub")
#pragma push_macro("_up")
#pragma push_macro("_ur")
#pragma push_macro("_ubuf")
#pragma push_macro("_nbuf")
#pragma push_macro("_lb")
#pragma push_macro("_blksize")
#pragma push_macro("_offset")
#pragma push_macro("_mbstate")
#pragma push_macro("_flags2")
#pragma push_macro("_errno")
#pragma push_macro("_stdin")
#pragma push_macro("_stdout")
#pragma push_macro("_stderr")
#pragma push_macro("_inc")
#pragma push_macro("_emergency")
#pragma push_macro("_unspecified_locale_info")
#pragma push_macro("_locale")
#pragma push_macro("_result")
#pragma push_macro("_result_k")
#pragma push_macro("_p5s")
#pragma push_macro("_freelist")
#pragma push_macro("_cvtlen")
#pragma push_macro("_cvtbuf")
#pragma push_macro("_unused_rand")
#pragma push_macro("_strtok_last")
#pragma push_macro("_asctime_buf")
#pragma push_macro("_localtime_buf")
#pragma push_macro("_gamma_signgam")
#pragma push_macro("_rand_next")
#pragma push_macro("_r48")
#pragma push_macro("_mblen_state")
#pragma push_macro("_mbtowc_state")
#pragma push_macro("_wctomb_state")
#pragma push_macro("_l64a_buf")
#pragma push_macro("_signal_buf")
#pragma push_macro("_getdate_err")
#pragma push_macro("_mbrlen_state")
#pragma push_macro("_mbrtowc_state")
#pragma push_macro("_mbsrtowcs_state")
#pragma push_macro("_wcrtomb_state")
#pragma push_macro("_wcsrtombs_state")
#pragma push_macro("_h_errno")
#pragma push_macro("_nextf")
#pragma push_macro("_nmalloc")
#pragma push_macro("_unused")
#pragma push_macro("_new")
#pragma push_macro("_sig_func")
#ifdef _REENT_SMALL
#pragma push_macro("_mp")
#pragma push_macro("_data")
#pragma push_macro("_misc")
#endif /* _REENT_SMALL */
#ifndef __SINGLE_THREAD__
#pragma push_macro("_lock")
#endif /* __SINGLE_THREAD__ */
#ifndef _REENT_GLOBAL_ATEXIT
#pragma push_macro("_atexit0")
#endif /* _REENT_GLOBAL_ATEXIT */
#ifdef _REENT_SMALL
#pragma push_macro("_mprec")
#pragma push_macro("_misc_reent")
#endif
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
#endif
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#undef _Bigint
#undef _on_exit_args
#undef _atexit
#undef _glue
#undef _rand48
#undef _reent
#undef _next
#undef _k
#undef _maxwds
#undef _sign
#undef _wds
#undef _x
#undef _fnargs
#undef _dso_handle
#undef _fntypes
#undef _is_cxa
#undef _ind
#undef _fns
#undef _on_exit_args_ptr
#undef _base
#undef _size
#undef _p
#undef _r
#undef _w
#undef _flags
#undef _file
#undef _bf
#undef _lbfsize
#undef _cookie
#undef _read
#undef _write
#undef _seek
#undef _close
#undef _ub
#undef _up
#undef _ur
#undef _ubuf
#undef _nbuf
#undef _lb
#undef _blksize
#undef _offset
#undef _mbstate
#undef _flags2
#undef _errno
#undef _stdin
#undef _stdout
#undef _stderr
#undef _inc
#undef _emergency
#undef _unspecified_locale_info
#undef _locale
#undef _result
#undef _result_k
#undef _p5s
#undef _freelist
#undef _cvtlen
#undef _cvtbuf
#undef _unused_rand
#undef _strtok_last
#undef _asctime_buf
#undef _localtime_buf
#undef _gamma_signgam
#undef _rand_next
#undef _r48
#undef _mblen_state
#undef _mbtowc_state
#undef _wctomb_state
#undef _l64a_buf
#undef _signal_buf
#undef _getdate_err
#undef _mbrlen_state
#undef _mbrtowc_state
#undef _mbsrtowcs_state
#undef _wcrtomb_state
#undef _wcsrtombs_state
#undef _h_errno
#undef _nextf
#undef _nmalloc
#undef _unused
#undef _new
#undef _sig_func
#ifdef _REENT_SMALL
#undef _mp
#undef _data
#undef _misc
#endif /* _REENT_SMALL */
#ifndef __SINGLE_THREAD__
#undef _lock
#endif /* __SINGLE_THREAD__ */
#ifndef _REENT_GLOBAL_ATEXIT
#undef _atexit0
#endif /* _REENT_GLOBAL_ATEXIT */

#define __cyg_Bigint                  _Bigint
#define __cyg_on_exit_args            _on_exit_args
#define __cyg_atexit                  _atexit
#define __cyg_glue                    _glue
#define __cyg_rand48                  _rand48
#define __cyg_reent                   _reent
#ifdef _REENT_SMALL
#undef _mprec
#undef _misc_reent
#define __cyg_mprec                   _mprec
#define __cyg_misc_reent              _misc_reent
#endif
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
#define __DEFINE_CYG_MEMBER(T,name)            union{ T __cyg##name; T name; }
#define __DEFINE_CYG_MEMBER_FUN(ret,name,args) union{ ret (__LIBCCALL *__cyg##name)args; ret (__LIBCCALL *name)args; }
#else /* __COMPILER_HAVE_TRANSPARENT_UNION */
#define __DEFINE_CYG_MEMBER(T,name)            T name;
#define __DEFINE_CYG_MEMBER_FUN(ret,name,args) ret (__LIBCCALL *name) args
#define __cyg_next                    _next
#define __cyg_k                       _k
#define __cyg_maxwds                  _maxwds
#define __cyg_sign                    _sign
#define __cyg_wds                     _wds
#define __cyg_x                       _x
#define __cyg_fnargs                  _fnargs
#define __cyg_dso_handle              _dso_handle
#define __cyg_fntypes                 _fntypes
#define __cyg_is_cxa                  _is_cxa
#define __cyg_ind                     _ind
#define __cyg_fns                     _fns
#define __cyg_on_exit_args_ptr        _on_exit_args_ptr
#define __cyg_base                    _base
#define __cyg_size                    _size
#define __cyg_p                       _p
#define __cyg_r                       _r
#define __cyg_w                       _w
#define __cyg_flags                   _flags
#define __cyg_file                    _file
#define __cyg_bf                      _bf
#define __cyg_lbfsize                 _lbfsize
#define __cyg_cookie                  _cookie
#define __cyg_read                    _read
#define __cyg_write                   _write
#define __cyg_seek                    _seek
#define __cyg_close                   _close
#define __cyg_ub                      _ub
#define __cyg_up                      _up
#define __cyg_ur                      _ur
#define __cyg_ubuf                    _ubuf
#define __cyg_nbuf                    _nbuf
#define __cyg_lb                      _lb
#define __cyg_blksize                 _blksize
#define __cyg_offset                  _offset
#define __cyg_mbstate                 _mbstate
#define __cyg_flags2                  _flags2
#define __cyg_errno                   _errno
#define __cyg_stdin                   _stdin
#define __cyg_stdout                  _stdout
#define __cyg_stderr                  _stderr
#define __cyg_inc                     _inc
#define __cyg_emergency               _emergency
#define __cyg_unspecified_locale_info _unspecified_locale_info
#define __cyg_locale                  _locale
#define __cyg_result                  _result
#define __cyg_result_k                _result_k
#define __cyg_p5s                     _p5s
#define __cyg_freelist                _freelist
#define __cyg_cvtlen                  _cvtlen
#define __cyg_cvtbuf                  _cvtbuf
#define __cyg_unused_rand             _unused_rand
#define __cyg_strtok_last             _strtok_last
#define __cyg_asctime_buf             _asctime_buf
#define __cyg_localtime_buf           _localtime_buf
#define __cyg_gamma_signgam           _gamma_signgam
#define __cyg_rand_next               _rand_next
#define __cyg_r48                     _r48
#define __cyg_mblen_state             _mblen_state
#define __cyg_mbtowc_state            _mbtowc_state
#define __cyg_wctomb_state            _wctomb_state
#define __cyg_l64a_buf                _l64a_buf
#define __cyg_signal_buf              _signal_buf
#define __cyg_getdate_err             _getdate_err
#define __cyg_mbrlen_state            _mbrlen_state
#define __cyg_mbrtowc_state           _mbrtowc_state
#define __cyg_mbsrtowcs_state         _mbsrtowcs_state
#define __cyg_wcrtomb_state           _wcrtomb_state
#define __cyg_wcsrtombs_state         _wcsrtombs_state
#define __cyg_h_errno                 _h_errno
#define __cyg_nextf                   _nextf
#define __cyg_nmalloc                 _nmalloc
#define __cyg_unused                  _unused
#define __cyg_new                     _new
#define __cyg_sig_func                _sig_func
#ifdef _REENT_SMALL
#define __cyg_mp                      _mp
#define __cyg_data                    _data
#define __cyg_misc                    _misc
#endif /* _REENT_SMALL */
#ifndef __SINGLE_THREAD__
#define __cyg_lock                    _lock
#endif /* __SINGLE_THREAD__ */
#ifndef _REENT_GLOBAL_ATEXIT
#define __cyg_atexit0                 _atexit0
#endif /* _REENT_GLOBAL_ATEXIT */
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
#endif /* __USE_CYG */




#ifndef _NULL
#define _NULL __NULLPTR
#endif

#ifndef __Long
#if __SIZEOF_LONG__ == 4
#define __Long          long
typedef unsigned __Long __ULong;
#elif __SIZEOF_INT__ == 4
#define __Long          int
typedef unsigned __Long __ULong;
#else
#define __Long          __INT32_TYPE__
#define __ULong         __UINT32_TYPE__
#endif
#endif

struct __cyg_reent;
struct __cyg_locale_t;
struct __cyg_Bigint {
 __DEFINE_CYG_MEMBER(struct __cyg_Bigint *,_next);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_k);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_maxwds);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_sign);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_wds);
 __DEFINE_CYG_MEMBER(__ULong,_x[1]);
};
struct __tm {
 __INT32_TYPE__ __tm_sec,__tm_min,__tm_hour,__tm_mday;
 __INT32_TYPE__ __tm_mon,__tm_year,__tm_wday,__tm_yday;
 __INT32_TYPE__ __tm_isdst;
};
#define _ATEXIT_SIZE 32
struct __cyg_on_exit_args {
 __DEFINE_CYG_MEMBER(void *,_fnargs[_ATEXIT_SIZE]);
 __DEFINE_CYG_MEMBER(void *,_dso_handle[_ATEXIT_SIZE]);
 __DEFINE_CYG_MEMBER(__ULong,_fntypes);
 __DEFINE_CYG_MEMBER(__ULong,_is_cxa);
};
struct __cyg_atexit {
 __DEFINE_CYG_MEMBER(struct __cyg_atexit *,_next);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_ind);
 __DEFINE_CYG_MEMBER_FUN(void,_fns[_ATEXIT_SIZE],(void));
#ifdef _REENT_SMALL
 __DEFINE_CYG_MEMBER(struct __cyg_on_exit_args *,_on_exit_args_ptr);
#else /* _REENT_SMALL */
 struct __cyg_on_exit_args  __cyg_on_exit_args;
#endif /* !_REENT_SMALL */
};

#ifndef __USE_CYG
/* The declarations of these aren't actually used,
 * so try not to expose them because they might change. */
struct __sFILE;
struct __sFILE64;
#else

#ifdef _REENT_SMALL
#   define _ATEXIT_INIT  {__NULLPTR,0,{__NULLPTR},__NULLPTR}
#else
#   define _ATEXIT_INIT  {__NULLPTR,0,{__NULLPTR},{{__NULLPTR},{__NULLPTR},0,0}}
#endif
#ifdef _REENT_GLOBAL_ATEXIT
#   define _REENT_INIT_ATEXIT
#else
#   define _REENT_INIT_ATEXIT   __NULLPTR,_ATEXIT_INIT,
#endif

struct __sbuf {
 __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_base);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_size);
};
#if !defined(__USE_FILE_OFFSET64)
__NAMESPACE_STD_BEGIN
struct __IO_FILE
#else
struct __sFILE
#endif
{
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_p);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_r);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_w);
  __DEFINE_CYG_MEMBER(__INT16_TYPE__,_flags);
  __DEFINE_CYG_MEMBER(__INT16_TYPE__,_file);
  __DEFINE_CYG_MEMBER(struct __sbuf,_bf);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_lbfsize);
#ifdef _REENT_SMALL
  __DEFINE_CYG_MEMBER(struct __cyg_reent *,_data);
#endif
  __DEFINE_CYG_MEMBER(void *,_cookie);
  __DEFINE_CYG_MEMBER_FUN(_READ_WRITE_RETURN_TYPE,_read,(struct __cyg_reent *,_PTR,__CHAR8_TYPE__ *,_READ_WRITE_BUFSIZE_TYPE));
  __DEFINE_CYG_MEMBER_FUN(_READ_WRITE_RETURN_TYPE,_write,(struct __cyg_reent *,_PTR,__CHAR8_TYPE__ const *,_READ_WRITE_BUFSIZE_TYPE));
  __DEFINE_CYG_MEMBER_FUN(__cyg_fpos_t,_seek,(struct __cyg_reent *,_PTR,__cyg_fpos_t,__INT32_TYPE__));
  __DEFINE_CYG_MEMBER_FUN(__INT32_TYPE__,_close,(struct __cyg_reent *,_PTR));
  __DEFINE_CYG_MEMBER(struct __sbuf,_ub);
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_up);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_ur);
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__,_ubuf[3]);
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__,_nbuf[1]);
  __DEFINE_CYG_MEMBER(struct __sbuf,_lb);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_blksize);
  __DEFINE_CYG_MEMBER(__cyg_off_t,_offset);
#ifndef _REENT_SMALL
  __DEFINE_CYG_MEMBER(struct __cyg_reent *,_data);
#endif
#ifndef __SINGLE_THREAD__
  __DEFINE_CYG_MEMBER(__cyg_flock_t,_lock);
#endif
  __DEFINE_CYG_MEMBER(__mbstate_t,_mbstate);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_flags2);
};

#if !defined(__USE_FILE_OFFSET64) && defined(__USE_CYG)
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(__IO_FILE)
#define __sFILE  __IO_FILE
#endif


#ifdef __USE_FILE_OFFSET64
__NAMESPACE_STD_BEGIN
struct __IO_FILE {
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_p);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_r);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_w);
  __DEFINE_CYG_MEMBER(__INT16_TYPE__,_flags);
  __DEFINE_CYG_MEMBER(__INT16_TYPE__,_file);
  __DEFINE_CYG_MEMBER(struct __sbuf,_bf);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_lbfsize);
  __DEFINE_CYG_MEMBER(struct __cyg_reent *,_data);
  __DEFINE_CYG_MEMBER(_PTR,_cookie);
  __DEFINE_CYG_MEMBER_FUN(_READ_WRITE_RETURN_TYPE,_read,(struct __cyg_reent *,_PTR,__CHAR8_TYPE__ *,_READ_WRITE_BUFSIZE_TYPE));
  __DEFINE_CYG_MEMBER_FUN(_READ_WRITE_RETURN_TYPE,_write,(struct __cyg_reent *,_PTR,__CHAR8_TYPE__ const *,_READ_WRITE_BUFSIZE_TYPE));
  __DEFINE_CYG_MEMBER_FUN(__cyg_fpos_t,_seek,(struct __cyg_reent *,_PTR,__cyg_fpos_t,__INT32_TYPE__));
  __DEFINE_CYG_MEMBER_FUN(__INT32_TYPE__,_close,(struct __cyg_reent *,_PTR));
  __DEFINE_CYG_MEMBER(struct __sbuf,_ub);
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_up);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_ur);
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__,_ubuf[3]);
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__,_nbuf[1]);
  __DEFINE_CYG_MEMBER(struct __sbuf,_lb);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_blksize);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_flags2);
  __DEFINE_CYG_MEMBER(__cyg_off64_t,_offset);
  __DEFINE_CYG_MEMBER_FUN(__cyg_fpos64_t,_seek64,(struct __cyg_reent *,_PTR,__cyg_fpos64_t,__INT32_TYPE__));
#ifndef __SINGLE_THREAD__
  __DEFINE_CYG_MEMBER(__cyg_flock_t,_lock);
#endif
  __DEFINE_CYG_MEMBER(__mbstate_t,_mbstate);
};
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(__IO_FILE)
#define __sFILE64  __IO_FILE
#endif /* __USE_FILE_OFFSET64 */
#endif /* __USE_CYG */

struct __cyg_glue {
 __DEFINE_CYG_MEMBER(struct __cyg_glue *,_next);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_niobs);
 __DEFINE_CYG_MEMBER(__FILE *,_iobs);
};

#define __CYG_RAND48_SEED_0        0x330e
#define __CYG_RAND48_SEED_1        0xabcd
#define __CYG_RAND48_SEED_2        0x1234
#define __CYG_RAND48_MULT_0        0xe66d
#define __CYG_RAND48_MULT_1        0xdeec
#define __CYG_RAND48_MULT_2        0x0005
#define __CYG_RAND48_ADD           0x000b
#define __CYG_REENT_EMERGENCY_SIZE 25
#define __CYG_REENT_ASCTIME_SIZE   26
#define __CYG_REENT_SIGNAL_SIZE    24
#ifdef __USE_CYG
#define _RAND48_SEED_0             __CYG_RAND48_SEED_0
#define _RAND48_SEED_1             __CYG_RAND48_SEED_1
#define _RAND48_SEED_2             __CYG_RAND48_SEED_2
#define _RAND48_MULT_0             __CYG_RAND48_MULT_0
#define _RAND48_MULT_1             __CYG_RAND48_MULT_1
#define _RAND48_MULT_2             __CYG_RAND48_MULT_2
#define _RAND48_ADD                __CYG_RAND48_ADD
#define _REENT_EMERGENCY_SIZE      __CYG_REENT_EMERGENCY_SIZE
#define _REENT_ASCTIME_SIZE        __CYG_REENT_ASCTIME_SIZE
#define _REENT_SIGNAL_SIZE         __CYG_REENT_SIGNAL_SIZE
#endif
struct __cyg_rand48 {
  __DEFINE_CYG_MEMBER(__UINT16_TYPE__,_seed[3]);
  __DEFINE_CYG_MEMBER(__UINT16_TYPE__,_mult[3]);
  __DEFINE_CYG_MEMBER(__UINT16_TYPE__,_add);
#ifdef _REENT_SMALL
  __DEFINE_CYG_MEMBER(__UINT64_TYPE__,_rand_next);
#endif
};


#ifdef _REENT_SMALL

struct __cyg_mprec;
struct __cyg_misc_reent;
struct __cyg_reent {
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_errno);
  __DEFINE_CYG_MEMBER(__FILE *,_stdin);
  __DEFINE_CYG_MEMBER(__FILE *,_stdout);
  __DEFINE_CYG_MEMBER(__FILE *,_stderr);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_inc);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_emergency);
  __INT32_TYPE__ __sdidinit;
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_unspecified_locale_info);
  __DEFINE_CYG_MEMBER(struct __cyg_locale_t *,_locale);
  __DEFINE_CYG_MEMBER(struct __cyg_mprec *,_mp);
  void _EXFNPTR(__cleanup,(struct __cyg_reent *));
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_gamma_signgam);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_cvtlen);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_cvtbuf);
  __DEFINE_CYG_MEMBER(struct __cyg_rand48 *,_r48);
  __DEFINE_CYG_MEMBER(struct __tm *,_localtime_buf);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_asctime_buf);
#ifdef __USE_CYG
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
union{
  void (**_sig_func)(__INT32_TYPE__);
  void (**__cyg_sig_func)(__INT32_TYPE__);
};
#else
#define __cyg_sig_func   _sig_func
  void (**_sig_func)(__INT32_TYPE__);
#endif
#else
  void (**__cyg_sig_func)(__INT32_TYPE__);
#endif
#ifndef _REENT_GLOBAL_ATEXIT
  struct __cyg_atexit *__cyg_atexit;
  __DEFINE_CYG_MEMBER(struct __cyg_atexit,_atexit0);
#endif
  struct __cyg_glue __sglue; __FILE *__sf;
  __DEFINE_CYG_MEMBER(struct __cyg_misc_reent *,_misc);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_signal_buf);
};


#ifdef __USE_CYG
struct __cyg_mprec {
 __DEFINE_CYG_MEMBER(struct __cyg_Bigint *,_result);
 __DEFINE_CYG_MEMBER(__INT32_TYPE__,_result_k);
 __DEFINE_CYG_MEMBER(struct __cyg_Bigint *,_p5s);
 __DEFINE_CYG_MEMBER(struct __cyg_Bigint **,_freelist);
};
struct __cyg_misc_reent {
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_strtok_last);
  __DEFINE_CYG_MEMBER(__mbstate_t,_mblen_state);
  __DEFINE_CYG_MEMBER(__mbstate_t,_wctomb_state);
  __DEFINE_CYG_MEMBER(__mbstate_t,_mbtowc_state);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__,_l64a_buf[8]);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_getdate_err);
  __DEFINE_CYG_MEMBER(__mbstate_t,_mbrlen_state);
  __DEFINE_CYG_MEMBER(__mbstate_t,_mbrtowc_state);
  __DEFINE_CYG_MEMBER(__mbstate_t,_mbsrtowcs_state);
  __DEFINE_CYG_MEMBER(__mbstate_t,_wcrtomb_state);
  __DEFINE_CYG_MEMBER(__mbstate_t,_wcsrtombs_state);
};
struct __sFILE_fake {
  __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_p);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_r);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_w);
  __DEFINE_CYG_MEMBER(__INT16_TYPE__,_flags);
  __DEFINE_CYG_MEMBER(__INT16_TYPE__,_file);
  __DEFINE_CYG_MEMBER(struct __sbuf,_bf);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_lbfsize);
  __DEFINE_CYG_MEMBER(struct __cyg_reent *,_data);
};
__LIBC void (__LIBCCALL __sinit)(struct __cyg_reent *);
#define _REENT_SMALL_CHECK_INIT(ptr) do{ if ((ptr) && !(ptr)->__sdidinit) __sinit(ptr); }__WHILE0
__LIBC struct __sFILE_fake const __sf_fake_stdin;
__LIBC struct __sFILE_fake const __sf_fake_stdout;
__LIBC struct __sFILE_fake const __sf_fake_stderr;
#define _REENT_INIT(var) \
  {0,(__FILE *)&__sf_fake_stdin,(__FILE *)&__sf_fake_stdout,(__FILE *)&__sf_fake_stderr, \
   0,__NULLPTR,0,0,__NULLPTR,__NULLPTR,__NULLPTR,0,0,__NULLPTR,__NULLPTR,__NULLPTR,__NULLPTR, \
   __NULLPTR,_REENT_INIT_ATEXIT {__NULLPTR,0,__NULLPTR},__NULLPTR,__NULLPTR,__NULLPTR}
#define _REENT_INIT_PTR_ZEROED(var) \
  ((var)->__cyg_stdin = (__FILE *)&__sf_fake_stdin, \
   (var)->__cyg_stdout = (__FILE *)&__sf_fake_stdout, \
   (var)->__cyg_stderr = (__FILE *)&__sf_fake_stderr)
#ifdef _REENT_CHECK_DEBUG
#include <assert.h>
#define __reent_assert(x) assert(x)
#else
#define __reent_assert(x) ((void)0)
#endif
#define _REENT_CHECK(var,what,type,size,init) do { \
  struct __cyg_reent *_r = (var); \
  if (_r->what == NULL) { \
    _r->what = (type)malloc(size); \
    __reent_assert(_r->what); \
    init; \
  } \
}__WHILE0
#define _REENT_CHECK_TM(var)          _REENT_CHECK(var,__cyg_localtime_buf,struct __tm *,sizeof(*((var)->__cyg_localtime_buf)),/* nothing */)
#define _REENT_CHECK_ASCTIME_BUF(var) _REENT_CHECK(var,__cyg_asctime_buf,__CHAR8_TYPE__ *,__CYG_REENT_ASCTIME_SIZE,memset((var)->__cyg_asctime_buf,0,__CYG_REENT_ASCTIME_SIZE))
#define _REENT_INIT_RAND48(var)       do{ struct __cyg_reent *_r = (var); _r->__cyg_r48->__cyg_seed[0] = __CYG_RAND48_SEED_0; _r->__cyg_r48->__cyg_seed[1] = __CYG_RAND48_SEED_1; _r->__cyg_r48->__cyg_seed[2] = __CYG_RAND48_SEED_2; \
                                          _r->__cyg_r48->__cyg_mult[0] = __CYG_RAND48_MULT_0; _r->__cyg_r48->__cyg_mult[1] = __CYG_RAND48_MULT_1; _r->__cyg_r48->__cyg_mult[2] = __CYG_RAND48_MULT_2; _r->__cyg_r48->__cyg_add = __CYG_RAND48_ADD; _r->__cyg_r48->__cyg_rand_next = 1; }__WHILE0
#define _REENT_CHECK_RAND48(var)      _REENT_CHECK(var, __cyg_r48, struct __cyg_rand48 *, sizeof *((var)->__cyg_r48), _REENT_INIT_RAND48((var)))
#define _REENT_INIT_MP(var)           do{ struct __cyg_reent *_r = (var); _r->__cyg_mp->__cyg_result_k = 0; _r->__cyg_mp->__cyg_result = _r->__cyg_mp->__cyg_p5s = __NULLPTR; _r->__cyg_mp->__cyg_freelist = __NULLPTR; }__WHILE0
#define _REENT_CHECK_MP(var)          _REENT_CHECK(var,_mp,struct __cyg_mprec *,sizeof(*((var)->__cyg_mp)),_REENT_INIT_MP(var))
#define _REENT_CHECK_EMERGENCY(var)   _REENT_CHECK(var,_emergency,__CHAR8_TYPE__ *,__CYG_REENT_EMERGENCY_SIZE,/* nothing */)
#define _REENT_INIT_MISC(var)         do{ struct __cyg_reent *_r = (var); _r->__cyg_misc->__cyg_strtok_last = __NULLPTR; _r->__cyg_misc->__cyg_mblen_state.__count = 0; _r->__cyg_misc->__cyg_mblen_state.__value.__wch = 0; \
                                          _r->__cyg_misc->__cyg_wctomb_state.__count = 0; _r->__cyg_misc->__cyg_wctomb_state.__value.__wch = 0; _r->__cyg_misc->__cyg_mbtowc_state.__count = 0; _r->__cyg_misc->__cyg_mbtowc_state.__value.__wch = 0; \
                                          _r->__cyg_misc->__cyg_mbrlen_state.__count = 0; _r->__cyg_misc->__cyg_mbrlen_state.__value.__wch = 0; _r->__cyg_misc->__cyg_mbrtowc_state.__count = 0; _r->__cyg_misc->__cyg_mbrtowc_state.__value.__wch = 0; \
                                          _r->__cyg_misc->__cyg_mbsrtowcs_state.__count = 0; _r->__cyg_misc->__cyg_mbsrtowcs_state.__value.__wch = 0; _r->__cyg_misc->__cyg_wcrtomb_state.__count = 0; _r->__cyg_misc->__cyg_wcrtomb_state.__value.__wch = 0; \
                                          _r->__cyg_misc->__cyg_wcsrtombs_state.__count = 0; _r->__cyg_misc->__cyg_wcsrtombs_state.__value.__wch = 0; _r->__cyg_misc->__cyg_l64a_buf[0] = '\0'; _r->__cyg_misc->__cyg_getdate_err = 0; }__WHILE0
#define _REENT_CHECK_MISC(var)        _REENT_CHECK(var,__cyg_misc,struct __cyg_misc_reent *,sizeof(*((var)->__cyg_misc)),_REENT_INIT_MISC(var))
#define _REENT_CHECK_SIGNAL_BUF(var)  _REENT_CHECK(var,_signal_buf,__CHAR8_TYPE__ *,__CYG_REENT_SIGNAL_SIZE,/* nothing */)
#define _REENT_SIGNGAM(ptr)         ((ptr)->__cyg_gamma_signgam)
#define _REENT_RAND_NEXT(ptr)       ((ptr)->__cyg_r48->__cyg_rand_next)
#define _REENT_RAND48_SEED(ptr)     ((ptr)->__cyg_r48->__cyg_seed)
#define _REENT_RAND48_MULT(ptr)     ((ptr)->__cyg_r48->__cyg_mult)
#define _REENT_RAND48_ADD(ptr)      ((ptr)->__cyg_r48->__cyg_add)
#define _REENT_MP_RESULT(ptr)       ((ptr)->__cyg_mp->__cyg_result)
#define _REENT_MP_RESULT_K(ptr)     ((ptr)->__cyg_mp->__cyg_result_k)
#define _REENT_MP_P5S(ptr)          ((ptr)->__cyg_mp->__cyg_p5s)
#define _REENT_MP_FREELIST(ptr)     ((ptr)->__cyg_mp->__cyg_freelist)
#define _REENT_ASCTIME_BUF(ptr)     ((ptr)->__cyg_asctime_buf)
#define _REENT_TM(ptr)              ((ptr)->__cyg_localtime_buf)
#define _REENT_EMERGENCY(ptr)       ((ptr)->__cyg_emergency)
#define _REENT_STRTOK_LAST(ptr)     ((ptr)->__cyg_misc->__cyg_strtok_last)
#define _REENT_MBLEN_STATE(ptr)     ((ptr)->__cyg_misc->__cyg_mblen_state)
#define _REENT_MBTOWC_STATE(ptr)    ((ptr)->__cyg_misc->__cyg_mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)    ((ptr)->__cyg_misc->__cyg_wctomb_state)
#define _REENT_MBRLEN_STATE(ptr)    ((ptr)->__cyg_misc->__cyg_mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr)   ((ptr)->__cyg_misc->__cyg_mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr) ((ptr)->__cyg_misc->__cyg_mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr)   ((ptr)->__cyg_misc->__cyg_wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr) ((ptr)->__cyg_misc->__cyg_wcsrtombs_state)
#define _REENT_L64A_BUF(ptr)        ((ptr)->__cyg_misc->__cyg_l64a_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->__cyg_misc->__cyg_getdate_err))
#define _REENT_SIGNAL_BUF(ptr)      ((ptr)->__cyg_signal_buf)
#endif /* __USE_CYG */
#else /* !_REENT_SMALL */

struct __cyg_reent {
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_errno);
  __DEFINE_CYG_MEMBER(__FILE *,_stdin);
  __DEFINE_CYG_MEMBER(__FILE *,_stdout);
  __DEFINE_CYG_MEMBER(__FILE *,_stderr);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_inc);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__,_emergency[__CYG_REENT_EMERGENCY_SIZE]);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_unspecified_locale_info);
  __DEFINE_CYG_MEMBER(struct __cyg_locale_t *,_locale);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_sdidinit);
  __DEFINE_CYG_MEMBER_FUN(void,_cleanup,(struct __cyg_reent *));
  __DEFINE_CYG_MEMBER(struct __cyg_Bigint *,_result);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_result_k);
  __DEFINE_CYG_MEMBER(struct __cyg_Bigint *,_p5s);
  __DEFINE_CYG_MEMBER(struct __cyg_Bigint **,_freelist);
  __DEFINE_CYG_MEMBER(__INT32_TYPE__,_cvtlen);
  __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_cvtbuf);
  union {
      struct {
          __DEFINE_CYG_MEMBER(__UINT32_TYPE__,_unused_rand);
          __DEFINE_CYG_MEMBER(__CHAR8_TYPE__ *,_strtok_last);
          __DEFINE_CYG_MEMBER(__CHAR8_TYPE__,_asctime_buf[__CYG_REENT_ASCTIME_SIZE]);
          __DEFINE_CYG_MEMBER(struct __tm,_localtime_buf);
          __DEFINE_CYG_MEMBER(__INT32_TYPE__,_gamma_signgam);
          __DEFINE_CYG_MEMBER(__UINT64_TYPE__,_rand_next);
          __DEFINE_CYG_MEMBER(struct __cyg_rand48,_r48);
          __DEFINE_CYG_MEMBER(__mbstate_t,_mblen_state);
          __DEFINE_CYG_MEMBER(__mbstate_t,_mbtowc_state);
          __DEFINE_CYG_MEMBER(__mbstate_t,_wctomb_state);
          __DEFINE_CYG_MEMBER(__CHAR8_TYPE__,_l64a_buf[8]);
          __DEFINE_CYG_MEMBER(__CHAR8_TYPE__,_signal_buf[__CYG_REENT_SIGNAL_SIZE]);
          __DEFINE_CYG_MEMBER(__INT32_TYPE__,_getdate_err);
          __DEFINE_CYG_MEMBER(__mbstate_t,_mbrlen_state);
          __DEFINE_CYG_MEMBER(__mbstate_t,_mbrtowc_state);
          __DEFINE_CYG_MEMBER(__mbstate_t,_mbsrtowcs_state);
          __DEFINE_CYG_MEMBER(__mbstate_t,_wcrtomb_state);
          __DEFINE_CYG_MEMBER(__mbstate_t,_wcsrtombs_state);
          __DEFINE_CYG_MEMBER(__INT32_TYPE__,_h_errno);
      } __cyg_reent;
      struct {
#define _N_LISTS 30
          __DEFINE_CYG_MEMBER(__UINT8_TYPE__ *,_nextf[_N_LISTS]);
          __DEFINE_CYG_MEMBER(__UINT32_TYPE__,_nmalloc[_N_LISTS]);
        } _unused;
    } _new;
#ifndef _REENT_GLOBAL_ATEXIT
  struct __cyg_atexit *__cyg_atexit;
  __DEFINE_CYG_MEMBER(struct __cyg_atexit,_atexit0);
#endif
#ifdef __USE_CYG
#ifdef __COMPILER_HAVE_TRANSPARENT_UNION
union{
  void (**_sig_func)(__INT32_TYPE__);
  void (**__cyg_sig_func)(__INT32_TYPE__);
};
#else
#define __cyg_sig_func   _sig_func
  void (**_sig_func)(__INT32_TYPE__);
#endif
#else
  void (**__cyg_sig_func)(__INT32_TYPE__);
#endif
  struct __cyg_glue    __sglue;
#ifdef __USE_CYG
#ifndef _REENT_GLOBAL_STDIO_STREAMS
  __FILE          __sf[3];
#endif
#endif /* !__USE_CYG */
};

#ifdef __USE_CYG
#define _REENT_SMALL_CHECK_INIT(ptr) /* nothing */
#ifdef _REENT_GLOBAL_STDIO_STREAMS
__LIBC __FILE __sf[3];
#define _REENT_STDIO_STREAM(var,index) &__sf[index]
#else
#define _REENT_STDIO_STREAM(var,index) &(var)->__sf[index]
#endif
#define _REENT_INIT(var) \
  {0,_REENT_STDIO_STREAM(&(var),0),_REENT_STDIO_STREAM(&(var),1),_REENT_STDIO_STREAM(&(var),2),0,"",0,__NULLPTR,0,__NULLPTR,__NULLPTR,0,__NULLPTR, \
   __NULLPTR,0,__NULLPTR,{{0,__NULLPTR,"",{0,0,0,0,0,0,0,0,0},0,1,{{__CYG_RAND48_SEED_0,__CYG_RAND48_SEED_1,__CYG_RAND48_SEED_2},{__CYG_RAND48_MULT_0,__CYG_RAND48_MULT_1, \
   __CYG_RAND48_MULT_2},__CYG_RAND48_ADD},{0,{0}},{0,{0}},{0,{0}},"","",0,{0,{0}},{0,{0}},{0,{0}},{0,{0}},{0,{0}}}},_REENT_INIT_ATEXIT __NULLPTR,{__NULLPTR,0,__NULLPTR}}
#define _REENT_INIT_PTR_ZEROED(var) \
  { (var)->__cyg_stdin = _REENT_STDIO_STREAM(var,0); (var)->__cyg_stdout = _REENT_STDIO_STREAM(var,1); (var)->__cyg_stderr = _REENT_STDIO_STREAM(var,2); \
    (var)->__cyg_new.__cyg_reent._rand_next = 1; (var)->__cyg_new.__cyg_reent._r48._seed[0] = __CYG_RAND48_SEED_0; (var)->__cyg_new.__cyg_reent._r48._seed[1] = __CYG_RAND48_SEED_1; \
    (var)->__cyg_new.__cyg_reent._r48._seed[2] = __CYG_RAND48_SEED_2; (var)->__cyg_new.__cyg_reent._r48._mult[0] = __CYG_RAND48_MULT_0; (var)->__cyg_new.__cyg_reent._r48._mult[1] = __CYG_RAND48_MULT_1; \
    (var)->__cyg_new.__cyg_reent._r48._mult[2] = __CYG_RAND48_MULT_2; (var)->__cyg_new.__cyg_reent._r48._add = __CYG_RAND48_ADD; }
#define _REENT_CHECK_RAND48(ptr)      /* nothing */
#define _REENT_CHECK_MP(ptr)          /* nothing */
#define _REENT_CHECK_TM(ptr)          /* nothing */
#define _REENT_CHECK_ASCTIME_BUF(ptr) /* nothing */
#define _REENT_CHECK_EMERGENCY(ptr)   /* nothing */
#define _REENT_CHECK_MISC(ptr)        /* nothing */
#define _REENT_CHECK_SIGNAL_BUF(ptr)  /* nothing */
#define _REENT_SIGNGAM(ptr)         ((ptr)->__cyg_new.__cyg_reent._gamma_signgam)
#define _REENT_RAND_NEXT(ptr)       ((ptr)->__cyg_new.__cyg_reent._rand_next)
#define _REENT_RAND48_SEED(ptr)     ((ptr)->__cyg_new.__cyg_reent._r48._seed)
#define _REENT_RAND48_MULT(ptr)     ((ptr)->__cyg_new.__cyg_reent._r48._mult)
#define _REENT_RAND48_ADD(ptr)      ((ptr)->__cyg_new.__cyg_reent._r48._add)
#define _REENT_MP_RESULT(ptr)       ((ptr)->__cyg_result)
#define _REENT_MP_RESULT_K(ptr)     ((ptr)->__cyg_result_k)
#define _REENT_MP_P5S(ptr)          ((ptr)->__cyg_p5s)
#define _REENT_MP_FREELIST(ptr)     ((ptr)->__cyg_freelist)
#define _REENT_ASCTIME_BUF(ptr)     ((ptr)->__cyg_new.__cyg_reent._asctime_buf)
#define _REENT_TM(ptr)             (&(ptr)->__cyg_new.__cyg_reent._localtime_buf)
#define _REENT_EMERGENCY(ptr)       ((ptr)->__cyg_emergency)
#define _REENT_STRTOK_LAST(ptr)     ((ptr)->__cyg_new.__cyg_reent._strtok_last)
#define _REENT_MBLEN_STATE(ptr)     ((ptr)->__cyg_new.__cyg_reent._mblen_state)
#define _REENT_MBTOWC_STATE(ptr)    ((ptr)->__cyg_new.__cyg_reent._mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)    ((ptr)->__cyg_new.__cyg_reent._wctomb_state)
#define _REENT_MBRLEN_STATE(ptr)    ((ptr)->__cyg_new.__cyg_reent._mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr)   ((ptr)->__cyg_new.__cyg_reent._mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr) ((ptr)->__cyg_new.__cyg_reent._mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr)   ((ptr)->__cyg_new.__cyg_reent._wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr) ((ptr)->__cyg_new.__cyg_reent._wcsrtombs_state)
#define _REENT_L64A_BUF(ptr)        ((ptr)->__cyg_new.__cyg_reent._l64a_buf)
#define _REENT_SIGNAL_BUF(ptr)      ((ptr)->__cyg_new.__cyg_reent._signal_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->__cyg_new.__cyg_reent._getdate_err))
#endif /* __USE_CYG */
#endif /* !_REENT_SMALL */

#ifdef __USE_CYG
#define _REENT_INIT_PTR(var) { memset((var),0,sizeof(*(var))); _REENT_INIT_PTR_ZEROED(var); }
#define _Kmax (sizeof(size_t) << 3)
#endif /* !__USE_CYG */

#if (!defined(__NO_ASMNAME) || defined(__USE_CYG))
__LIBC struct __cyg_reent *_impure_ptr;
__LIBC struct __cyg_reent *const _global_impure_ptr;
__LIBC void (__LIBCCALL _reclaim_reent)(struct __cyg_reent *);
#define __cyg_impure_ptr        _impure_ptr
#define __cyg_global_impure_ptr _global_impure_ptr
#define __cyg_reclaim_reent     _reclaim_reent
#else
__LIBC struct __cyg_reent *__cyg_impure_ptr __ASMNAME("_impure_ptr");
__LIBC struct __cyg_reent *const __cyg_global_impure_ptr __ASMNAME("_global_impure_ptr");
__LIBC void (__LIBCCALL __cyg_reclaim_reent)(struct __cyg_reent *) __ASMNAME("_reclaim_reent");
#endif

#ifdef __USE_REENTRANT
__LIBC struct __cyg_reent *(__LIBCCALL __getreent)(void);
#define __CYG_REENT   (__getreent())
#else /* __USE_REENTRANT */
#define __CYG_REENT    _impure_ptr
#endif /* !__USE_REENTRANT */

#undef __DEFINE_CYG_MEMBER_FUN
#undef __DEFINE_CYG_MEMBER

#ifdef __USE_CYG
#define _REENT         __CYG_REENT

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#ifdef _REENT_SMALL
#pragma pop_macro("_misc_reent")
#pragma pop_macro("_mprec")
#endif
#ifndef _REENT_GLOBAL_ATEXIT
#pragma pop_macro("_atexit0")
#endif /* _REENT_GLOBAL_ATEXIT */
#ifndef __SINGLE_THREAD__
#pragma pop_macro("_lock")
#endif /* __SINGLE_THREAD__ */
#ifdef _REENT_SMALL
#pragma pop_macro("_misc")
#pragma pop_macro("_data")
#pragma pop_macro("_mp")
#endif /* _REENT_SMALL */
#pragma pop_macro("_sig_func")
#pragma pop_macro("_new")
#pragma pop_macro("_unused")
#pragma pop_macro("_nmalloc")
#pragma pop_macro("_nextf")
#pragma pop_macro("_h_errno")
#pragma pop_macro("_wcsrtombs_state")
#pragma pop_macro("_wcrtomb_state")
#pragma pop_macro("_mbsrtowcs_state")
#pragma pop_macro("_mbrtowc_state")
#pragma pop_macro("_mbrlen_state")
#pragma pop_macro("_getdate_err")
#pragma pop_macro("_signal_buf")
#pragma pop_macro("_l64a_buf")
#pragma pop_macro("_wctomb_state")
#pragma pop_macro("_mbtowc_state")
#pragma pop_macro("_mblen_state")
#pragma pop_macro("_r48")
#pragma pop_macro("_rand_next")
#pragma pop_macro("_gamma_signgam")
#pragma pop_macro("_localtime_buf")
#pragma pop_macro("_asctime_buf")
#pragma pop_macro("_strtok_last")
#pragma pop_macro("_unused_rand")
#pragma pop_macro("_cvtbuf")
#pragma pop_macro("_cvtlen")
#pragma pop_macro("_freelist")
#pragma pop_macro("_p5s")
#pragma pop_macro("_result_k")
#pragma pop_macro("_result")
#pragma pop_macro("_locale")
#pragma pop_macro("_unspecified_locale_info")
#pragma pop_macro("_emergency")
#pragma pop_macro("_inc")
#pragma pop_macro("_stderr")
#pragma pop_macro("_stdout")
#pragma pop_macro("_stdin")
#pragma pop_macro("_errno")
#pragma pop_macro("_flags2")
#pragma pop_macro("_mbstate")
#pragma pop_macro("_offset")
#pragma pop_macro("_blksize")
#pragma pop_macro("_lb")
#pragma pop_macro("_nbuf")
#pragma pop_macro("_ubuf")
#pragma pop_macro("_ur")
#pragma pop_macro("_up")
#pragma pop_macro("_ub")
#pragma pop_macro("_close")
#pragma pop_macro("_seek")
#pragma pop_macro("_write")
#pragma pop_macro("_read")
#pragma pop_macro("_cookie")
#pragma pop_macro("_lbfsize")
#pragma pop_macro("_bf")
#pragma pop_macro("_file")
#pragma pop_macro("_flags")
#pragma pop_macro("_w")
#pragma pop_macro("_r")
#pragma pop_macro("_p")
#pragma pop_macro("_size")
#pragma pop_macro("_base")
#pragma pop_macro("_on_exit_args_ptr")
#pragma pop_macro("_fns")
#pragma pop_macro("_ind")
#pragma pop_macro("_is_cxa")
#pragma pop_macro("_fntypes")
#pragma pop_macro("_dso_handle")
#pragma pop_macro("_fnargs")
#pragma pop_macro("_x")
#pragma pop_macro("_wds")
#pragma pop_macro("_sign")
#pragma pop_macro("_maxwds")
#pragma pop_macro("_k")
#pragma pop_macro("_next")
#pragma pop_macro("_reent")
#pragma pop_macro("_rand48")
#pragma pop_macro("_glue")
#pragma pop_macro("_atexit")
#pragma pop_macro("_on_exit_args")
#pragma pop_macro("_Bigint")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */
#endif /* __USE_CYG */


__SYSDECL_END

#endif /* !_SYS_GENERIC_REENT_H */
