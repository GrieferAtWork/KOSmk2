#!/bin/bash

binutils_folder=$(dirname $(readlink -f "$0"))
deemon_exe="deemon.102.102"
if ! [ -z "$CONFIG_DEEMON" ]; then
	deemon_exe="$CONFIG_DEEMON"
fi

cd "$binutils_folder/src"

cmd() {
	$* || {
		local error=$?
		echo "Command failed '$*' -> '$error'"
		exit $error
	}
}

mkdir -p deemon
cmd cd deemon

if ! [ -f "make.sh" ]; then
	# Fetch a copy of deemon known to work
	cmd git init
	cmd git remote add origin https://github.com/GrieferAtWork/deemon.git
	cmd git fetch origin 4b79f9c4dec76292f40f072c3a601683cfc4bd58
	cmd git reset --hard FETCH_HEAD
fi

if ! [ -f "$deemon_exe" ]; then
	# Build it as a static binary
	cmd bash make.sh --static make
fi
