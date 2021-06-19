#!/bin/sh
set -e
. ./iso.sh

DEBUG_FILE="debug-logs.txt"
FS_IMG="MSX"

cp fat16_disk_images/$FS_IMG.img fat16_disk_images/$FS_IMG-cp.img
qemu-system-$(./target-triplet-to-arch.sh $HOST) -monitor telnet:127.0.0.1:1234,server,nowait  -serial stdio -no-reboot -drive file=fat16_disk_images/$FS_IMG-cp.img,media=disk,index=1,if=ide,format=raw -drive file=aros.iso,format=raw | tee $DEBUG_FILE
#-d int
