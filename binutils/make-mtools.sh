#!/bin/bash

binutils_folder=$(dirname $(readlink -f "$0"))
mtools_build_folder="$binutils_folder/build-mtools"
mtools_src_folder="$binutils_folder/src/mtools-4.0.18"

cmd() {
	$* || {
		local error=$?
		echo "Command failed '$*' -> '$error'"
		exit $error
	}
}

cmd cd "$binutils_folder"
mkdir -p src
cmd cd src

if ! [ -f "$mtools_src_folder/configure" ]; then
	if ! [ -f "mtools-4.0.18.tar" ] && ! gunzip mtools-4.0.18.tar.gz; then
		unlink mtools-4.0.18.tar.gz
		cmd wget http://ftp.gnu.org/gnu/mtools/mtools-4.0.18.tar.gz
		cmd gunzip mtools-4.0.18.tar.gz
	fi
	cmd tar -xf mtools-4.0.18.tar
fi

mkdir "$mtools_build_folder"
cmd cd "$mtools_build_folder"

if ! [ -f "Makefile" ]; then
	cmd bash "$mtools_src_folder/configure"
fi

cmd make -j 8



