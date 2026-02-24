#!/bin/bash

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== Building user space ==="
bash "$ROOT_DIR/user-space/build-user.sh"

echo "=== Building kernel ==="
bash "$ROOT_DIR/kernel-space/build-kernel.sh"

echo "=== Launching QEMU ==="
qemu-system-x86_64 -drive format=raw,file="$ROOT_DIR/kernel-space/build/os-image.bin"
