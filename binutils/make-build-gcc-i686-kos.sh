#!/bin/bash

GCCBUILDFOLDER="build-gcc-i686-kos"
BUILDFOLDER="build-binutils-i686-kos"
ROOT=$(dirname $(readlink -f "$0"))
export PREFIX="$ROOT/$BUILDFOLDER"
export TARGET=i686-kos
export PATH="$PREFIX/bin:$PATH"

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || { echo $TARGET-as is not in the PATH; exit 1; }

cd "$ROOT/$GCCBUILDFOLDER"

../src/gcc-6.2.0/configure \
	--target="$TARGET" \
	--prefix="$PREFIX" \
	--with-sysroot=/opt/kos \
	--enable-languages=c,c++ \
	--enable-64-bit-bfd \
|| exit $?

# HINT: When something goes wrong here, try starting
#       over by clearing "build-gcc-i686-kos"
make all-gcc || exit $?
make all-target-libgcc || exit $?
make install-gcc || exit $?
make install-target-libgcc || exit $?

#make all-target-libstdc++-v3
#make install-target-libstdc++-v3
