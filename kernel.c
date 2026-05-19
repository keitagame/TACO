#include <stdint.h>


// メモリマップの各エントリ構造（GRUB仕様）
struct multiboot_mmap_entry {
    uint32_t size;         // このエントリ自体のサイズ（このフィールド自体は含まない）
    uint64_t addr;         // 領域の開始物理アドレス
    uint64_t len;          // 領域のバイトサイズ
    uint32_t type;         // メモリタイプ (1=Usable, 2=Reserved, etc.)
} __attribute__((packed));
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
    uint32_t mmap_length;  // メモリマップ全体のバイトサイズ
    uint32_t mmap_addr;    // メモリマップの先頭物理アドレス
} __attribute__((packed));
// アライメントによる隙間を防ぐため packed を指定
struct e820_entry {
    uint64_t base_addr;    // 領域の開始物理アドレス
    uint64_t length;       // 領域のバイトサイズ
    uint32_t type;         // 領域の属性タイプ
    uint32_t extended;     // ACPI 3.0 拡張属性（省略される場合あり）
} __attribute__((packed));
#define MULTIBOOT_FLAG_MMAP (1 << 6) // メモリマップ有効フラグ

void parse_multiboot_mmap(struct multiboot_info *mbi) {
    // 1. GRUBからメモリマップが正しく渡されているかチェック
    if (!(mbi->flags & MULTIBOOT_FLAG_MMAP)) {
        // メモリマップが利用できない場合の致命的エラー処理
        print_serial("NO MEM\n");
        return;
    }
    print_serial("HELLO MEM\n");
    // 2. 開始アドレスと終了アドレス（限界点）を計算
    uintptr_t mmap_start = (uintptr_t)mbi->mmap_addr;
    uintptr_t mmap_end = mmap_start + mbi->mmap_length;

    // 3. ポインタを進めながらループ走査
    struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry *)mmap_start;

    while ((uintptr_t)entry < mmap_end) {
        print_serial("LOOP\n");
        // エントリの情報を取得
        uint64_t base = entry->addr;
        uint64_t length = entry->len;
        uint32_t type = entry->type;

        // タイプ別の処理
        switch (type) {
            case 1: // Usable RAM
                print_serial("USABLE RAM\n");
                // TODO: 物理メモリアロケータ（Page Frame Allocator）へ利用可能領域として登録
                break;
            case 2: // Reserved
                // システム予約領域（触ってはダメな場所）
                print_serial("RESERVED\n");
                break;
            case 3: // ACPI Reclaimable
            case 4: // ACPI NVS
                print_serial("HOGO SIRO\n");
                // ACPI関連領域として保護
                break;
            default:
                // 未定義、または不良メモリ領域
                print_serial("GOMI MEM\n");
                break;
        }

        // 【最重要】次のエントリへポインタを進める
        // entry->size に格納されているバイト数 ＋ sizeフィールド自身の4バイト
        uintptr_t next_addr = (uintptr_t)entry + sizeof(entry->size) + entry->size;
        entry = (struct multiboot_mmap_entry *)next_addr;
    }
}

// I/Oポートに1バイト書き込む関数
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// I/Oポートから1バイト読み込む関数
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#define COM1 0x3f8

// シリアルポート(COM1)の初期化
void init_serial() {
    outb(COM1 + 1, 0x00);    // 割り込みを無効化
    outb(COM1 + 3, 0x80);    // DLAB (分周器ラッチアクセス) を有効化
    outb(COM1 + 0, 0x03);    // ボーレートの設定 (低バイト): 38400 baud
    outb(COM1 + 1, 0x00);    // ボーレートの設定 (高バイト)
    outb(COM1 + 3, 0x03);    // 8ビット、パリティなし、1ストップビット (DLABクリア)
    outb(COM1 + 2, 0xC7);    // FIFOを有効化、クリア、トリガレベル14バイト
    outb(COM1 + 4, 0x0B);    // IRQを有効化、RTS/DSRをセット
}

// 送信バッファが空になるのを待つ
int is_transmit_empty() {
    return inb(COM1 + 5) & 0x20;
}

// シリアルポートに1文字出力
void write_serial(char a) {
    while (is_transmit_empty() == 0);
    outb(COM1, a);
}

// シリアルポートに文字列を出力
void print_serial(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        write_serial(str[i]);
    }
}

void kernel_main(unsigned long magic, struct multiboot_info *mbi) {
    // 1. シリアルポートの初期化
    init_serial();
    print_serial("Booting Kernel...\n");

    // 2. VGA画面への出力
    volatile char *vga = (volatile char *)0xB8000;
    vga[0] = '6';
    vga[1] = 0x0F;
    vga[2] = '4';
    vga[3] = 0x0F;

    // 3. シリアルポートへの出力
    print_serial("HELLO KERNEL\n");
    parse_multiboot_mmap(mbi);
    while (1) {
        __asm__ volatile("hlt");
    }
}
