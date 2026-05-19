.section .text.boot
.code32
.global start32

start32:
    # GDTロード（32bit用）
    lgdt gdt_pointer

    # セグメントレジスタ初期化
    mov $0x10, %ax   # データセグメント
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # スタック設定
    mov $stack_top, %esp

    # 引数設定（cdecl）
    push $0x00000000
    push $0x36D32896
    mov $0x3F8, %dx
    mov $'A', %al
    out %al, %dx
    # C関数呼び出し
    call kernel_main

_loop:
    hlt
    jmp _loop
.section .data.boot
.align 8

gdt_start:
    .quad 0x0000000000000000       # null
    .quad 0x00CF9A000000FFFF       # 0x08: 32bit code
    .quad 0x00CF92000000FFFF       # 0x10: 32bit data
gdt_end:

.global gdt_pointer
gdt_pointer:
    .word gdt_end - gdt_start - 1
    .long gdt_start
.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top: