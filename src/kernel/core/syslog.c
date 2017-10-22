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
#ifndef GUARD_KERNEL_CORE_SYSLOG_C
#define GUARD_KERNEL_CORE_SYSLOG_C 1
#define _KOS_SOURCE 1

#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/syscall.h>
#include <kernel/syslog.h>
#include <kernel/user.h>
#include <modules/tty.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/io.h>
#include <sched/cpu.h>
#include <kernel/boot.h>

DECL_BEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

PRIVATE u8 const syslog_colors[LOG_PRIMASK+1] = {
   [0 ... LOG_PRIMASK] = TTY_DEFAULT_COLOR,
#if 1
   [LOG_EMERG]         = tty_entry_color(TTY_COLOR_WHITE,TTY_COLOR_DARK_GREY),
#else
   [LOG_EMERG]         = tty_entry_color(TTY_COLOR_RED,TTY_COLOR_BLACK),
#endif
   [LOG_ALERT]         = tty_entry_color(TTY_COLOR_LIGHT_RED,TTY_COLOR_BLACK),
   [LOG_CRIT]          = tty_entry_color(TTY_COLOR_MAGENTA,TTY_COLOR_BLACK),
   [LOG_ERR]           = tty_entry_color(TTY_COLOR_LIGHT_MAGENTA,TTY_COLOR_BLACK),
   [LOG_WARNING]       = tty_entry_color(TTY_COLOR_LIGHT_BROWN,TTY_COLOR_BLACK),
   [LOG_NOTICE]        = tty_entry_color(TTY_COLOR_LIGHT_GREEN,TTY_COLOR_BLACK),
#if 1
   [LOG_INFO]          = tty_entry_color(TTY_COLOR_WHITE,TTY_COLOR_BLACK),
#else
   [LOG_INFO]          = tty_entry_color(TTY_COLOR_LIGHT_GREY,TTY_COLOR_BLACK),
#endif
   [LOG_DEBUG]         = tty_entry_color(TTY_COLOR_DARK_GREY,TTY_COLOR_BLACK),
};

#pragma GCC diagnostic pop

FUNDEF ssize_t KCALL
syslog_print_tty(char const *__restrict data,
                 size_t datalen, void *closure) {
 TTY_PUSHCOLOR(syslog_colors[(uintptr_t)closure & LOG_PRIMASK]);
 tty_print(data,datalen);
 TTY_POPCOLOR();
 return (ssize_t)datalen;
}

FUNDEF ssize_t KCALL
syslog_print_serio(char const *__restrict data,
                   size_t datalen, void *closure) {
 if (boot_emulation == BOOT_EMULATION_QEMU) {
  outsb(boot_emulation_logport,data,datalen);
 } else {
  char const *end = data+datalen;
  /* TODO: This way of writing to serial only works in QEMU! */
  /* TODO: Proper serial communication module? */
  for (; data != end; ++data) outb(0x3F8,(unsigned char)*data);
 }
 return (ssize_t)datalen;
}


#if defined(__i386__) || defined(__x86_64__)
GLOBAL_ASM(
L(.section .data                                                              )
L(PUBLIC_ENTRY(syslog_printer)                                                )
L(    jmp syslog_print_default                                                )
L(syslog_target_rel_addr = . - __SIZEOF_POINTER__                             )
L(syslog_target_rel_base = .                                                  )
L(SYM_END(syslog_printer)                                                     )
L(.previous                                                                   )
);
INTDEF uintptr_t syslog_target_rel_addr;
INTDEF byte_t    syslog_target_rel_base[];

PUBLIC pformatprinter KCALL syslog_set_printer(pformatprinter printer) {
 uintptr_t rel_addr;
 rel_addr = (uintptr_t)printer-(uintptr_t)syslog_target_rel_base;
 rel_addr = ATOMIC_XCH(syslog_target_rel_addr,rel_addr);
 return (pformatprinter)((uintptr_t)syslog_target_rel_base+rel_addr);
}
#else
PRIVATE pformatprinter syslog_target = &syslog_print_default;
PUBLIC ssize_t (LIBCCALL syslog_printer)(char const *__restrict data,
                                         size_t datalen, void *closure) {
 return (*syslog_target)(data,datalen,closure);
}
PUBLIC pformatprinter KCALL syslog_set_printer(pformatprinter printer) {
 return ATOMIC_XCH(syslog_target,printer);
}
#endif



PUBLIC void (ATTR_CDECL syslog)(int level, char const *format, ...) {
 va_list args;
 va_start(args,format);
#ifdef CONFIG_DEBUG
 { bool was = PREEMPTION_ENABLED();
   format_vprintf(&syslog_printer,
                   SYSLOG_PRINTER_CLOSURE(level),
                   format,args);
   assert(was == PREEMPTION_ENABLED());
 }
#else
 format_vprintf(&syslog_printer,
                 SYSLOG_PRINTER_CLOSURE(level),
                 format,args);
#endif
 va_end(args);
}
PUBLIC void (LIBCCALL vsyslog)(int level, char const *format, va_list args) {
 format_vprintf(&syslog_printer,
                 SYSLOG_PRINTER_CLOSURE(level),
                 format,args);
}

#ifdef CONFIG_DEBUG
SYSCALL_DEFINE1(xpaused,USER char const *,message) {
 volatile unsigned int i = 0;
 syslog(LOG_CONFIRM,"PAUSED: %q (EIP = %p)\n",message,THIS_SYSCALL_EIP);
 while (++i < (1 << 29));
 return -EOK;
}
#endif

SYSCALL_DEFINE3(xsyslog,int,type,char const *,bufp,size_t,len) {
 char buf[64];
 ssize_t result = 0,temp;
 while (len > 0) {
  temp = MIN(len*sizeof(char),sizeof(buf));
  if (copy_from_user(buf,bufp,temp))
      return -EFAULT;
#if 0
  temp = format_printf(&syslog_printer,SYSLOG_PRINTER_CLOSURE(type),
                       "{%.?q}",temp/sizeof(char),buf);
#else
  temp = syslog_printer(buf,temp/sizeof(char),
                        SYSLOG_PRINTER_CLOSURE(type));
#endif
  if (temp < 0) return temp;
  if (temp == 0) break;
  result += temp;
  if ((size_t)temp >= len) break;
  len  -= temp;
  bufp += temp;
 }
 return result;
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_SYSLOG_C */
