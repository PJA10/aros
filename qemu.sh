#!/bin/sh
set -e
. ./iso.sh

DEBUG_FILE="debug-logs.txt"
FS_IMG="MSX"

cp fat16_disk_images/$FS_IMG.img fat16_disk_images/$FS_IMG-cp.img
qemu-system-$(./target-triplet-to-arch.sh $HOST) -monitor telnet:127.0.0.1:6666,server,nowait -serial stdio -no-reboot -drive file=fat16_disk_images/$FS_IMG-cp.img,media=disk,index=1,if=ide,format=raw -drive file=aros.iso,format=raw $1| tee $DEBUG_FILE
#-d int
# -monitor telnet:127.0.0.1:1234,server,nowait
# -s //  Shorthand for -gdb tcp::1234, i.e. open a gdbserver on TCP port 1234
# -S // Do not start CPU at startup (you must type 'c' in the monitor).
# -nographic
