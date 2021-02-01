#!/bin/sh
set -e
. ./iso.sh

DEBUG_FILE="debug-logs.txt"

qemu-system-$(./target-triplet-to-arch.sh $HOST) -monitor telnet:127.0.0.1:1234,server,nowait -vnc :1 -serial stdio -no-reboot -d int -cdrom aros.iso | tee $DEBUG_FILE
#-d int
