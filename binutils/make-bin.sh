#!/bin/bash

ROOT=$(dirname $(readlink -f "$0"))
cd "$ROOT"
mkdir "bin"
export TARGET="x86_64-kos"
#export TARGET="i686-kos"

BIN="build-binutils-$TARGET/bin"

addbin() {
	unlink "bin/$1"
	ln -s "../${BIN}/$TARGET-$1"  "bin/$1"
	#unlink "/usr/local/bin/$TARGET-$1"
	#ln -s "${ROOT}/${BIN}/$TARGET-$1"  "/usr/local/bin/$TARGET-$1"
}

addbin "addr2line"
addbin "ar"
addbin "as"
addbin "c++"
addbin "c++filt"
addbin "cpp"
addbin "elfedit"
addbin "g++"
addbin "gcc-6.2.0"
addbin "gcc-ar"
addbin "gcc-nm"
addbin "gcc-ranlib"
addbin "gcc"
addbin "gcov-tool"
addbin "gcov"
addbin "gprof"
addbin "ld.bfd"
addbin "ld"
addbin "nm"
addbin "objcopy"
addbin "objdump"
addbin "ranlib"
addbin "readelf"
addbin "size"
addbin "strings"
addbin "strip"
