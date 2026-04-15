/* qemu-virt.ld: 完整的 QEMU RISC-V Virt 链接脚本，兼容 Picolibc */
OUTPUT_ARCH("riscv")
ENTRY(_start)

/* 强制指定 QEMU Virt 的大内存布局 */
MEMORY
{
    ROM (rx)  : ORIGIN = 0x80000000, LENGTH = 64M  /* .text */
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 512M /* .data */
}

/* 全局指针初始化 (Picolibc 和 RISC-V 必须) */
PROVIDE(__global_pointer$ = . + 0x800);

SECTIONS
{
    .text : ALIGN(16) {
        KEEP(*(.text._start))
        KEEP(*(.init))
        KEEP(*(.fini))

        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)

        . = ALIGN(16);
    } > ROM

    .rodata : ALIGN(16) {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)

        . = ALIGN(8);
        __EH_FRAME_BEGIN__ = .;
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame.*))
        . = ALIGN(8);

        . = ALIGN(16);
    } > ROM

    .preinit_array : ALIGN(8) {
        PROVIDE(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE(__preinit_array_end = .);
    } > ROM

    .init_array : ALIGN(8) {
        PROVIDE(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE(__init_array_end = .);
    } > ROM

    .fini_array : ALIGN(8) {
        PROVIDE(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE(__fini_array_end = .);
    } > ROM

    .data : ALIGN(16) {
        _sdata = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)

        . = ALIGN(16);
        _edata = .;
    } > RAM AT > ROM
    _sidata = LOADADDR(.data);

    .sdata : ALIGN(16) {
        __sdata_start = .;
        *(.sdata .sdata.* .gnu.linkonce.s.*)
        . = ALIGN(16);
        __sdata_end = .;
    } > RAM AT > ROM

    .sbss (NOLOAD) : ALIGN(16) {
        __sbss_start = .;
        *(.sbss .sbss.* .gnu.linkonce.sb.*)
        . = ALIGN(16);
        __sbss_end = .;
    } > RAM

    .bss (NOLOAD) : ALIGN(16) {
        _sbss = .;
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(16);
        _ebss = .;
        __bss_end = .;
    } > RAM

    .heap (NOLOAD) : ALIGN(16) {
        _heap_start = .;
        . = . + 1M;
        . = ALIGN(16);
        _heap_end = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . = . + 64K;
        . = ALIGN(16);
        _stack_top = .;
    } > RAM

    PROVIDE(__flash_start = ORIGIN(ROM));
    PROVIDE(__flash_size = LENGTH(ROM));
    PROVIDE(__ram_start = ORIGIN(RAM));
    PROVIDE(__ram_size = LENGTH(RAM));

    PROVIDE(end = .);
    PROVIDE(_end = .);
}
