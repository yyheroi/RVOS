#include "CSR/CsrManager.hh"
#include <cassert>

namespace Arch::CSR {

#define CSR_RW_CASES(X) \
    X(MVENDORID, mvendorid) \
    X(MARCHID, marchid) \
    X(MIMPID, mimpid) \
    X(MHARTID, mhartid) \
    X(MSTATUS, mstatus) \
    X(MISA, misa) \
    X(MEDELEG, medeleg) \
    X(MIDELEG, mideleg) \
    X(MIE, mie) \
    X(MTVEC, mtvec) \
    X(MCOUNTEREN, mcounteren) \
    X(MSTATUSH, mstatush) \
    X(MSCRATCH, mscratch) \
    X(MEPC, mepc) \
    X(MCAUSE, mcause) \
    X(MTVAL, mtval) \
    X(MIP, mip) \
    X(MTINST, mtinst) \
    X(MTVAL2, mtval2)

void CsrManager::writeReg(CsrReg csr, uint64_t value) noexcept
{
    switch(csr) {
#define CSR_WRITE_CASE(reg, asm_name) \
    case CsrReg::reg: \
        __asm__ volatile("csrw " #asm_name ", %0" : : "r"(value)); \
        return;
        CSR_RW_CASES(CSR_WRITE_CASE)
#undef CSR_WRITE_CASE
    default:
        return;
    }
}

uint64_t CsrManager::readReg(CsrReg csr) noexcept
{
    uint64_t value= 0;
    switch(csr) {
#define CSR_READ_CASE(reg, asm_name) \
    case CsrReg::reg: \
        __asm__ volatile("csrr %0, " #asm_name : "=r"(value)); \
        return value;
        CSR_RW_CASES(CSR_READ_CASE)
#undef CSR_READ_CASE
    default:
        return 0;
    }
}

#undef CSR_RW_CASES


void CsrManager::Write(CsrReg csr, uint64_t value) noexcept
{
    writeReg(csr, value);
}

uint64_t CsrManager::Read(CsrReg csr) noexcept
{
    return readReg(csr);
}

} // namespace Arch::CSR
