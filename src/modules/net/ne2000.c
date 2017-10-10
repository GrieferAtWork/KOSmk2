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
#ifndef GUARD_MODULES_NET_NE2000_C
#define GUARD_MODULES_NET_NE2000_C 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <dev/net.h>
#include <modules/pci.h>

DECL_BEGIN

PRIVATE void MODULE_INIT KCALL net_init(void) {
 struct pci_device *dev;
 PCI_READ();
 /* Search for network cards. */
 PCI_FOREACH_CLASS(dev,PCI_DEV8_CLASS_NETWORK,
                   PCI_DEV8_CLASS_NOCLASS) {
  /* Try to enable the device. */
  if (E_ISERR(pci_enable(dev->pd_addr))) continue;


 }
 PCI_ENDREAD();
}

DECL_END

#endif /* !GUARD_MODULES_NET_NE2000_C */
