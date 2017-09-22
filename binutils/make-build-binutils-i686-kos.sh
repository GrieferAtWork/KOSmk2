#!/bin/bash

BUILDFOLDER="build-binutils-i686-kos"
ROOT=$(dirname $(readlink -f "$0"))
export PREFIX="$ROOT/$BUILDFOLDER"
export TARGET=i686-kos
export PATH="$PREFIX/bin:$PATH"

cd "$PREFIX"

if ! [ -f "Makefile" ]; then
	../src/binutils-2.27/configure \
		--target=$TARGET \
		--enable-64-bit-bfd \
		--prefix="$PREFIX" \
		--with-sysroot=/opt/kos \
		--with-headers=/opt/kos/usr/include \
		--disable-nls \
		--disable-werror \
		--enable-multilib \
	|| exit $?
fi

make -j 8 || exit $?
make -j 8 install || exit $?
