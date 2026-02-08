#!/bin/bash

# Use cross-compiler tools
CC=i686-elf-gcc
LD=i686-elf-ld

echo "Creating precompiled header..."
$CC -ffreestanding -fno-pic -c pch.h -o pch.h.gch

echo "Assembling files..."
nasm -f elf32 cpu/interrupt.asm -o interrupt.o
nasm -f elf32 boot/kernel-entry.asm -o kernel-entry.o
nasm -f bin boot/kernel-boot.asm -o kernel-boot.bin

echo "Compiling C files..."
$CC -ffreestanding -fno-pic -include pch.h -c kernel/kernel.c -o kernel.o
$CC -ffreestanding -fno-pic -include pch.h -c kernel/kmalloc.c -o kmalloc.o
$CC -ffreestanding -fno-pic -include pch.h -c drivers/screen.c -o screen.o
$CC -ffreestanding -fno-pic -include pch.h -c drivers/keyboard.c -o keyboard.o
$CC -ffreestanding -fno-pic -include pch.h -c drivers/timer.c -o timer.o
$CC -ffreestanding -fno-pic -include pch.h -c cpu/idt.c -o idt.o
$CC -ffreestanding -fno-pic -include pch.h -c lib/string.c -o string.o
$CC -ffreestanding -fno-pic -include pch.h -c lib/math.c -o math.o
$CC -ffreestanding -fno-pic -include pch.h -c lib/shell.c -o shell.o
$CC -ffreestanding -fno-pic -include pch.h -c lib/ramfs.c -o ramfs.o

echo "Linking kernel..."
$LD -T linker.ld -o kernel.bin kernel-entry.o kernel.o kmalloc.o screen.o \
    keyboard.o timer.o idt.o interrupt.o ramfs.o math.o string.o shell.o \
    --oformat binary

echo "Creating OS image..."
dd if=/dev/zero of=os-image.bin bs=1M count=10
dd if=kernel-boot.bin of=os-image.bin conv=notrunc
dd if=kernel.bin of=os-image.bin seek=1 conv=notrunc

echo "Moving artifacts to build directory..."
mkdir -p build
mv *.o build/ 2>/dev/null || true
mv *.bin build/ 2>/dev/null || true
mv *.gch build/ 2>/dev/null || true

echo "Build complete!"
qemu-system-x86_64 -drive format=raw,file=build/os-image.bin
