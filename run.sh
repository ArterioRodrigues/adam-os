#!/bin/bash

echo "Assembling files..."
nasm -f elf32 cpu/interrupt.asm -o interrupt.o
nasm -f elf32 boot/kernel-entry.asm -o kernel-entry.o
nasm -f bin boot/kernel-boot.asm -o kernel-boot.bin

echo "Compiling C files..."
gcc -m32 -ffreestanding -fno-pic -c kernel/kernel.c -o kernel.o
gcc -m32 -ffreestanding -fno-pic -c drivers/screen.c -o screen.o
gcc -m32 -ffreestanding -fno-pic -c drivers/keyboard.c -o keyboard.o
gcc -m32 -ffreestanding -fno-pic -c cpu/idt.c -o idt.o

echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o kernel.bin kernel-entry.o kernel.o screen.o keyboard.o idt.o interrupt.o --oformat binary

echo "Creating OS image..."
dd if=/dev/zero of=os-image.bin bs=1M count=10
dd if=kernel-boot.bin of=os-image.bin conv=notrunc
dd if=kernel.bin of=os-image.bin seek=1 conv=notrunc

echo "Moving artifacts to build directory..."
mkdir -p build
mv *.o build/ 
mv *.bin build/ 

echo "Build complete!"
qemu-system-x86_64 -drive format=raw,file=build/os-image.bin
