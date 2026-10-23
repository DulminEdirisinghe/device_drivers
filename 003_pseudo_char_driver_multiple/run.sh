#!/bin/bash

set -e

KERNEL_BUILD="/home/ddul6978/Documents/ldd/kernel/linux-build"
INITRAMFS_DIR="../../initramfs"

echo "==> Cleaning module..."
make -C "$KERNEL_BUILD" M="$PWD" clean

echo "==> Building module..."
make -C "$KERNEL_BUILD" M="$PWD" modules

echo "==> Copying module..."
cp main.ko "$INITRAMFS_DIR"

echo "==> Building initramfs..."
cd "$INITRAMFS_DIR"
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.img

echo "==> Starting QEMU..."
cd ..
qemu-system-x86_64 \
    -kernel linux-build/arch/x86/boot/bzImage \
    -initrd initramfs.img \
    -append "console=ttyS0" \
    -nographic