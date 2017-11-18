#!/bin/bash

export TARGET=x86_64-kos
#export TARGET=i686-kos
#export TARGET=arm-none-eabi
if (($# >= 1)); then
	export TARGET=$1
fi

binutils_folder=$(dirname $(readlink -f "$0"))
binutils_build_folder="$binutils_folder/build-binutils-$TARGET"
binutils_src_folder="$binutils_folder/src/binutils-2.27"
binutils_syshook="$binutils_folder/root"
export PREFIX="$binutils_build_folder"
export PATH="$PREFIX/bin:$PATH"

cmd() {
	$* || {
		local error=$?
		echo "Command failed '$*' -> '$error'"
		exit $error
	}
}

cmd cd "$binutils_folder"
mkdir -p src
mkdir -p root/usr
unlink root/usr/include
unlink root/usr/lib
ln -s "../../../include"  root/usr/include
ln -s "../../../bin/libs" root/usr/lib
cmd cd src

if ! [ -f "$binutils_src_folder/configure" ]; then
	if ! [ -f "binutils-2.27.tar" ] && ! gunzip binutils-2.27.tar.gz; then
		unlink binutils-2.27.tar.gz
		cmd wget https://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.gz
		cmd gunzip binutils-2.27.tar.gz
	fi
	cmd tar -xf binutils-2.27.tar
fi

if ! [ -f "$binutils_src_folder/.kos-patched" ]; then
	# Apply The KOS patch
	cmd patch -d "$binutils_src_folder" -p1 < "$binutils_folder/../apps/patches/binutils.patch"
	> $binutils_src_folder/.kos-patched
fi

mkdir -p "$PREFIX"
cmd cd "$PREFIX"

if ! [ -f "Makefile" ]; then
	options=""
	if [[ "$TARGET" == "x86_64"* ]]; then
		options="$options --enable-64-bit-bfd"
	fi
	cmd bash "$binutils_src_folder/configure" \
		--target=$TARGET \
		$options \
		--prefix="$PREFIX" \
		--with-sysroot="${binutils_syshook}" \
		--with-headers="${binutils_syshook}/usr/include" \
		--disable-nls \
		--disable-werror \
		--enable-multilib
fi

cmd make -j 8
cmd make -j 8 install

# Prevent redundancy: Use a symlink to integrate KOS system headers into binutils!
rm -rf "$TARGET/sys-include"
ln -s "../../include" "$TARGET/sys-include"


