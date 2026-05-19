# boot.S (GAS形式)

.code32                         
.section .multiboot, "a"        
.align 4                       

# 定数の定義
.set MULTIBOOT_PAGE_ALIGN,    (1 << 0)
.set MULTIBOOT_MEMORY_INFO,   (1 << 1)
.set MULTIBOOT_HEADER_MAGIC,  0x1BADB002
.set MULTIBOOT_HEADER_FLAGS,  (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO)
.set MULTIBOOT_CHECKSUM,      -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)


.long MULTIBOOT_HEADER_MAGIC     
.long MULTIBOOT_HEADER_FLAGS     
.long MULTIBOOT_CHECKSUM       

# ―― ここから以下にカーネルのエントリポイント等を記述 ――
.section .text
.global _start
_start:
    cli
    jmp start
    
