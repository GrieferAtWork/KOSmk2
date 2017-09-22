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

if ! [ -f "Makefile" ]; then
	../src/gcc-6.2.0/configure \
		--target=$TARGET \
		--prefix="$PREFIX" \
		--with-sysroot=/opt/kos \
		--with-tune=generic \
		--with-gnu-ld \
		--with-gnu-as \
		--with-dwarf2 \
		--with-cpu-32=i686 \
		--with-arch-32=i686 \
		--with-tune-32=i686 \
		--with-cpu-64=x86-64 \
		--with-arch-64=x86-64 \
		--with-tune-64=x86-64 \
		--enable-gnu-unique-object \
		--disable-vtable-verify \
		--enable-threads=single \
		--enable-targets=all \
		--enable-languages=c,c++ \
		--disable-nls \
		--enable-multiarch \
		--enable-multilib \
		--enable-64-bit-bfd \
		--with-multilib-list=m32,m64,mx32 \
		--enable-initfini-array \
		--enable-__cxa_atexit \
	|| exit $?
fi


# HINT: When something goes wrong here, try starting
#       over by clearing "build-gcc-i686-kos"
make -j 8 all-gcc || exit $?
make -j 8 all-target-libgcc || exit $?
make -j 8 install-gcc || exit $?
make -j 8 install-target-libgcc || exit $?

#make -j 8 all-target-libstdc++-v3 || exit $?
#make -j 8 install-target-libstdc++-v3 || exit $?


