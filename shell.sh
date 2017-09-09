#!/bin/bash

export ROOT=$(dirname $(readlink -f "$0"))
export TARGET="i686-kos"
export PATH="$ROOT/binutils/bin:$PATH"

if [ $# == 0 ]; then
	bash -i
else
	$*
fi
