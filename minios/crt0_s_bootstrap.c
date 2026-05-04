/*
 * picolibc + picolibcpp.ld： .data/.tdata 落在 RAM(VMA)，初值在 Flash(LMA)。
 * 无官方 crt0 时在此处完成复制，并把 .tbss+.bss 清零（与 crt0._cstart 逻辑一致）。
 */
#include <stddef.h>

extern char __data_start[];
extern char __data_end[];
extern char __data_source[];
extern char __bss_start[];
extern char __bss_end[];

void picolibc_s_bootstrap_bss(void)
{
    char *d;
    const char *s;
    size_t n;

    n = (size_t)(__data_end - __data_start);
    if (n != 0U) {
        d = __data_start;
        s = __data_source;
        while (n-- != 0U)
            *d++ = *s++;
    }

    for (d = __bss_start; d < __bss_end; ++d)
        *d = 0;
}
