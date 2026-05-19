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

void kernel_main(unsigned long magic, unsigned long addr) {
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
    print_serial("VGA displayed: 64\n");

    while (1) {
        __asm__ volatile("hlt");
    }
}
