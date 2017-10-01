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

#if 1
#include <kos/thread.h>
#include <stdio.h>

int main(int argc, char **argv) {
	printf("tib             = %p\n",THIS_TIB->ti_self);
	printf("tib->ti_stacklo = %p\n",THIS_TIB->ti_stacklo);
	printf("tib->ti_stackhi = %p\n",THIS_TIB->ti_stackhi);
	return 0;
}
#else
#define _DOS_SOURCE 1
 
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>

int main(int argc, char **argv) {
	wchar_t **iter = _wenviron;
	while (*iter) {
		printf("ENV: %ls\n",*iter);
		++iter;
	}
	iter = __wargv;
	while (*iter) {
		printf("ARG: %ls\n",*iter);
		++iter;
	}
	printf("APP: %ls\n",_wpgmptr);
	return 0;
}
#endif


