#!/bin/bash
CC=i686-elf-gcc
LD=i686-elf-ld
NASM=nasm
OBJCOPY=i686-elf-objcopy

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$SCRIPT_DIR/build"

# assemble syscalls once - shared by all programs
$NASM -f elf32 "$SCRIPT_DIR/lib/syscalls.asm" -o "$SCRIPT_DIR/build/syscalls.o"

# compile lib once - linked into each program
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/lib.c" -o "$SCRIPT_DIR/build/lib.o"

# build main binary
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/main.c" -o "$SCRIPT_DIR/build/main.o"
$LD -m elf_i386 -Ttext=0x40000000 \
    "$SCRIPT_DIR/build/syscalls.o" \
    "$SCRIPT_DIR/build/lib.o" \
    "$SCRIPT_DIR/build/main.o" \
    -o "$SCRIPT_DIR/build/main.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/main.elf" "$SCRIPT_DIR/build/main.bin"

# build idle binary
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/idle.c" -o "$SCRIPT_DIR/build/idle.o"
$LD -m elf_i386 -Ttext=0x40000000 \
    "$SCRIPT_DIR/build/syscalls.o" \
    "$SCRIPT_DIR/build/lib.o" \
    "$SCRIPT_DIR/build/idle.o" \
    -o "$SCRIPT_DIR/build/idle.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/idle.elf" "$SCRIPT_DIR/build/idle.bin"

# embed both as kernel objects
cd "$SCRIPT_DIR/build"
$OBJCOPY -I binary -O elf32-i386 -B i386 main.bin main_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 idle.bin idle_bin.o
cd "$SCRIPT_DIR"

echo "User space build complete!"
