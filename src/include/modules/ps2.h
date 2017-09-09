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
#ifndef GUARD_INCLUDE_MODULES_PS2_H
#define GUARD_INCLUDE_MODULES_PS2_H 1

#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>
#include <dev/chrdev.h>
#include <sync/sig.h>

DECL_BEGIN

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

/* PS/2 status port flags. */
#define PS2_STATUS_OUTFULL              0x01
#define PS2_STATUS_INFULL               0x02
#define PS2_STATUS_SYSTEM               0x04
#define PS2_STATUS_IN_IS_CONTROLLER_CMD 0x08
#define PS2_STATUS_KB_ENABLED           0x10
#define PS2_STATUS_SEND_TIMEOUT_ERROR   0x20
#define PS2_STATUS_RECV_TIMEOUT_ERROR   0x40
#define PS2_STATUS_PARITY_ERROR         0x80

#define ACK    0xFA
#define RESEND 0xFE

#define LED_SCROLLLOCK (1 << 0)
#define LED_NUMLOCK    (1 << 1)
#define LED_CAPSLOCK   (1 << 2)

/* PS/2 controller command codes (For use with 'struct ps2_cmd'). */
#define PS2_CONTROLLER_RRAM(n)           (0x20+(n))
#define PS2_CONTROLLER_WRAM(n)           (0x60+(n))
#define PS2_CONTROLLER_TEST_PORT1         0xaa
#define PS2_CONTROLLER_ENABLE_PORT1       0xae
#define PS2_CONTROLLER_DISABLE_PORT1      0xad
#define PS2_CONTROLLER_TEST_PORT2         0xa9
#define PS2_CONTROLLER_ENABLE_PORT2       0xa8
#define PS2_CONTROLLER_DISABLE_PORT2      0xa7
#define PS2_CONTROLLER_READ_OUTPUT        0xd0
#define PS2_CONTROLLER_WRITE_OUTPUT       0xd1
#define PS2_CONTROLLER_WRITE_PORT1_OUTPUT 0xd2
#define PS2_CONTROLLER_WRITE_PORT2_OUTPUT 0xd3
#define PS2_CONTROLLER_WRITE_PORT2_INPUT  0xd4

/* Layout of the 'PS2_CONTROLLER_RRAM(0)' (Controller configuration) byte. */
#define PS2_CONTROLLER_CFG_PORT1_IRQ       0x01
#define PS2_CONTROLLER_CFG_PORT2_IRQ       0x02
#define PS2_CONTROLLER_CFG_SYSTEMFLAG      0x04
#define PS2_CONTROLLER_CFG_PORT1_CLOCK     0x10
#define PS2_CONTROLLER_CFG_PORT2_CLOCK     0x20
#define PS2_CONTROLLER_CFG_PORT1_TRANSLATE 0x40

/* PS/2 device command codes (For use with 'struct ps2_cmd'). */
#define KEYBOARD_SETLED     0xed /* PS2_DATA: Set of 'LED_*' */
#define KEYBOARD_ECHO       0xee /* Responds with '0xee' if a keyboard was found. */
#define KEYBOARD_SCANSET    0xf0
#define KEYBOARD_RESET      0xff
#define KEYBOARD_SETDEFAULT 0xff

#define PS2_PORT1    0x00 /*< Send command to port #1 */
#define PS2_PORT2    0x01 /*< Send command to port #2 */

struct kbdev {
 struct chrdev kb_device;  /*< Underlying character device. */
 u8            kb_port;    /*< The keyboard port (One of 'PS2_PORT*') */
};


#define PS2_FREECMD  0x10 /*< free() the cmd once execution is done. */
#define PS2_NOACK    0x20 /*< Immediately switch to 'STATE_COMMANDACK'. */
#define PS2_HASACK   0x40 /*< Once an ACK was received, discard the previous response and fill it with new data. */
#define PS2_HASARG   0x80 /*< After 'c_cmd' is done, discard its response and write 'c_arg' to 'PS2_DATA'. */
#define PS2_MAXRESP  15
struct ps2_cmd {
 struct ps2_cmd *c_prev;              /*< [0..1] Previous command to-be executed after this one. */
 struct ps2_cmd *c_next;              /*< [0..1] Next command that is being executed before this one. */
 u8              c_port;              /*< One of 'PS2_PORT*', or'd with flags (PS2_HASARG, etc.). */
 u8              c_cmd;               /*< Command to execute (One of 'KEYBOARD_*'). */
 u8              c_arg;               /*< [valid_if(PS2_HASARG)] Command argument. */
 u8              c_ackmax;            /*< [valid_if(PS2_HASACK)] Max amount of bytes to receive after the last 'ACK'. */
 u8              c_resp[PS2_MAXRESP]; /*< Response text (terminated by 'ACK'). */
 u8              c_status;            /*< The final status byte that completed the command ACK/RESEND. */
};



/* PS/2 controller states */
#define STATE_IGNORE      0x00 /* Ignore all PS/2 interrupts. */
#define STATE_COMMAND     0x01 /* Waiting for response to a command being executed. */
#define STATE_COMMANDARG  0x02 /* Waiting for response after a command argument was written. */
#define STATE_COMMANDACK  0x03 /* Waiting for additional data after ACK has been received. */
#define STATE_INPUT_SET1  0x04 /* Process input keys in set #1 */
#define STATE_INPUT_SET2  0x05 /* Process input keys in set #2 */
#define STATE_INPUT_SET3  0x06 /* Process input keys in set #3 */

#define STATE_INPUT_SET1_E0                   0x20
#define STATE_INPUT_SET1_E0_2A                0x21
#define STATE_INPUT_SET1_E0_2A_E0             0x22
#define STATE_INPUT_SET1_E0_B7                0x23
#define STATE_INPUT_SET1_E0_B7_E0             0x24
#define STATE_INPUT_SET1_E1                   0x25
#define STATE_INPUT_SET1_E1_1D                0x26
#define STATE_INPUT_SET1_E1_1D_45             0x27
#define STATE_INPUT_SET1_E1_1D_45_E1          0x28
#define STATE_INPUT_SET1_E1_1D_45_E1_9D       0x29

#define STATE_INPUT_SET2_F0                   0x40
#define STATE_INPUT_SET2_E0                   0x41
#define STATE_INPUT_SET2_E0_F0                0x42
#define STATE_INPUT_SET2_E0_12                0x43
#define STATE_INPUT_SET2_E0_12_E0             0x44
#define STATE_INPUT_SET2_E0_F0_7C             0x45
#define STATE_INPUT_SET2_E0_F0_7C_E0          0x46
#define STATE_INPUT_SET2_E0_F0_7C_E0_F0       0x47
#define STATE_INPUT_SET2_E1                   0x48
#define STATE_INPUT_SET2_E1_14                0x49
#define STATE_INPUT_SET2_E1_14_77             0x4a
#define STATE_INPUT_SET2_E1_14_77_E1          0x4b
#define STATE_INPUT_SET2_E1_14_77_E1_F0       0x4c
#define STATE_INPUT_SET2_E1_14_77_E1_F0_14    0x4d
#define STATE_INPUT_SET2_E1_14_77_E1_F0_14_F0 0x4e

#define STATE_INPUT_SET3_F0                   0x60

struct ps2 {
 u8              p_state; /*< Current state of the state machine running under 'IRQ_PIC1_KBD'. */
 u8              p_state2;/*< State to return to once all commands have been served. */
#define PS2_SCANSET    0x03 /*< Mask for the keyboard scanset used by the keyboard. */
#define PS2_HAVE_PORT2 0x80 /*< Set if the second PS/2 is supported by the controller. */
 u8              p_flags; /*< Set of 'PS2_HAVE_*' */
 u8              p_retry; /*< Amount of retry attempts for the current command. */
 struct ps2_cmd *p_cmdf;  /*< [0..1] The first PS/2 command (list head). */
 struct ps2_cmd *p_cmdb;  /*< [0..1] The current PS/2 command (list tail). */
};



DECL_END

#endif /* !GUARD_INCLUDE_MODULES_PS2_H */
