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
#ifndef GUARD_APPS_MISC_DOSFS_C
#define GUARD_APPS_MISC_DOSFS_C 1
#define _KOS_SOURCE 1

#include <unistd.h> /* fsmask */
#include <fcntl.h>  /* AT_DOSPATH */
#include <errno.h>
#include <stdio.h>  /* fprintf() */
#include <stdlib.h> /* EXIT_FAILURE */
#include <hybrid/compiler.h>

DECL_BEGIN

int main(int argc, char **argv) {
	if (argc <= 1) {
		fprintf(stderr,"Usage: dosfs COMMAND\n");
		return EXIT_FAILURE;
	}

	/* Set the DOS filesystem mask. */
	fsmode((struct fsmask){
		.fm_mask = -1,
		.fm_mode = AT_DOSPATH,
	});

	/* Execute the given command. */
	execv(argv[1],argv+1);

	return errno;
}

DECL_END

#endif /* !GUARD_APPS_MISC_DOSFS_C */
