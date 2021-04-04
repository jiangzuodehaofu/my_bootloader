#include "s3c2440_soc.h"
#include "init.h"

void nand_init(void)
{
#define TACLS       0
#define TWRPH0      1
#define TWRPH1      0
    /* 设置Nand Flash的时序 */
    NFCONF |= (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1<<4);

    /* 使能Nand Flash控制器 */
    NFCONT = (1 << 4) | (1 << 1) | (1 << 0);
}

void nand_select(void)
{
    NFCONT &= ~(1 << 1);
}

void nand_deselect(void)
{
    NFCONT |= (1 << 1);
}

void nand_cmd(unsigned char cmd)
{
    volatile int i;
    NFCMD = cmd;
    for (i = 0; i < 10; i++);
}

void nand_addr(unsigned int addr)
{
    unsigned int col  = addr % 2048;
    unsigned int page = addr / 2048;
    volatile int i;

    NFADDR = col & 0xff;
    for (i = 0; i < 10; i++);
    NFADDR = (col >> 8) & 0xff;
    for (i = 0; i < 10; i++);
    
    NFADDR  = page & 0xff;
    for (i = 0; i < 10; i++);
    NFADDR  = (page >> 8) & 0xff;
    for (i = 0; i < 10; i++);
    NFADDR  = (page >> 16) & 0xff;
    for (i = 0; i < 10; i++);   
}


unsigned char nand_data(void)
{
    return NFDATA;
}

void nand_wait_ready(void)
{
    while(!(NFSTAT & (1<<0)));
}

void nand_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
    int i;
    int col;

    i = 0;
    col = addr & (2048 - 1);

    /* 1.选中片选 */
    nand_select();

    while (i < len) {
        /* 2.发出读命令：00H */
        nand_cmd(0x00);

        /* 3.发出地址 */
        nand_addr(addr);

        /* 4.发出读命令：30H */
        nand_cmd(0x30);

        /* 5.判断状态 */
        nand_wait_ready();

        /* 6.读数据 */
        for (; (col < 2048) && (i < len); col++) {
            buf[i] = nand_data();
            i++;
            addr++;
        }
        col = 0;
    }

    /* 7.取消片选 */
    nand_deselect();
}

/* 0：从Nand启动；1：从Nor启动 */
int isBootFromNorFlash(void)
{
    volatile int *p = (volatile int *)0;
    int val;
    int rlt;

    val = *p;
    *p = 0x12345678;

    if (*p == 0x12345678) {
        rlt = 0;    //Nand
    } else {
        rlt = 1;    //Nor
    }
    *p = val;

    return rlt;
}

void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)
{
    int i;

    puts("src:");
    puthex(src);
    puts("\n\r");

    puts("dest:");
    puthex(dest);
    puts("\n\r");

    puts("len:");
    puthex(len);
    puts("\n\r");

    if (isBootFromNorFlash()) {
        /* 如果是NOR启动 */
        for (i = 0; i < len; i++) {
            dest[i] = src[i];
        }
    } else {
        /* 如果是Nand启动 */
        //nand_init();
        nand_read((unsigned int)src, dest, len);
    }
}

void clear_bss(void)
{
    extern int __bss_start, __bss_end;
    int *p = &__bss_start;

    for (; p < &__bss_end; p++) {
        *p = 0;
    }
}

#define TXD0READY   (1<<2)
#define RXD0READY   (1)

#define PCLK                (50000000)
#define UART_CLK            PCLK
#define UART_BAUD_RATE      115200
#define UART_BRD            ((UART_CLK / (UART_BAUD_RATE * 16)) - 1)
void uart0_init(void)
{
    GPHCON |= 0xa0;
    GPHUP   = 0x0c;

    ULCON0  = 0x03;
    UCON0   = 0x05;
    UFCON0  = 0x00;
    UMCON0  = 0x00;
    UBRDIV0 = UART_BRD;
}

void putc(unsigned char c)
{
    /* 等待，直到发送缓冲区中的数据已经全部发送出去 */
    while (!(UTRSTAT0 & TXD0READY));

    UTXH0 = c;
}

void puts(char *str)
{
    int i = 0;
    while(str[i] != '\0') {
        putc(str[i]);
        i++;
    }
}

void puthex(unsigned int c)
{
    unsigned char ch;
    unsigned int tmp;
    int i;

    tmp = c;

    puts("0x");

    for (i = 0; i < (((sizeof(c) * 8) + 3) / 4); ++i) {
        ch = (tmp & 0xF0000000) >> 28;
        if (ch < 0x0a) {
            putc(ch + '0');
        } else {
            ch -= 0x0a;
            putc(ch + 'a');
        }
        tmp <<= 4;
    }
}

void debug_uart_a(void)
{
//    puts("debug uart init OK\n\r");
    putc('a');
}

void debug_uart_b(void)
{
//    puts("debug uart init OK\n\r");
    putc('b');
}

void debug_uart_c(void)
{
//    puts("debug uart init OK\n\r");
    putc('c');
}

void debug_uart_d(void)
{
//    puts("debug uart init OK\n\r");
    putc('d');
}

void debug_uart_e(void)
{
//    puts("debug uart init OK\n\r");
    putc('e');
}

