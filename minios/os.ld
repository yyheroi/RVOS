/* -bios default（OpenSBI）时一般用 0x80200000；无固件时 QEMU virt 复位会跳到 RAM 起始 0x80000000，须把代码段对齐到这里，否则会取到全 0 非法指令。 */
__flash = 0x80200000;       /* 代码/rodata/linker VMA 基底（须在复位向量可达区间） */
__flash_size = 32M;
__ram = 0x90000000;        /* DRAM 中高段：data/bss/stack/heap，勿与 FLASH 重叠 */
__ram_size = 512M;
__stack_size = 64K;
__heap_size_min = 128K;

INCLUDE picolibcpp.ld

