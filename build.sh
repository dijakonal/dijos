#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Define variables for your project
BUILD_DIR="build"
SRC_DIR="src"

echo "=== Starting Build Process ==="

echo "Cleaning old build directory..."
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"

cd build
mkdir src
i686-elf-as ../boot.s -o src/boot.o
i686-elf-gcc -c ../kernel.c -o src/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T ../linker.ld -o src/dijos -ffreestanding -O2 -nostdlib src/boot.o src/kernel.o -lgcc
if grub-file --is-x86-multiboot src/dijos; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi
mkdir -p src/isodir/boot/grub
cp src/dijos src/isodir/boot/dijos
cp ../grub.cfg src/isodir/boot/grub/grub.cfg
grub-mkrescue -o dijos.iso src/isodir

echo "=== Build Completed Successfully! ==="
