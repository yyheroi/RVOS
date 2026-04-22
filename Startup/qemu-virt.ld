__flash = 0x80000000;
__flash_size = 0x00080000;
__ram = 0x80080000;
__ram_size = 0x40000;
__stack_size = 1k;
INCLUDE picolibcpp.ld

/* os_start.S 使用 newlib 时代命名；picolibcpp.ld 提供 __stack、__data_* 等 */
PROVIDE(_stack_top = __stack);
PROVIDE(_sidata = __data_source);
PROVIDE(_sdata = __data_start);
PROVIDE(_edata = __edata);
PROVIDE(_sbss = __non_tls_bss_start);
PROVIDE(_ebss = __bss_end);
PROVIDE(__sbss_start = __bss_end);
PROVIDE(__sbss_end = __bss_end);
