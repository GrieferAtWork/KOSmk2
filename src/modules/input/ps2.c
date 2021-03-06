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
#ifndef GUARD_MODULES_INPUT_PS2_C
#define GUARD_MODULES_INPUT_PS2_C 1
#define _KOS_SOURCE 1

#include <dev/chrdev.h>
#include <fcntl.h>
#include <fs/atomic-iobuffer.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/compiler.h>
#include <hybrid/debug.h>
#include <hybrid/traceback.h>
#include <input/keyboard.h>
#include <arch/cpustate.h>
#include <kernel/export.h>
#include <kernel/interrupt.h>
#include <dev/rtc.h>
#include <sys/syslog.h>
#include <kernel/user.h>
#include <kos/keyboard.h>
#include <linker/module.h>
#include <malloc.h>
#include <modules/ps2.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/rpc.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <stddef.h>
#include <sync/sig.h>
#include <sys/io.h>
#include <kernel/mman.h>
#include <asm/instx.h>
#include <arch/pic.h>

#include "ps2_keymaps.h"

DECL_BEGIN

#if defined(CONFIG_DEBUG) && 1
#define HAVE_DUMP_TASK_STATES 1
PRIVATE void KCALL dump_task_states(void *UNUSED(closure)) {
 debug_printf("BOOT_TASK:\n");
 debug_tbprintl((void *)inittask.t_cstate->iret.xip,NULL,0);
 debug_tbprint2((void *)inittask.t_cstate->gp.xbp,0);
 debug_printf("IDLE_TASK:\n");
 debug_tbprintl((void *)THIS_CPU->c_idle.t_cstate->iret.xip,NULL,0);
 debug_tbprint2((void *)THIS_CPU->c_idle.t_cstate->gp.xbp,0);
 { pflag_t was = PREEMPTION_PUSH();
   struct task *start,*iter;
   iter = start = THIS_CPU->c_running;
   if (start) do {
    debug_printf("RUNNING TASK %p (PID = %d/%d) - %[file]\n",iter,
                 iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                 iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                 iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
    if (iter == THIS_TASK) {
#undef debug_tbprint
     debug_tbprint();
    } else {
     debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
     debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
    }
   } while ((iter = iter->t_sched.sd_running.re_next) != start);
   for (iter = THIS_CPU->c_idling;
        iter; iter = iter->t_sched.sd_running.re_next) {
    debug_printf("IDLING TASK %p (PID = %d/%d) - %[file]\n",iter,
                 iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                 iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                 iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
    if (iter == THIS_TASK) {
#undef debug_tbprint
     debug_tbprint();
    } else {
     debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
     debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
    }
   }
   task_crit();
#if 0
   if (cpu_tryread(THIS_CPU)) {
    cpu_validate_counters(false);
#endif
    for (iter = THIS_CPU->c_suspended; iter;
         iter = iter->t_sched.sd_suspended.le_next) {
     debug_printf("SUSPENDED TASK %p (PID = %d/%d) - %[file]\n",iter,
                  iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                  iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                  iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
     debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
     debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
    }
    for (iter = THIS_CPU->c_sleeping; iter;
         iter = iter->t_sched.sd_sleeping.le_next) {
     debug_printf("SLEEPING TASK %p (PID = %d/%d) - %[file]\n",iter,
                  iter->t_pid.tp_ids[PIDTYPE_GPID].tl_pid,
                  iter->t_pid.tp_ids[PIDTYPE_PID].tl_pid,
                  iter->t_real_mman->m_inst ? iter->t_real_mman->m_inst->i_module->m_file : NULL);
     debug_tbprintl((void *)iter->t_cstate->iret.xip,NULL,0);
     debug_tbprint2((void *)iter->t_cstate->gp.xbp,0);
    }
#if 0
    cpu_endread(THIS_CPU);
   }
   else {
    debug_printf("Failed to lock CPU for reading\n");
    cpu_validate_counters(true);
   }
#endif
   task_endcrit();
   PREEMPTION_POP(was);
 }
 debug_printf("==DONE\n");
}
PRIVATE struct job dump_task_states_job = JOB_INIT(&dump_task_states,NULL);
#endif

PRIVATE struct kbdev *ps2_keyboard;
PRIVATE void KCALL keyboard_send(kbkey_t k) {
#ifdef HAVE_DUMP_TASK_STATES
 if (k == KEY_F12)
     schedule_work(&dump_task_states_job);
#endif

#if 0
 { char c;
   if (k < 256 && (c = active_keymap.km_press[k]) != '\0') {
    syslog(LOG_HW|LOG_DEBUG,"Sending key %#.4I16x (`%#:1q')\n",k,&c);
   } else {
    syslog(LOG_HW|LOG_DEBUG,"Sending key %#.4I16x\n",k);
   }
 }
#endif

 if (!keyboard_putc(&ps2_keyboard->kb_device,k)) {
  syslog(LOG_HW|LOG_WARN,
         "[KBD] Failed to queue key press %#.2I16x: buffer is full\n",k);
 }
}

/* TODO: Use the interrupt controller to hold a reference to `ps2_keyboard' */
PRIVATE void INTCALL ps2_interrupt1_handler(void);
PRIVATE void INTCALL ps2_interruptc_handler(void);
PRIVATE struct interrupt ps2_interrupt1 = {
    .i_intno = INTNO_PIC1_KBD,
    .i_mode  = INTMODE_HW,
    .i_type  = INTTYPE_FAST|INTTYPE_NOSHARE,
    .i_prio  = INTPRIO_MAX,
    .i_flags = INTFLAG_PRIMARY,
    .i_proto = {
        .p_noshare_fast = &ps2_interrupt1_handler,
    },
    .i_owner = THIS_INSTANCE,
};
PRIVATE struct interrupt ps2_interruptc = {
    .i_intno = INTNO_PIC2_PS2M,
    .i_mode  = INTMODE_HW,
    .i_type  = INTTYPE_FAST|INTTYPE_NOSHARE,
    .i_prio  = INTPRIO_MAX,
    .i_flags = INTFLAG_PRIMARY,
    .i_proto = {
        .p_noshare_fast = &ps2_interruptc_handler,
    },
    .i_owner = THIS_INSTANCE,
};

PRIVATE u8 ps2_cmd_maxretry = 3; /* TODO: Commandline option? */
PRIVATE jtime_t ps2_cmd_timeout = MSEC_TO_JIFFIES(100); /* TODO: Commandline option? */

PRIVATE struct ps2 p = {
    /* Start out by ignoring all input events. */
    .p_state  = STATE_IGNORE,
    .p_state2 = STATE_IGNORE,
    .p_flags  = 0,
    .p_retry  = 0,
    .p_cmdf   = NULL,
    .p_cmdb   = NULL,
};

LOCAL void KCALL ps2_write_cmd(u8 cmd) {
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
 outb_p(PS2_CMD,cmd);
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
}
LOCAL void KCALL ps2_write_data(u8 data) {
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
 outb_p(PS2_DATA,data);
}
PRIVATE void KCALL
ps2_start_command(struct ps2_cmd *__restrict c) {
#if 0
 syslog(LOG_HW|LOG_DEBUG,"[PS2] Execute command %#.2I8x (port %d)\n",
        c->c_cmd,c->c_port&PS2_PORT2 ? 2 : 1);
#endif
 if (c->c_port&PS2_PORT2)
     ps2_write_cmd(PS2_CONTROLLER_WRITE_PORT2_INPUT);
 while (inb_p(PS2_STATUS) & PS2_STATUS_INFULL);
 outb_p(PS2_DATA,c->c_cmd);
}

PRIVATE bool KCALL
ps2_command(struct ps2_cmd *__restrict c) {
 pflag_t was; bool result = true;
 c->c_prev = NULL;
 c->c_next = NULL;
 memset(c->c_resp,ACK,PS2_MAXRESP+1);
 was = PREEMPTION_PUSH();
 assert((p.p_cmdf != NULL) == (p.p_cmdb != NULL));
 if (p.p_cmdf) {
  assert(p.p_state == STATE_COMMAND ||
         p.p_state == STATE_COMMANDARG ||
         p.p_state == STATE_COMMANDACK);
  c->c_next = p.p_cmdf;
  p.p_cmdf->c_prev = c;
  p.p_cmdf = c;
 } else {
  /* Send the command for the first time. */
  ps2_start_command(c);

  /* Setup the PS/2 descriptor to act
   * like it has execute the above command. */
  p.p_retry  = 0;
  p.p_state2 = p.p_state;
  p.p_cmdf   = c;
  p.p_cmdb   = c;
  p.p_state  = c->c_port&PS2_NOACK ? STATE_COMMANDACK : STATE_COMMAND;
 }

 /* Wait for the interrupt to arrive (HINT: `sti' only enables interrupts after 'hlt') */
 __asm__ __volatile__("sti\nhlt\ncli\n" : : : "memory");
 if (p.p_cmdb == c) {
  /* To optimize for quick hardware, only setup a
   * timeout after the first interrupt-wait failed.
   * >> This way we reduce load from the RTC chip. */
  jtime_t start;
wait_again:
  start = jiffies;
  for (;;) {
   __asm__ __volatile__("sti\nhlt\ncli\n" : : : "memory");
   if (p.p_cmdb != c) break;
   if ((jiffies-start) >= ps2_cmd_timeout) {
    assert(p.p_cmdb == c);
    if (++p.p_retry < ps2_cmd_maxretry) {
     /* Try to send the command again (I've seen it never arriving before...) */
     ps2_start_command(p.p_cmdb);
     goto wait_again;
    }
    /* Delete the command. */
    p.p_cmdb  = p.p_cmdb->c_prev;
    p.p_retry = 0;
    if (p.p_cmdb) {
     p.p_cmdb->c_next = NULL;
     /* Send the next command. */
     ps2_start_command(p.p_cmdb);
    } else {
     /* Switch back to the pre-command state. */
     p.p_state = p.p_state2;
     p.p_cmdf  = NULL;
    }
    memset(c->c_resp,RESEND,PS2_MAXRESP+1);
    result = false;
    break;
   }
  }
 }
 PREEMPTION_POP(was);
#if 1
 syslog(LOG_HW|LOG_DEBUG,"[PS2] Command finished: %d (%.2I8x,%.2I8x) %.2I8X %.2I8X %.2I8X %.2I8X %.2I8X %.2I8X\n",
        result,c->c_cmd,c->c_arg,
        c->c_resp[0],c->c_resp[1],c->c_resp[2],c->c_resp[3],c->c_resp[4],c->c_resp[5]);
#endif
 return result;
}


PRIVATE void KCALL unwind_scanset_1(u8 const *keys, u8 last) {
 syslog(LOG_HW|LOG_WARN,"[PS2] Unknown set-1 keycode {");
 for (; *keys; ++keys) syslog(LOG_HW|LOG_WARN,"%.2I8X-",*keys);
 syslog(LOG_HW|LOG_WARN,"%.2I8X}\n",last);
 for (; *keys; ++keys) keyboard_send(KEYMAP_GET_PS2_SCANSET1(*keys));
 keyboard_send(KEYMAP_GET_PS2_SCANSET1(last));
 p.p_state = STATE_INPUT_SET1;
}
PRIVATE void KCALL unwind_scanset_2(kbkey_t flag, u8 const *keys, u8 last) {
 syslog(LOG_HW|LOG_WARN,"[PS2] Unknown set-2 keycode {");
 for (; *keys; ++keys) syslog(LOG_HW|LOG_WARN,"%.2I8X-",*keys);
 syslog(LOG_HW|LOG_WARN,"%.2I8X}\n",last);
 for (; *keys; ++keys) keyboard_send(flag|keymap_ps2_scanset_2[*keys]);
 keyboard_send(flag|keymap_ps2_scanset_2[last]);
 p.p_state = STATE_INPUT_SET2;
}

PRIVATE void KCALL
ps2_handle_interrupt(void) {
 /* Read the scancode. */
 u8 e = inb(PS2_DATA);

#if 0
 syslog(LOG_HW|LOG_DEBUG,"[PS2] IRQ %#.2I8x (%d)\n",e,p.p_state);
#endif

 assert(!PREEMPTION_ENABLED());
 switch (p.p_state) {
 case STATE_IGNORE:
  /* Ignore everything! */
  syslog(LOG_HW|LOG_WARN,"[PS2] Ignoring interrupt for %#.2I8x\n",e);
  break;

 {
  u8 *dst,*end;
 case STATE_COMMAND:
 case STATE_COMMANDARG:
 case STATE_COMMANDACK:
  /* Service commands. */
  assert(p.p_cmdf);
  assert(p.p_cmdb);
  assert(!p.p_cmdf->c_prev);
  assert(!p.p_cmdb->c_next);

  if (e == RESEND) {
   if (++p.p_retry > ps2_cmd_maxretry) {
    /* NOTE: We exceed the response array by 1, so-as to also write `c_status' */
    memset(p.p_cmdb->c_resp,RESEND,PS2_MAXRESP+1);
    goto remove_cmd;
   }
   memset(p.p_cmdb->c_resp,ACK,PS2_MAXRESP+1);
   /* Retry the command. */
send_command:
   ps2_start_command(p.p_cmdb);
   p.p_state = p.p_cmdb->c_port&PS2_NOACK ? STATE_COMMANDACK : STATE_COMMAND;
   break;
  }
  end = (dst = p.p_cmdb->c_resp)+PS2_MAXRESP;
  for (; dst != end; ++dst) {
   if (*dst != ACK) continue;
   *dst = e;
   break;
  }

  /* When post-ack-mode, stop when enough data has been received. */
  if (p.p_state == STATE_COMMANDACK &&
      p.p_cmdb->c_port&PS2_HASACK &&
     ((dst+1)-p.p_cmdb->c_resp) >= p.p_cmdb->c_ackmax)
      goto remove_cmd;

  /* Remove the command. */
  if (e == ACK) {
   if (p.p_state != STATE_COMMANDARG &&
       p.p_cmdb->c_port&PS2_HASARG) {
    /* Must now send the command argument. */
    ps2_write_data(p.p_cmdb->c_arg);
    /* Reset the previous response. */
    memset(p.p_cmdb->c_resp,ACK,PS2_MAXRESP+1);
    p.p_state = STATE_COMMANDARG;
    break;
   }
   if (p.p_state != STATE_COMMANDACK &&
       p.p_cmdb->c_port&PS2_HASACK &&
       p.p_cmdb->c_ackmax != 0) {
    /* Reset the previous response. */
    memset(p.p_cmdb->c_resp,ACK,PS2_MAXRESP+1);
    /* Must receive more data after the ACK. */
    p.p_state = STATE_COMMANDACK;
    break;
   }
   p.p_cmdb->c_status = ACK;
remove_cmd:
   /* WARNING: We leave the `c_prev' of the finished command dangling! */
   { struct ps2_cmd *next_cmd = p.p_cmdb->c_prev;
     if (p.p_cmdb->c_port&PS2_FREECMD) free(p.p_cmdb);
     p.p_cmdb = next_cmd;
     p.p_retry = 0; /* Reset the retry counter. */
     if (p.p_cmdb) {
      p.p_cmdb->c_next = NULL;
      /* Send the next command. */
      goto send_command;
     }
     /* Switch back to the pre-command state. */
     p.p_state = p.p_state2;
     p.p_cmdf  = NULL;
   }
  }
 } break;

 /* Key event processing */
#define UNWIND(...) \
  { PRIVATE u8 const _k[] = {__VA_ARGS__,0}; \
    unwind_scanset_1(_k,e); \
    break; \
  }
#define SWITCH_IF(x,s) if (e == x) { p.p_state = s; break; }

 {
  kbkey_t key;
 case STATE_INPUT_SET1_E0:
  if (e == 0x2a) { p.p_state = STATE_INPUT_SET1_E0_2A; break; }
  if (e == 0xb7) { p.p_state = STATE_INPUT_SET1_E0_B7; break; }
  key = KEYMAP_GET_PS2_SCANSET1_E0(e);
  if (key == KEY_UNKNOWN) { UNWIND(0xe0) }
  keyboard_send(key);
  p.p_state = STATE_INPUT_SET1;
 } break;
 case STATE_INPUT_SET1_E0_2A      : SWITCH_IF(0xe0,STATE_INPUT_SET1_E0_2A_E0)       UNWIND(0xe0,0x2a)
 case STATE_INPUT_SET1_E0_B7      : SWITCH_IF(0xe0,STATE_INPUT_SET1_E0_B7_E0)       UNWIND(0xe0,0xb7)
 case STATE_INPUT_SET1_E1         : SWITCH_IF(0x1d,STATE_INPUT_SET1_E1_1D)          UNWIND(0xe1)
 case STATE_INPUT_SET1_E1_1D      : SWITCH_IF(0x45,STATE_INPUT_SET1_E1_1D_45)       UNWIND(0xe1,0x1d)
 case STATE_INPUT_SET1_E1_1D_45   : SWITCH_IF(0xe1,STATE_INPUT_SET1_E1_1D_45_E1)    UNWIND(0xe1,0x1d,0x45)
 case STATE_INPUT_SET1_E1_1D_45_E1: SWITCH_IF(0x9d,STATE_INPUT_SET1_E1_1D_45_E1_9D) UNWIND(0xe1,0x1d,0x45,0xe1)
 case STATE_INPUT_SET1_E0_2A_E0:
  if (e == 0x37) {
   keyboard_send(KEY_PRESSED|KEY_PRINTSCREEN);
   p.p_state = STATE_INPUT_SET1;
   break;
  }
  UNWIND(0xe0,0x2a,0xe0)

 case STATE_INPUT_SET1_E0_B7_E0:
  if (e == 0xaa) {
   keyboard_send(KEY_RELEASED|KEY_PRINTSCREEN);
   p.p_state = STATE_INPUT_SET1;
   break;
  }
  UNWIND(0xe0,0x57,0xe0)

 case STATE_INPUT_SET1_E1_1D_45_E1_9D:
  if (e == 0xc5) {
   keyboard_send(KEY_PRESSED|KEY_PAUSE);
   keyboard_send(KEY_RELEASED|KEY_PAUSE);
   p.p_state = STATE_INPUT_SET1;
   break;
  }
  UNWIND(0x01,0x1d,0x45,0x9d)

 default: p.p_state = STATE_INPUT_SET1; /* Shouldn't happen. */
 case STATE_INPUT_SET1:
  if (e == 0xe0) { p.p_state = STATE_INPUT_SET1_E0; break; }
  if (e == 0xe1) { p.p_state = STATE_INPUT_SET1_E1; break; }
  keyboard_send(KEYMAP_GET_PS2_SCANSET1(e));
  break;
#undef UNWIND



#define UNWIND(f,...) \
  { PRIVATE u8 const _k[] = {__VA_ARGS__,0}; \
    unwind_scanset_2(f,_k,e); \
    break; \
  }
 {
  kbkey_t key;
 case STATE_INPUT_SET2:
  if (e == 0xf0) { p.p_state = STATE_INPUT_SET2_F0; break; }
  if (e == 0xe0) { p.p_state = STATE_INPUT_SET2_E0; break; }
  if (e == 0xe1) { p.p_state = STATE_INPUT_SET2_E1; break; }
  key = (kbkey_t)keymap_ps2_scanset_2[e];
  if (key == KEY_UNKNOWN)
      syslog(LOG_HW|LOG_WARN,"[PS2] Unknown set-2 keycode {%.2I8X}\n",e);
  keyboard_send(key);
  break;
 case STATE_INPUT_SET2_F0:
  key = (kbkey_t)keymap_ps2_scanset_2[e];
  if (key == KEY_UNKNOWN)
      syslog(LOG_HW|LOG_WARN,"[PS2] Unknown set-2 keycode {F0-%.2I8X}\n",e);
  keyboard_send(KEY_RELEASED|key);
  p.p_state = STATE_INPUT_SET2;
  break;
 case STATE_INPUT_SET2_E0:
  if (e == 0xf0) { p.p_state = STATE_INPUT_SET2_E0_F0; break; }
  if (e == 0x12) { p.p_state = STATE_INPUT_SET2_E0_12; break; }
  key = keymap_ps2_scanset_2_e0[e];
  if (key == KEY_UNKNOWN) { UNWIND(KEY_PRESSED,0xe0) }
  keyboard_send(key);
  p.p_state = STATE_INPUT_SET2;
  break;
 case STATE_INPUT_SET2_E0_F0:
  if (e == 0x7c) { p.p_state = STATE_INPUT_SET2_E0_F0_7C; break; }
  key = keymap_ps2_scanset_2_e0[e];
  if (key == KEY_UNKNOWN) { UNWIND(KEY_RELEASED,0xe0,0xf0) }
  keyboard_send(KEY_RELEASED|key);
  p.p_state = STATE_INPUT_SET2;
  break;
 }
 case STATE_INPUT_SET2_E0_12_E0:
  if (e == 0x7c) {
   keyboard_send(KEY_PRESSED|KEY_PRINTSCREEN);
   p.p_state = STATE_INPUT_SET2;
   break;
  }
  UNWIND(0xe0,0x12,0xe0)

 case STATE_INPUT_SET2_E0_F0_7C:          SWITCH_IF(0xe0,STATE_INPUT_SET2_E0_F0_7C_E0)          UNWIND(KEY_RELEASED,0xe0,0xf0,0x7c)
 case STATE_INPUT_SET2_E0_F0_7C_E0:       SWITCH_IF(0xf0,STATE_INPUT_SET2_E0_F0_7C_E0_F0)       UNWIND(KEY_RELEASED,0xe0,0xf0,0x7c,0xe0)
 case STATE_INPUT_SET2_E1:                SWITCH_IF(0x14,STATE_INPUT_SET2_E1_14)                UNWIND(KEY_RELEASED,0xe1)
 case STATE_INPUT_SET2_E1_14:             SWITCH_IF(0x77,STATE_INPUT_SET2_E1_14_77)             UNWIND(KEY_RELEASED,0xe1,0x14)
 case STATE_INPUT_SET2_E1_14_77:          SWITCH_IF(0xe1,STATE_INPUT_SET2_E1_14_77_E1)          UNWIND(KEY_RELEASED,0xe1,0x14,0x77)
 case STATE_INPUT_SET2_E1_14_77_E1:       SWITCH_IF(0xf0,STATE_INPUT_SET2_E1_14_77_E1_F0)       UNWIND(KEY_RELEASED,0xe1,0x14,0x77,0xe1)
 case STATE_INPUT_SET2_E1_14_77_E1_F0:    SWITCH_IF(0x14,STATE_INPUT_SET2_E1_14_77_E1_F0_14)    UNWIND(KEY_RELEASED,0xe1,0x14,0x77,0xe1,0xf0)
 case STATE_INPUT_SET2_E1_14_77_E1_F0_14: SWITCH_IF(0xf0,STATE_INPUT_SET2_E1_14_77_E1_F0_14_F0) UNWIND(KEY_RELEASED,0xe1,0x14,0x77,0xe1,0xf0,0x14)
 case STATE_INPUT_SET2_E0_F0_7C_E0_F0:
  if (e != 0x12) { UNWIND(0xe0,0xf0,0x7c,0xe0,0xf0) }
  keyboard_send(KEY_RELEASED|KEY_PRINTSCREEN);
  p.p_state = STATE_INPUT_SET2;
  break;
 case STATE_INPUT_SET2_E1_14_77_E1_F0_14_F0:
  if (e != 0x12) { UNWIND(0xe1,0x14,0x77,0xe1,0xf0,0x14,0xf0) }
  keyboard_send(KEY_PRESSED|KEY_PAUSE);
  keyboard_send(KEY_RELEASED|KEY_PAUSE);
  p.p_state = STATE_INPUT_SET2;
  break;

#undef UNWIND

 {
  kbkey_t key;
 case STATE_INPUT_SET3:
  if (e == 0xf0) { p.p_state = STATE_INPUT_SET3_F0; break; }
  key = keymap_ps2_scanset_3[e];
  if (key == KEY_UNKNOWN)
      syslog(LOG_HW|LOG_WARN,"[PS2] Unknown set-3 keycode {%.2I8X}\n",e);
  keyboard_send(key);
  break;
 case STATE_INPUT_SET3_F0:
  key = keymap_ps2_scanset_3[e];
  if (key == KEY_UNKNOWN)
      syslog(LOG_HW|LOG_WARN,"[PS2] Unknown set-3 keycode {F0-%.2I8X}\n",e);
  keyboard_send(KEY_RELEASED|key);
  p.p_state = STATE_INPUT_SET3;
 } break;

 }
}

PRIVATE void KCALL
keyboard_irqctl(struct device *__restrict dev, unsigned int cmd) {
 /* XXX: Disable keyboard interrupts before BIOS? */
 if (cmd != IRQCTL_ENABLE) return;
 /* Check if keyboard data arrived while we were gone. */
 if (inb(PS2_STATUS) & PS2_STATUS_OUTFULL) {
  pflag_t was;
  syslog(LOG_HW|LOG_INFO,"[PS2] Handling lost interrupt\n");
  was = PREEMPTION_PUSH();
  ps2_handle_interrupt();
  PREEMPTION_POP(was);
 }
}


PRIVATE ATTR_USED void INTCALL ps2_interrupt1_handler(void) {
 ps2_handle_interrupt();
 PIC_EOI(INTNO_PIC1_KBD);
}
PRIVATE ATTR_USED void INTCALL ps2_interruptc_handler(void) {
 /* TODO: PS/2 mouse event? */
 PIC_EOI(INTNO_PIC2_PS2M);
}


/* Helper function for executing device commands. */
PRIVATE u8 KCALL
ps2_device_command(u8 port_and_flags, u8 cmd) {
 struct ps2_cmd c;
 c.c_port = port_and_flags;
 c.c_cmd  = cmd;
 ps2_command(&c);
 return c.c_resp[0];
}

PRIVATE ATTR_UNUSED bool KCALL ps2_keyboard_echo(u8 port);
PRIVATE ATTR_UNUSED bool KCALL ps2_keyboard_reset(u8 port); /* Reset & start self-test. */
PRIVATE ATTR_UNUSED u8   KCALL ps2_keyboard_get_scanset(u8 port);
PRIVATE ATTR_UNUSED bool KCALL ps2_keyboard_set_scanset(u8 port, u8 v);

/* Preferred scan-set. */
#define PS2_PREFERRED_SCANSET 2

PRIVATE MODULE_INIT errno_t ps2_init(void) {
 errno_t error; u8 ps_scanset,ps_syscfg,port;

 /* Disable both PS/2 ports */
 ps2_write_cmd(PS2_CONTROLLER_DISABLE_PORT1);
 if (p.p_flags&PS2_HAVE_PORT2)
     ps2_write_cmd(PS2_CONTROLLER_DISABLE_PORT2);

 /* Install the PS/2 interrupt handlers. */
 int_addboot(&ps2_interrupt1);
 int_addboot(&ps2_interruptc);

 ps_syscfg = (PS2_CONTROLLER_CFG_PORT1_IRQ|
              PS2_CONTROLLER_CFG_SYSTEMFLAG);
 if (p.p_flags&PS2_HAVE_PORT2)
     ps_syscfg |= PS2_CONTROLLER_CFG_PORT2_IRQ;

 /* Setup our initial PS/2 configuration. */
 ps2_write_cmd(PS2_CONTROLLER_WRAM(0)); /* Read RAM #0 */
 ps2_write_data(ps_syscfg);

 /* Enable the first PS/2 port */
 ps2_write_cmd(PS2_CONTROLLER_ENABLE_PORT1);

 /* Wait a bit here, in case interrupts were send above... */
 //{ volatile int x = 10000;
 //  while (x--);
 //}

#if 0
 port = PS2_PORT1;
 goto got_keyboard;
#endif

 /* Search for an attached keyboard. */
 for (port = PS2_PORT1;; ++port) {
  if (port == PS2_PORT2 && !(p.p_flags&PS2_HAVE_PORT2)) goto no_keyboard;
  if (port > PS2_PORT2) goto no_keyboard;

  /* Reset + self-test. */
  if (!ps2_keyboard_echo(port)) continue;
  if (!ps2_keyboard_reset(port)) continue;

  break;
 }

got_keyboard:
 ps2_device_command(port,0xf6); /* Set defaults. */
 ps2_device_command(port,0xf5); /* Disable scanning. */
 ps2_keyboard_set_scanset(port,PS2_PREFERRED_SCANSET); /* Set scancode set. */
 ps_scanset = ps2_keyboard_get_scanset(port); /* Get scancode set. */

 if (ps_scanset > 3) {
  /* Failed to set scanset #2 */
  ps2_device_command(port,0xff); /* Reset + self-test. */
  ps2_device_command(port,0xf6); /* Set defaults. */
  ps_scanset = ps2_keyboard_get_scanset(port); /* Get default scancode set. */
 }
 if (ps_scanset) --ps_scanset;
 ps_scanset &= PS2_SCANSET;
 if (ps_scanset == 2) ps_scanset = 0; /* Fix illegal scansets. */
 syslog(LOG_HW|LOG_INFO,"[PS2] Using keyboard scanset %I8u\n",ps_scanset+1);
 p.p_flags |= ps_scanset;
 ps2_device_command(port,0xf4); /* Enable scanning. */

 /* Switch to input-accept-mode. */
 assert(!p.p_cmdf);
 assert(!p.p_cmdb);

 /* Select the scanset-specific state machine ring. */
 p.p_state = STATE_INPUT_SET1+ps_scanset;

 ps2_keyboard = (struct kbdev *)keyboard_new(sizeof(struct kbdev));
 if unlikely(!ps2_keyboard) return -ENOMEM;
 ps2_keyboard->kb_port                                = port;
 ps2_keyboard->kb_device.k_device.cd_device.d_irq_ctl = &keyboard_irqctl;
 error = device_setup(&ps2_keyboard->kb_device.k_device.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) goto err;

 /* Register the PS/2 keyboard driver device file. */
 CHRDEV_REGISTER(ps2_keyboard,DV_PS2_KEYBOARD);

 /* Install a new default keyboard device. */
 set_default_keyboard(&ps2_keyboard->kb_device,false);

 KEYBOARD_DECREF(&ps2_keyboard->kb_device);

 return -EOK;
err:
 free(ps2_keyboard);
 ps2_keyboard = NULL;
 return error;
no_keyboard:
 syslog(LOG_HW|LOG_WARN,"[PS2] No keyboard found (Assuming one is connected to port #1)\n");
 ps2_keyboard_reset(PS2_PORT1);
 port = PS2_PORT1;
 goto got_keyboard;
}

PRIVATE bool KCALL ps2_keyboard_echo(u8 port) {
 struct ps2_cmd c;
 c.c_port   = PS2_HASACK|PS2_NOACK|port;
 c.c_cmd    = KEYBOARD_ECHO;
 c.c_ackmax = 1;
 ps2_command(&c);
 return c.c_resp[0] == 0xee;
}
PRIVATE bool KCALL ps2_keyboard_reset(u8 port) {
 struct ps2_cmd c;
 c.c_port   = PS2_HASACK|port;
 c.c_cmd    = KEYBOARD_RESET;
 c.c_ackmax = 1;
 return ps2_command(&c) && c.c_resp[0] == 0xaa;
}
PRIVATE u8 KCALL ps2_keyboard_get_scanset(u8 port) {
 struct ps2_cmd c;
 c.c_port   = PS2_HASARG|PS2_HASACK|port;
 c.c_cmd    = KEYBOARD_SCANSET;
 c.c_arg    = 0;
 c.c_ackmax = 1;
 ps2_command(&c);
 return c.c_resp[0];
}
PRIVATE bool KCALL ps2_keyboard_set_scanset(u8 port, u8 v) {
 struct ps2_cmd c;
 c.c_port   = PS2_HASARG|port;
 c.c_cmd    = KEYBOARD_SCANSET;
 c.c_arg    = v;
 ps2_command(&c);
 return c.c_status == ACK;
}

DECL_END

#endif /* !GUARD_MODULES_INPUT_PS2_C */
