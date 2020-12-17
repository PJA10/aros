#!/bin/sh
set -e
. ./iso.sh

DEBUG_FILE="debug-logs.txt"

qemu-system-$(./target-triplet-to-arch.sh $HOST) -serial stdio -cdrom aros.iso | tee $DEBUG_FILE
