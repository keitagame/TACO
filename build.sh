#!/bin/sh
gcc -m32 -c boot.S -o boot.o
gcc -m32 -c jump64.S -o jump64.o
ld -m elf_i386 -T linker.ld -o kernel.elf boot.o jump64.o