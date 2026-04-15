/* qemu-virt.ld: 修正版，适配 Picolibc 和 RISC-V 64位 */
OUTPUT_ARCH("riscv")
ENTRY(_start)

/* 内存布局定义 */
MEMORY
{
    /* QEMU Virt Machine 默认 DDR 起始地址 */
    ROM (rx)  : ORIGIN = 0x40000000, LENGTH = 64M
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 512M
}

/* 关键修复 1: 定义全局指针位置，必须位于 .sdata 附近以便 -mcmodel=medany 工作 */
PROVIDE(__global_pointer$ = . + 0x800);

SECTIONS
{
    /* --- 代码段 --- */
    .text : ALIGN(16) {
        KEEP(*(.text._start)) /* 确保启动代码在最前面 */
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
        . = ALIGN(16);
    } > ROM

    /* --- 只读数据 --- */
    .rodata : ALIGN(16) {
        *(.rodata .rodata.*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(8);

        /* 异常帧处理 */
        __EH_FRAME_BEGIN__ = .;
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame.*))
        . = ALIGN(8);
    } > ROM

    /* --- 初始化数组 (C++ 构造函数等) --- */
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

    /* --- 数据段 (已初始化) --- */
    /* 关键修复 2: 明确 LMA (加载地址) 和 VMA (运行地址) */
    _sidata = LOADADDR(.data);

    .data : ALIGN(16) {
        _sdata = .;
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(16);
        _edata = .;
    } > RAM AT > ROM

    /* --- 小数据段 (SData/SBSS) --- */
    /* 关键修复 3: Picolibc 依赖这些符号来通过 gp 寄存器快速访问 */
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

    /* --- BSS 段 (未初始化) --- */
    .bss (NOLOAD) : ALIGN(16) {
        _sbss = .;
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON) /* 必须包含 COMMON，否则未初始化全局变量会丢失 */
        . = ALIGN(16);
        _ebss = .;
        __bss_end = .;
    } > RAM

    /* --- 堆 (Heap) --- */
    /* 关键修复 4: 提供双下划线版本 __heap_start，适配 Picolibc */
    .heap (NOLOAD) : ALIGN(16) {
        _heap_start = .;
        PROVIDE(__heap_start = .); /* 修复 undefined reference */
        . = . + 1M; /* 堆大小，可根据需要调整 */
        _heap_end = .;
        PROVIDE(__heap_end = .);
    } > RAM

    /* --- 栈 (Stack) --- */
    .stack (NOLOAD) : ALIGN(16) {
        _stack_bottom = .;
        . = . + 64K; /* 栈大小 */
        _stack_top = .;
    } > RAM

    /* 调试符号 */
    PROVIDE(end = .);
    PROVIDE(_end = .);
}
