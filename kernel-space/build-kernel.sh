#!/bin/bash

CC=i686-elf-gcc
LD=i686-elf-ld

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
USER_BUILD="$SCRIPT_DIR/../build"
SHARED_DIR="$SCRIPT_DIR/../shared"

cd "$SCRIPT_DIR"

echo "Creating precompiled header..."
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -c pch.h -o pch.h.gch

echo "Assembling files..."
nasm -f elf32 cpu/interrupts.asm -o interrupts.o
nasm -f elf32 cpu/exceptions.asm -o exceptions.o
nasm -f elf32 cpu/gdt.asm -o gdt.o
nasm -f elf32 boot/kernel-entry.asm -o kernel-entry.o
nasm -f elf32 kernel/page-table.asm -o page-table.o
nasm -f elf32 kernel/process-control-block.asm -o process-control-block.o
nasm -f bin boot/kernel-boot.asm -o kernel-boot.bin

echo "Compiling C files..."
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/kernel.c -o kernel.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/kmalloc.c -o kmalloc.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/page-table.c -o page-tablec.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/frame.c -o frame.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/process-control-block.c -o process-control-blockc.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/scheduler.c -o scheduler.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/window.c -o window.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/stdin.c -o stdin.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c kernel/event.c -o event.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c drivers/ata-disk.c -o ata-disk.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c drivers/keyboard.c -o keyboard.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c drivers/mouse.c -o mouse.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c drivers/timer.c -o timer.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c drivers/vga-graphics.c -o vga-graphics.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c drivers/terminal.c -o terminal.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c cpu/idt.c -o idt.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c cpu/exceptions.c -o exceptionsc.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c cpu/gdt.c -o gdtc.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c cpu/syscall.c -o syscall.o

# shared library files (from shared/)
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c "$SHARED_DIR/string.c" -o string.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c "$SHARED_DIR/math.c" -o math.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c "$SHARED_DIR/mem.c" -o mem.o

# kernel-only lib files (stay in kernel-space/lib/)
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c lib/fat16.c -o fat16.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c lib/status-bar.c -o status-bar.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c lib/font.c -o font.o
$CC -ffreestanding -fno-pic -I"$SHARED_DIR" -include pch.h -c lib/desktop.c -o desktop.o

# collect all embedded user program objects
USER_BINS=$(ls "$USER_BUILD"/*_bin.o 2>/dev/null | tr '\n' ' ')
echo "Including user binaries: $USER_BINS"

echo "Linking kernel..."
$LD -T linker.ld -o kernel.bin \
    kernel-entry.o process-control-blockc.o scheduler.o window.o process-control-block.o \
    kernel.o kmalloc.o page-tablec.o frame.o syscall.o \
    mem.o keyboard.o mouse.o timer.o vga-graphics.o terminal.o ata-disk.o idt.o interrupts.o gdt.o gdtc.o \
    exceptions.o exceptionsc.o fat16.o math.o string.o stdin.o event.o\
    page-table.o status-bar.o font.o desktop.o\
    $USER_BINS \
    --oformat binary

# build fat storage
truncate -s 9M fat16.bin
mkfs.fat -F 16 fat16.bin

mcopy -i fat16.bin $USER_BUILD/bf.bin ::BF
mcopy -i fat16.bin $USER_BUILD/shell.bin ::SHELL
mcopy -i fat16.bin $USER_BUILD/idle.bin ::IDLE
mcopy -i fat16.bin $USER_BUILD/main.bf ::MAIN.BF
mcopy -i fat16.bin $USER_BUILD/tetris.bin ::TETRIS
mcopy -i fat16.bin $USER_BUILD/calculator.bin ::CALC
mcopy -i fat16.bin $USER_BUILD/obj_viewer.bin ::OBJ_VIEW
mcopy -i fat16.bin $USER_BUILD/cube.obj ::CUBE.OBJ
mcopy -i fat16.bin $USER_BUILD/diamond.obj ::DIAMOND.OBJ
mcopy -i fat16.bin $USER_BUILD/pyramid.obj ::PYRAMID.OBJ
mcopy -i fat16.bin $USER_BUILD/octahedron.obj ::OCTAHED.OBJ
mcopy -i fat16.bin $USER_BUILD/tetrahedron.obj ::TETRAHE.OBJ
mcopy -i fat16.bin $USER_BUILD/sphere.obj ::SPHERE.OBJ

echo "Creating OS image..."
dd if=/dev/zero of=os-image.bin bs=1M count=10
dd if=kernel-boot.bin of=os-image.bin conv=notrunc
dd if=kernel.bin of=os-image.bin seek=1 conv=notrunc
dd if=fat16.bin of=os-image.bin seek=2048 conv=notrunc

echo "Moving artifacts to build..."
mkdir -p ../build
mv *.o ../build/ 2>/dev/null || true
mv *.bin ../build/ 2>/dev/null || true
mv *.gch ../build/ 2>/dev/null || true

echo "Kernel build complete!"
