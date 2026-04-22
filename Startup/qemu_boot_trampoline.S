/* QEMU virt + -kernel 时，固件从 FLASH 起点 (0x80000000) 取指，而不是 ELF e_entry。
 * picolibcpp.ld 把 .text.init.enter 放在 .text 之前；此处仅占位跳转，真正的入口仍是 _start。 */
.section .text.init.enter, "ax", @progbits
.global rvos_qemu_entry
.type rvos_qemu_entry, @function

rvos_qemu_entry:
    j _start

.size rvos_qemu_entry, . - rvos_qemu_entry
