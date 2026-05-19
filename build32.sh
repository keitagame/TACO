#!/bin/sh
gcc -m32 -ffreestanding -fno-pie -no-pie -fno-stack-protector -O0 -c boot.S -o boot.o
gcc -m32 -ffreestanding -fno-pie -no-pie -fno-stack-protector -O0 -c jump32.S -o jump32.o
gcc -m32 -ffreestanding -fno-pie -no-pie -fno-stack-protector -O0 -c kernel.c -o entry.o
ld -m elf_i386 -T linker.ld -o kernel.elf boot.o jump32.o entry.o
cp kernel.elf iso/boot/kernel.elf
grub-mkrescue -o os.iso iso/
qemu-system-i386 -cdrom os.iso -boot d -nographic
#qemu-system-i386 -kernel kernel.elf -nographic -serial mon:stdio