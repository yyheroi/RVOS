/**
    Script for _os_start.ld
*/
OUTPUT_ARCH("riscv")
ENTRY(_start)
PROVIDE(__global_pointer$ = . + 0x800);
MEMORY
{
    /** We have encountered a problem, but we don't have a good solution for the time being.
    *   newlib uses code model medlow (Only +- 2GB), but I now need to load RAM to 0x80000000 of qemu virt

        RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 2M
        ROM (rx)  : ORIGIN = 0x82000000, LENGTH = 4M
    */

    RAM (rwx)  : ORIGIN = 0x80000000, LENGTH = 1024M
    ROM (rx)   : ORIGIN = 0x80200000, LENGTH = 1024M
}

SECTIONS
{
    .text : ALIGN(16) {
        KEEP(*(.text._start))
        *(.text)
        *(.text.*)
    } > ROM

    .rodata : ALIGN(16) {
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(16);
    } > ROM

    /* Runtime .data in RAM, initial image in ROM. */
    .data : ALIGN(16) {
        _sdata = .;
        *(.data)
        *(.data.*)
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

    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . = . + 0x4000; /* 16 KiB stack */
        . = ALIGN(16);
        _stack_top = .;
    } > RAM

    _end = .;
}
