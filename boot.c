#include "regdef.h"
#include "setup.h"
#include "s3c2440_soc.h"

static struct tag *params;

static void setup_start_tag (void)
{
//    params = (struct tag *) bd->bi_boot_params;
    params = (struct tag *) 0x30000100;

    params->hdr.tag = ATAG_CORE;
    params->hdr.size = tag_size (tag_core);

    params->u.core.flags = 0;
    params->u.core.pagesize = 0;
    params->u.core.rootdev = 0;

    params = tag_next (params);
}


void setup_memory_tags(void)
{
    int i;

    for (i = 0; i < 1; i++) {
        params->hdr.tag = ATAG_MEM;
        params->hdr.size = tag_size (tag_mem32);

//        params->u.mem.start = bd->bi_dram[i].start;
//        params->u.mem.size = bd->bi_dram[i].size;
        params->u.mem.start = 0x30000000;
        params->u.mem.size = 64 * 1024 * 1024;

        params = tag_next (params);
    }
}

int strlen(char *str)
{
    const char *sc;

    for (sc = str; *sc != '\0'; ++sc);
    return sc - str;
}

char *strcpy(char *dest, char *src)
{
    char *tmp = dest;

    while((*dest++ = *src++) != '\0'); 

    return tmp;
}

void setup_commandline_tag(char *cmdline)
{
    char *p;
    int len;

    if (!cmdline) {
        return;
    }

    /* eat leading white space */
    for (p = cmdline; *p == ' '; p++);

    /* skip non-existent command lines so the kernel will still
    * use its default command line.
    */
    if (*p == '\0') {
        return;
    }

    len = strlen(p) + 1;

    params->hdr.tag = ATAG_CMDLINE;
    params->hdr.size =
        (sizeof (struct tag_header) + len + 3) >> 2;

    strcpy (params->u.cmdline.cmdline, cmdline);

    params = tag_next (params);

}

void setup_end_tag(void)
{
    params->hdr.tag = ATAG_NONE;
    params->hdr.size = 0;
}


void main(void)
{
#define MACH_TYPE_S3C2440              362
    void (*theKernel)(int zero, int arch, unsigned int params);
    volatile unsigned int *p = (volatile unsigned int *)0x30008000;

    /* 0.设置串口：内核启动的开启部分会从串口打印一些信息，但是内核一开始没有初始化串口 */
    uart0_init();

    puthex(0x1234abcd);
    /* 1.从NAND FLASH里把内核读入内存 */
    puts("Copy kernel from nand\n\r");
    nand_read(0x60000+64, (unsigned char *)0x30008000, 0x200000);
    puts("(unsigned int *)0x30008000:\n\r");
    puthex(*p);
    puts("\n\r");

    /* 2.设置参数 */
    puts("Set boot params\n\r");
    setup_start_tag ();
    setup_memory_tags ();
    setup_commandline_tag ("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0,115200");
    setup_end_tag ();

    /* 3.跳转执行 */
    puts("boot kernel\n\r");
    theKernel = (void (*)(int, int, unsigned))0x30008000;
    theKernel (0, MACH_TYPE_S3C2440, 0x30000100); //mov pc, #0x30008000

    /* 如果成功跳转，不会再回到这里 */
    puts("error\n\r");
}

