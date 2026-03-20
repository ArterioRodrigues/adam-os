#!/bin/bash
CC=i686-elf-gcc
LD=i686-elf-ld
NASM=nasm
OBJCOPY=i686-elf-objcopy

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SHARED_DIR="$SCRIPT_DIR/../shared"
mkdir -p "$SCRIPT_DIR/build"

# assemble syscalls once - shared by all programs
$NASM -f elf32 "$SCRIPT_DIR/lib/syscalls.asm" -o "$SCRIPT_DIR/build/syscalls.o"

# compile lib once - linked into each program
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -I"$SHARED_DIR" -I"$SCRIPT_DIR/lib" \
    -c "$SCRIPT_DIR/programs/lib.c" -o "$SCRIPT_DIR/build/lib.o"

# compile shared library files once - linked into each program
$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -I"$SHARED_DIR" -I"$SCRIPT_DIR/lib" \
    -c "$SHARED_DIR/string.c" -o "$SCRIPT_DIR/build/string.o"

$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -I"$SHARED_DIR" -I"$SCRIPT_DIR/lib" \
    -c "$SHARED_DIR/mem.c" -o "$SCRIPT_DIR/build/mem.o"

$CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
    -I"$SHARED_DIR" -I"$SCRIPT_DIR/lib" \
    -c "$SHARED_DIR/math.c" -o "$SCRIPT_DIR/build/math.o"

build_minimal() {
    local name="$1"
    $CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
        -I"$SHARED_DIR" -I"$SCRIPT_DIR/lib" \
        -c "$SCRIPT_DIR/programs/${name}.c" -o "$SCRIPT_DIR/build/${name}.o"
    $LD -m elf_i386 -Ttext=0x40000000 \
        "$SCRIPT_DIR/build/syscalls.o" \
        "$SCRIPT_DIR/build/${name}.o" \
        -o "$SCRIPT_DIR/build/${name}.elf"
    $OBJCOPY -O binary "$SCRIPT_DIR/build/${name}.elf" "$SCRIPT_DIR/build/${name}.bin"
}

build_program() {
    local name="$1"
    local src="$SCRIPT_DIR/programs/${name}.c"

    $CC -m32 -ffreestanding -fno-pic -nostdlib -nostdinc \
        -I"$SHARED_DIR" -I"$SCRIPT_DIR/lib" \
        -c "$src" -o "$SCRIPT_DIR/build/${name}.o"

    $LD -m elf_i386 -Ttext=0x40000000 \
        "$SCRIPT_DIR/build/syscalls.o" \
        "$SCRIPT_DIR/build/lib.o" \
        "$SCRIPT_DIR/build/string.o" \
        "$SCRIPT_DIR/build/mem.o" \
        "$SCRIPT_DIR/build/math.o" \
        "$SCRIPT_DIR/build/${name}.o" \
        -o "$SCRIPT_DIR/build/${name}.elf"

    $OBJCOPY -O binary "$SCRIPT_DIR/build/${name}.elf" "$SCRIPT_DIR/build/${name}.bin"
}

# build all user programs
build_minimal main
build_minimal idle

build_program shell
build_program bf
build_program vim
build_program tetris
build_program calculator

# embed all as kernel objects
cd "$SCRIPT_DIR/build"
$OBJCOPY -I binary -O elf32-i386 -B i386 main.bin main_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 idle.bin idle_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 shell.bin shell_bin.o
$OBJCOPY -I binary -O elf32-i386 -B i386 bf.bin bf.o
$OBJCOPY -I binary -O elf32-i386 -B i386 vim.bin vim.o
$OBJCOPY -I binary -O elf32-i386 -B i386 tetris.bin tetris.o
$OBJCOPY -I binary -O elf32-i386 -B i386 calculator.bin calculator.o
cd "$SCRIPT_DIR"

echo "User space build complete!"
