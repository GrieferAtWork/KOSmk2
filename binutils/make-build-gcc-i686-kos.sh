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
#make -j 8 all-gcc || exit $?
#make -j 8 all-target-libgcc || exit $?
#make -j 8 install-gcc || exit $?
#make -j 8 install-target-libgcc || exit $?
#make -j 8 all-target-libstdc++-v3 || exit $?
#make -j 8 install-target-libstdc++-v3 || exit $?

# Create the symlink that's used for wrapping libstdc++ headers.
mv "$PREFIX/i686-kos/include/c++"  "$PREFIX/i686-kos/include-c++"
ln -s "../../binutils/$BUILDFOLDER/i686-kos/include-c++/6.2.0"  "$ROOT/../include/c++/6.2"
ln -s "6.2" "$ROOT/../include/c++/6"
ln -s "6.2" "$ROOT/../include/c++/current"

# Copy libstdc++ files into KOS's lib folder.
cp "$PREFIX/i686-kos/lib/libstdc++.a"         "$ROOT/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libstdc++.la"        "$ROOT/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libstdc++.a-gdb.py"  "$ROOT/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libsupc++.a"         "$ROOT/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libsupc++.la"        "$ROOT/../bin/libs/"


# Get rid of broken ~FiXiNcLuDe~ headers that we don't want
disable_fixinclude() {
	mv "$PREFIX/lib/gcc/i686-kos/6.2.0/include/$1" "$PREFIX/lib/gcc/i686-kos/6.2.0/include/$1.nope"
}
disable_fixinclude2() {
	mv "$PREFIX/lib/gcc/i686-kos/6.2.0/include-fixed/$1" "$PREFIX/lib/gcc/i686-kos/6.2.0/include-fixed/$1.nope"
}

# These are headers that _WE_ are implementing. - So shut your face GCC
disable_fixinclude "stddef.h"
disable_fixinclude "stdbool.h"
disable_fixinclude "stdarg.h"
disable_fixinclude "stdnoreturn.h"
disable_fixinclude "stdalign.h"
disable_fixinclude "float.h"
disable_fixinclude "iso646.h"
disable_fixinclude2 "limits.h"

# The header missing should really be the better indicator...
disable_fixinclude "varargs.h"

# Switch back to the binutils root folder and fix stdlibc++ header files.
cd "$ROOT"
deemon "fix_stdlibcxx_relations.dee"


