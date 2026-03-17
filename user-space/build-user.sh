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

# build shell binary
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/shell.c" -o "$SCRIPT_DIR/build/shell.o"
$LD -m elf_i386 -Ttext=0x40000000 \
    "$SCRIPT_DIR/build/syscalls.o" \
    "$SCRIPT_DIR/build/lib.o" \
    "$SCRIPT_DIR/build/shell.o" \
    -o "$SCRIPT_DIR/build/shell.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/shell.elf" "$SCRIPT_DIR/build/shell.bin"

# build bf binary
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/bf.c" -o "$SCRIPT_DIR/build/bf.o"
$LD -m elf_i386 -Ttext=0x40000000 \
    "$SCRIPT_DIR/build/syscalls.o" \
    "$SCRIPT_DIR/build/lib.o" \
    "$SCRIPT_DIR/build/bf.o" \
    -o "$SCRIPT_DIR/build/bf.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/bf.elf" "$SCRIPT_DIR/build/bf.bin"

# build vim binary
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/vim.c" -o "$SCRIPT_DIR/build/vim.o"
$LD -m elf_i386 -Ttext=0x40000000 \
    "$SCRIPT_DIR/build/syscalls.o" \
    "$SCRIPT_DIR/build/lib.o" \
    "$SCRIPT_DIR/build/vim.o" \
    -o "$SCRIPT_DIR/build/vim.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/vim.elf" "$SCRIPT_DIR/build/vim.bin"

# build tetris binary
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -c "$SCRIPT_DIR/programs/tetris.c" -o "$SCRIPT_DIR/build/tetris.o"
$LD -m elf_i386 -Ttext=0x40000000 \
    "$SCRIPT_DIR/build/syscalls.o" \
    "$SCRIPT_DIR/build/lib.o" \
    "$SCRIPT_DIR/build/tetris.o" \
    -o "$SCRIPT_DIR/build/tetris.elf"
$OBJCOPY -O binary "$SCRIPT_DIR/build/tetris.elf" "$SCRIPT_DIR/build/tetris.bin"
# embed both as kernel objects
cd "$SCRIPT_DIR/build"
$OBJCOPY -I binary -O elf32-i386 -B i386 main.bin main_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 idle.bin idle_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 shell.bin shell_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 bf.bin bf.o
$OBJCOPY -I binary -O elf32-i386 -B i386 vim.bin vim.o
$OBJCOPY -I binary -O elf32-i386 -B i386 tetris.bin tetris.o
cd "$SCRIPT_DIR"

echo "User space build complete!"

