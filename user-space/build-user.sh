#!/bin/bash
CC=i686-elf-gcc
LD=i686-elf-ld
NASM=nasm
OBJCOPY=i686-elf-objcopy

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "$SCRIPT_DIR/build"

$NASM -f elf32 "$SCRIPT_DIR/lib/syscalls.asm" -o "$SCRIPT_DIR/build/syscalls.o"
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc -c "$SCRIPT_DIR/programs/main.c" -o "$SCRIPT_DIR/build/main.o"
$LD -m elf_i386 -Ttext=0x40000000 "$SCRIPT_DIR/build/syscalls.o" "$SCRIPT_DIR/build/main.o" -o "$SCRIPT_DIR/build/main.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/main.elf" "$SCRIPT_DIR/build/main.bin"

cd "$SCRIPT_DIR/build"
$OBJCOPY -I binary -O elf32-i386 -B i386 main.bin main_bin.o
cd "$SCRIPT_DIR"

echo "User space build complete!"
