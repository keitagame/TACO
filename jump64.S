.section .text.boot
.code32
.global start
start:
    # 1. ページテーブルのベースをCR3にロード
    movl $page_table_l4, %eax     
    movl %eax, %cr3

    # 2. CR4レジスタでPAE（物理アドレス拡張）を有効化
    movl %cr4, %eax
    orl $(1 << 5), %eax           
    movl %eax, %cr4

    # 3. IA32_EFER MSRでロングモード（LME）を有効化
    movl $0xC0000080, %ecx        
    rdmsr                         
    orl $(1 << 8), %eax           
    wrmsr                         

    # 4. CR0レジスタでページング（PG）を有効化
    movl %cr0, %eax
    orl $(1 << 31), %eax          
    movl %eax, %cr0

    # 5. 64ビット記述子を含むGDTをロード
    lgdt gdt_pointer              

    # 6. ロングジャンプで完全に64ビット環境へ遷移
    ljmp $0x08, $entry_64bit

.code64
entry_64bit:
    # データセグメントレジスタを0で初期化
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    # 64ビット用のスタックポインタ（RSP）を設定
    leaq stack_top(%rip), %rsp

    # Cの関数に渡す引数をレジスタにセット（System V ABI）
    movq $0x36D32896, %rdi  
    movq $0x00000000, %rsi  

    # 外部のC関数（kernel_main）を呼び出す
    leaq kernel_main(%rip), %rax
    call *%rax

_loop:
    hlt
    jmp _loop

# =============================================================================
# データ定義セクション
# =============================================================================
.section .data.boot
.align 4096

.global page_table_l4
page_table_l4:
    .quad page_table_pdpt + 0x003  # 0x00000000_00000000用（低位マッピング）
    .fill 510, 8, 0                
    .quad page_table_pdpt + 0x003  # 0xFFFFFFFF_80000000用（高位マッピング）

page_table_pdpt:
    .quad page_table_pd + 0x003    
    .fill 511, 8, 0

page_table_pd:
    .quad 0x00000000 + 0x083      # 0MB ~ 2MB (Huge Page)
    .quad 0x00200000 + 0x083      # 2MB ~ 4MB
    .quad 0x00400000 + 0x083      # 4MB ~ 6MB
    .fill 509, 8, 0               

.align 8
gdt_start:
    .quad 0x0000000000000000       # ヌル記述子
    .quad 0x00209A0000000000       # 0x08: 64ビット コード
    .quad 0x0000920000000000       # 0x10: 64ビット データ
gdt_end:

.global gdt_pointer
gdt_pointer:
    .word gdt_end - gdt_start - 1  
    .quad gdt_start                # as --64 を使うため、ここも安全に .quad で定義可能

# =============================================================================
# セキュリティ警告を消去するための特殊セクション (必須)
# =============================================================================
.section .note.GNU-stack,"",@progbits

.section .bss
.align 16
stack_bottom:
    .skip 16384             
stack_top:
