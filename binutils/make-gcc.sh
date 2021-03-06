#!/bin/bash

export TARGET=x86_64-kos
#export TARGET=i686-kos
#export TARGET=arm-none-eabi
if (($# >= 1)); then
	export TARGET=$1
fi

gcc_build_folder="build-gcc-$TARGET"
binutils_build_folder="build-binutils-$TARGET"
binutils_folder=$(dirname $(readlink -f "$0"))
binutils_syshook="$binutils_folder/root"
gcc_src_folder="$binutils_folder/src/gcc-6.2.0"

export PREFIX="${binutils_folder}/${binutils_build_folder}"
#export PATH="$PREFIX/bin:$PATH"
export PATH="$PREFIX/bin:/bin:/usr/bin:/usr/local/bin"

cmd() {
	$* || {
		local error=$?
		echo "Command failed '$*' -> '$error'"
		exit $error
	}
}

cmd cd "$binutils_folder"
mkdir -p root/usr
unlink root/usr/include
unlink root/usr/lib
ln -s "../../../include"  root/usr/include
ln -s "../../../bin/libs" root/usr/lib

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
	options=""
	if [[ "$TARGET" == "x86_64"* ]]; then
		options="$options --with-cpu-32=i686"
		options="$options --with-arch-32=i686"
		options="$options --with-tune-32=i686"
		options="$options --with-cpu-64=x86-64"
		options="$options --with-arch-64=x86-64"
		options="$options --with-tune-64=x86-64"
		options="$options --enable-64-bit-bfd"
		options="$options --with-multilib-list=m32,m64,mx32"
		options="$options --with-tune=generic"
		options="$options --enable-multilib"
		options="$options --with-sysroot=$binutils_syshook"
	fi

	cmd bash "$gcc_src_folder/configure" \
		--target=$TARGET \
		--prefix="$PREFIX" \
		--with-gnu-ld \
		--with-gnu-as \
		--with-dwarf2 \
		$options \
		--enable-gnu-unique-object \
		--disable-vtable-verify \
		--enable-threads=single \
		--enable-targets=all \
		--enable-languages=c,c++ \
		--disable-nls \
		--enable-multiarch \
		--enable-initfini-array \
		--enable-__cxa_atexit
fi

# HINT: When something goes wrong here, try starting
#       over by clearing "build-gcc-i686-kos"
cmd make -j 8 all-gcc
cmd make -j 8 all-target-libgcc
cmd make -j 8 install-gcc
cmd make -j 8 install-target-libgcc

# Create the symlink that's used for wrapping libstdc++ headers.
mv "$PREFIX/$TARGET/include/c++"  "$PREFIX/$TARGET/include-c++"
ln -s "../../binutils/$binutils_build_folder/$TARGET/include-c++/6.2.0"  "$binutils_folder/../include/c++/6.2"
ln -s "6.2" "$binutils_folder/../include/c++/6"
ln -s "6.2" "$binutils_folder/../include/c++/current"

# Copy libstdc++ files into KOS's lib folder.
cp "$PREFIX/$TARGET/lib/libstdc++.a"         "$binutils_folder/../bin/libs/"
cp "$PREFIX/$TARGET/lib/libstdc++.la"        "$binutils_folder/../bin/libs/"
cp "$PREFIX/$TARGET/lib/libstdc++.a-gdb.py"  "$binutils_folder/../bin/libs/"
cp "$PREFIX/$TARGET/lib/libsupc++.a"         "$binutils_folder/../bin/libs/"
cp "$PREFIX/$TARGET/lib/libsupc++.la"        "$binutils_folder/../bin/libs/"


# Get rid of broken ~FiXiNcLuDe~ headers that we don't want
disable_fixinclude() {
	mv "$PREFIX/lib/gcc/$TARGET/6.2.0/include/$1" "$PREFIX/lib/gcc/$TARGET/6.2.0/include/$1.nope"
}
disable_fixinclude2() {
	mv "$PREFIX/lib/gcc/$TARGET/6.2.0/include-fixed/$1" "$PREFIX/lib/gcc/$TARGET/6.2.0/include-fixed/$1.nope"
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

cmd make -j 8 all-target-libstdc++-v3
cmd make -j 8 install-target-libstdc++-v3

# Switch back to the binutils root folder and fix stdlibc++ header files.
cd "$binutils_folder"
bin/deemon "fix_stdlibcxx_relations.dee"

