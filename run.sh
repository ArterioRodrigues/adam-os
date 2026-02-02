nasm -f elf32 interrupt.asm -o interrupt.o
nasm -f elf32 kernel-entry.asm -o kernel-entry.o
nasm -f bin kernel-boot.asm -o kernel-boot.bin

gcc -m32 -ffreestanding -fno-pic -c kernel.c -o kernel.o

ld -m elf_i386 -T linker.ld -o kernel.bin kernel-entry.o kernel.o interrupt.o --oformat binary

dd if=/dev/zero of=os-image.bin bs=1M count=10
dd if=kernel-boot.bin of=os-image.bin conv=notrunc
dd if=kernel.bin of=os-image.bin seek=1 conv=notrunc

qemu-system-x86_64 -drive format=raw,file=os-image.bin

mv *.o build/
mv *.bin build/
