#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/aros.kernel isodir/boot/aros.kernel
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
set default=0 # Set the default menu entry
menuentry "aros" {
	multiboot /boot/aros.kernel
}
EOF
grub-mkrescue -o aros.iso isodir
