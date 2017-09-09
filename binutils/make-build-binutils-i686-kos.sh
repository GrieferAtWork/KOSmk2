#!/bin/bash

BUILDFOLDER="build-binutils-i686-kos"
ROOT=$(dirname $(readlink -f "$0"))
export PREFIX="$ROOT/$BUILDFOLDER"
export TARGET=i686-kos
export PATH="$PREFIX/bin:$PATH"

cd "$PREFIX"

../src/binutils-2.27/configure \
	--target=$TARGET \
	--prefix="$PREFIX" \
	--with-sysroot=/opt/kos \
	--disable-nls \
	--disable-werror \
	--enable-64-bit-bfd \
|| exit $?

# HINT: When something goes wrong here, try starting
#       over by clearing "build-binutils-i686-kos"
make || exit $?
make install || exit $?
