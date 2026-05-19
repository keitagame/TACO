#!/bin/sh
gcc -m64 -c boot.S -o boot.o
gcc -m64 -c jump64.S -o jump64.o
gcc -m64 -c kernel.c -o entry.o
ld -m elf_x86_64 -T linker.ld -o kernel.elf boot.o jump64.o entry.o
qemu-system-x86_64 -kernel kernel.elf -nographic
