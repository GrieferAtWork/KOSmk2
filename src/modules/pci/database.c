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

/* Include the database itself. */
#pragma GCC visibility push(default)
#include "PCIHDR.H"
#pragma GCC visibility pop

/* In addition, export the length of the various tables.
 * >> Since tables aren't terminated by some sentinal, this is required for iteration. */
#include <hybrid/compiler.h>
#include <hybrid/types.h>
PUBLIC size_t pci_ventable_len       = PCI_VENTABLE_LEN;
PUBLIC size_t pci_devtable_len       = PCI_DEVTABLE_LEN;
PUBLIC size_t pci_classcodetable_len = PCI_CLASSCODETABLE_LEN;
PUBLIC size_t pci_commandflags_len   = PCI_COMMANDFLAGS_LEN;
PUBLIC size_t pci_statusflags_len    = PCI_STATUSFLAGS_LEN;
PUBLIC size_t pci_devselflags_len    = PCI_DEVSELFLAGS_LEN;


