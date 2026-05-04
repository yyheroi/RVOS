/*
 * S 态 crt0 入口（与 OpenSBI「-bios default -kernel」配套）。
 *
 * 发行版 picolibc 的 crt0 默认按「从复位进 M 态」写，会去改 mstatus/mtvec，
 * OpenSBI 把 payload 跳进 S 态时这些指令非法，进不了 main。
 *
 * 本文件替代官方 crt0 的汇编入口职责：栈、gp，再调用 C 里搬 .data/清 BSS，最后
 * __libc_init_array → main。（链接仍须 -nostartfiles，避免再把官方 crt0 链进来）
 */
.section .text.init.enter
.globl _start

.extern picolibc_s_bootstrap_bss
.extern __libc_init_array

_start:
    la sp, __stack
    la gp, __global_pointer$
    call picolibc_s_bootstrap_bss
    call __libc_init_array
    call main

1:
    j 1b
