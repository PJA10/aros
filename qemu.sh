#!/bin/sh
set -e
. ./iso.sh

DEBUG_FILE="debug-logs.txt"

qemu-system-$(./target-triplet-to-arch.sh $HOST) -vnc :1 -serial stdio -no-reboot -d int -cdrom aros.iso | tee $DEBUG_FILE
#-d int
