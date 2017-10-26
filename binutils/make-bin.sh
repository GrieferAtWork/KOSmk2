#!/bin/bash

ROOT=$(dirname $(readlink -f "$0"))
cd "$ROOT"
mkdir "bin"

BIN="build-binutils-i686-kos/bin"

addbin() {
	ln -s "../${BIN}/i686-kos-$1"  "bin/$1"
	#ln -s "${ROOT}/${BIN}/i686-kos-$1"  "/usr/local/bin/i686-kos-$1"
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
