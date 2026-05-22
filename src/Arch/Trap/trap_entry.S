/* trap_entry.S — machine-mode trap vector (RV64, based on 05-traps/entry.S) */

#define LOAD    ld
#define STORE   sd
#define SIZE_REG 8

/* Save GP registers to context at base (in t6). gp/tp slots are skipped. */
.macro reg_save base
    STORE ra,   0*SIZE_REG(\base)
    STORE sp,   1*SIZE_REG(\base)
    STORE t0,   4*SIZE_REG(\base)
    STORE t1,   5*SIZE_REG(\base)
    STORE t2,   6*SIZE_REG(\base)
    STORE s0,   7*SIZE_REG(\base)
    STORE s1,   8*SIZE_REG(\base)
    STORE a0,   9*SIZE_REG(\base)
    STORE a1,  10*SIZE_REG(\base)
    STORE a2,  11*SIZE_REG(\base)
    STORE a3,  12*SIZE_REG(\base)
    STORE a4,  13*SIZE_REG(\base)
    STORE a5,  14*SIZE_REG(\base)
    STORE a6,  15*SIZE_REG(\base)
    STORE a7,  16*SIZE_REG(\base)
    STORE s2,  17*SIZE_REG(\base)
    STORE s3,  18*SIZE_REG(\base)
    STORE s4,  19*SIZE_REG(\base)
    STORE s5,  20*SIZE_REG(\base)
    STORE s6,  21*SIZE_REG(\base)
    STORE s7,  22*SIZE_REG(\base)
    STORE s8,  23*SIZE_REG(\base)
    STORE s9,  24*SIZE_REG(\base)
    STORE s10, 25*SIZE_REG(\base)
    STORE s11, 26*SIZE_REG(\base)
    STORE t3,  27*SIZE_REG(\base)
    STORE t4,  28*SIZE_REG(\base)
    STORE t5,  29*SIZE_REG(\base)
.endm

.macro reg_restore base
    LOAD ra,   0*SIZE_REG(\base)
    LOAD sp,   1*SIZE_REG(\base)
    LOAD t0,   4*SIZE_REG(\base)
    LOAD t1,   5*SIZE_REG(\base)
    LOAD t2,   6*SIZE_REG(\base)
    LOAD s0,   7*SIZE_REG(\base)
    LOAD s1,   8*SIZE_REG(\base)
    LOAD a0,   9*SIZE_REG(\base)
    LOAD a1,  10*SIZE_REG(\base)
    LOAD a2,  11*SIZE_REG(\base)
    LOAD a3,  12*SIZE_REG(\base)
    LOAD a4,  13*SIZE_REG(\base)
    LOAD a5,  14*SIZE_REG(\base)
    LOAD a6,  15*SIZE_REG(\base)
    LOAD a7,  16*SIZE_REG(\base)
    LOAD s2,  17*SIZE_REG(\base)
    LOAD s3,  18*SIZE_REG(\base)
    LOAD s4,  19*SIZE_REG(\base)
    LOAD s5,  20*SIZE_REG(\base)
    LOAD s6,  21*SIZE_REG(\base)
    LOAD s7,  22*SIZE_REG(\base)
    LOAD s8,  23*SIZE_REG(\base)
    LOAD s9,  24*SIZE_REG(\base)
    LOAD s10, 25*SIZE_REG(\base)
    LOAD s11, 26*SIZE_REG(\base)
    LOAD t3,  27*SIZE_REG(\base)
    LOAD t4,  28*SIZE_REG(\base)
    LOAD t5,  29*SIZE_REG(\base)
    LOAD t6,  30*SIZE_REG(\base)
.endm

.section .text.mvec_table, "ax", @progbits
.align 4
.globl m_trap_vector_base

/*
 * RISC-V M-Mode Trap Vector Table (Vector Mode)
 * Hardware jumps to: base + 4 * mcause
 */
m_trap_vector_base:
    j       trap_exc_common         /* cause 0  : exception */
    j       trap_int_msw            /* cause 1  : M-mode software interrupt */
    j       trap_int_ssw            /* cause 2  : S-mode software interrupt */
    j       trap_resv0               /* cause 3  : reserved */
    j       trap_int_mtimer         /* cause 4  : M-mode timer interrupt */
    j       trap_int_stimer         /* cause 5  : S-mode timer interrupt */
    j       trap_resv1               /* cause 6  : reserved */
    j       trap_int_mext           /* cause 7  : M-mode external interrupt */
    j       trap_int_sext           /* cause 8  : S-mode external interrupt */
    .rept   24                      /* Fill remaining causes */
    j       trap_unknown
    .endr

/* Reserved / unknown traps */
trap_resv0:
trap_resv1:
trap_unknown:
    j       trap_exc_common

/*
 * Individual trap handlers – all jump to common save path
 */
trap_exc_common:
trap_int_msw:
trap_int_ssw:
trap_int_mtimer:
trap_int_stimer:
trap_int_mext:
trap_int_sext:
    j       trap_save_entry

/*
 * Common trap entry: save context, call C handler, restore context
 */
trap_save_entry:
    /* Swap t6 with mscratch to get context pointer */
    csrrw   t6, mscratch, t6
    reg_save t6

    /* Save original t6 into the context frame */
    mv      t5, t6
    csrr    t6, mscratch
    STORE   t6, 30 * SIZE_REG(t5)

    # Restore the context pointer into mscratch
    csrw    mscratch, t5

    /* Pass mepc (a0) and mcause (a1) to C dispatch function */
    csrr    a0, mepc
    csrr    a1, mcause
    csrr    a2, mtval

    call    m_trap_handler
    /* Write back return address */
    csrw    mepc, a0

/* Restore context and return from trap */
trap_restore_exit:
    csrr    t6, mscratch
    reg_restore t6
    mret
