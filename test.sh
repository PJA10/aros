#!/bin/sh
set -e

. ./qemu.sh > /dev/null 2>&1

DEBUG_OUTPUT=$(cat $DEBUG_FILE)

cat $DEBUG_FILE | grep "debug:" | awk '{for (i=2; i<NF; i++) printf $i " "; print $NF}'

if ! (echo $DEBUG_OUTPUT | grep -q "boot - OK")
        then
                echo "error in boot"
fi

if ! (echo $DEBUG_OUTPUT | grep -q "serial debug - OK")
	then
		echo "error in serial port setup"
fi

