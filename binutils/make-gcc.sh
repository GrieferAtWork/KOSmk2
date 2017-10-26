#!/bin/bash

gcc_build_folder="build-gcc-i686-kos"
binutils_build_folder="build-binutils-i686-kos"
binutils_folder=$(dirname $(readlink -f "$0"))
binutils_syshook="$binutils_folder/root"
gcc_src_folder="$binutils_folder/src/gcc-6.2.0"
if ! [ -z "$CONFIG_GCC_PREFIX" ]; then
	binutils_syshook="$CONFIG_GCC_PREFIX"
fi

export PREFIX="${binutils_folder}/${binutils_build_folder}"
export TARGET=i686-kos
export PATH="$PREFIX/bin:$PATH"

cmd() {
	$* || {
		local error=$?
		echo "Command failed '$*' -> '$error'"
		exit $error
	}
}

if ! [ -f "$gcc_src_folder/configure" ]; then
	if ! [ -f "gcc-6.2.0.tar" ] && ! gunzip gcc-6.2.0.tar.gz; then
		unlink gcc-6.2.0.tar.gz
		cmd wget https://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.gz
		cmd gunzip gcc-6.2.0.tar.gz
	fi
	cmd tar -xf gcc-6.2.0.tar
fi

if ! [ -f "$gcc_src_folder/.kos-patched" ]; then
	# Apply The KOS patch
	cmd patch -d "$gcc_src_folder" -p1 < "$binutils_folder/../apps/patches/gcc.patch"
	> $gcc_src_folder/.kos-patched
fi


cmd which -- $TARGET-as
mkdir "$binutils_folder/$gcc_build_folder"
cmd cd "$binutils_folder/$gcc_build_folder"

if ! [ -f "Makefile" ]; then
	cmd bash "$gcc_src_folder/configure" \
		--target=$TARGET \
		--prefix="$PREFIX" \
		--with-sysroot="$binutils_syshook" \
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
		--enable-__cxa_atexit
fi

# HINT: When something goes wrong here, try starting
#       over by clearing "build-gcc-i686-kos"
cmd make -j 8 all-gcc
cmd make -j 8 all-target-libgcc
cmd make -j 8 install-gcc
cmd make -j 8 install-target-libgcc
cmd make -j 8 all-target-libstdc++-v3
cmd make -j 8 install-target-libstdc++-v3

# Create the symlink that's used for wrapping libstdc++ headers.
mv "$PREFIX/i686-kos/include/c++"  "$PREFIX/i686-kos/include-c++"
ln -s "../../binutils/$binutils_build_folder/i686-kos/include-c++/6.2.0"  "$binutils_folder/../include/c++/6.2"
ln -s "6.2" "$binutils_folder/../include/c++/6"
ln -s "6.2" "$binutils_folder/../include/c++/current"

# Copy libstdc++ files into KOS's lib folder.
cp "$PREFIX/i686-kos/lib/libstdc++.a"         "$binutils_folder/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libstdc++.la"        "$binutils_folder/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libstdc++.a-gdb.py"  "$binutils_folder/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libsupc++.a"         "$binutils_folder/../bin/libs/"
cp "$PREFIX/i686-kos/lib/libsupc++.la"        "$binutils_folder/../bin/libs/"


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
cd "$binutils_folder"
bin/deemon "fix_stdlibcxx_relations.dee"

