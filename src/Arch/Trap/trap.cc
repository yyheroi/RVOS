/**
 * @file trap.cc
 * @brief Machine-mode trap dispatch
 */

#include <cstdio>
#include <string_view>
#include <cstdint>

#include "CSR/CsrManager.hh"
#include "Trap/trap.hh"

extern "C" void m_trap_vector_base(void);

namespace Arch::Trap {

using ::Arch::CSR::CsrManager;
using ::Arch::CSR::CsrReg;

namespace {

std::string_view ExceptionName(uint64_t code) noexcept
{
    switch(static_cast<SyncException>(code)) {
    case SyncException::INST_ADDR_MISALIGNED: return "inst addr misaligned";
    case SyncException::INST_ACCESS_FAULT: return "inst access fault";
    case SyncException::ILLEGAL_INST: return "illegal instruction";
    case SyncException::BREAKPOINT: return "breakpoint";
    case SyncException::LOAD_ADDR_MISALIGNED: return "load addr misaligned";
    case SyncException::LOAD_ACCESS_FAULT: return "load access fault";
    case SyncException::STORE_ADDR_MISALIGNED: return "store addr misaligned";
    case SyncException::STORE_AMO_FAULT: return "store/amo access fault";
    case SyncException::ECALL_FROM_U_MODE: return "ecall from U-mode";
    case SyncException::ECALL_FROM_S_MODE: return "ecall from S-mode";
    case SyncException::ECALL_FROM_M_MODE: return "ecall from M-mode";
    default: return "unknown";
    }
}

[[noreturn]] void PanicOnSyncException(const TrapInfo &info) noexcept
{
    const uint64_t hart= CsrManager::Instance().Read(CsrReg::MHARTID);
    printf("PANIC hart%lu: sync trap code=%lu (%s) mepc=0x%lx mtval=0x%lx\n",
                hart,
                info.Code(),
                ExceptionName(info.Code()).data(),
                info.mepc_,
                info.mtval_);

    while(true) {
        __asm__ volatile("wfi");
    }
}

MachineTrap &TrapForHart(uint64_t hartid) noexcept
{
    static MachineTrap traps[G_K_MAX_HARTS] {};

    if(hartid >= G_K_MAX_HARTS) {
        hartid= 0;
    }
    return traps[hartid];
}

} // namespace

std::string_view SyncName(uint64_t code) noexcept
{
    return ExceptionName(code);
}

MachineTrap &MachineTrap::GetInstance() noexcept
{
    return GetInstance(CsrManager::Instance().Read(CsrReg::MHARTID));
}

MachineTrap &MachineTrap::GetInstance(uint64_t hartid) noexcept
{
    return TrapForHart(hartid);
}

void MachineTrap::SetupVector()
{
    CsrManager::Instance().Write(CsrReg::MSCRATCH, reinterpret_cast<uint64_t>(&TrapContext_));
    CsrManager::Instance().Write(CsrReg::MTVEC, reinterpret_cast<uint64_t>(
        reinterpret_cast<uintptr_t>(m_trap_vector_base) | 1U));
}

uint64_t MachineTrap::OnTrap(const TrapInfo &info)
{
    LastTrapRecord_.Save(info);

    if(info.IsInterrupt()) {
        const uint64_t hart= CsrManager::Instance().Read(CsrReg::MHARTID);
        switch(info.Code()) {
        case 3:
            printf("hart%lu interrupt: M-mode software\n", hart);
            break;
        case 7:
            printf("hart%lu interrupt: M-mode timer\n", hart);
            break;
        case 11:
            printf("hart%lu interrupt: M-mode external\n", hart);
            break;
        default:
            printf("hart%lu interrupt: code=%lu\n", hart, info.Code());
            break;
        }
        return info.mepc_;
    }

    switch(static_cast<SyncException>(info.Code())) {
    /* trap_test: skip faulting insn (mepc += 4); extend policy here later */
    case SyncException::ILLEGAL_INST:
    case SyncException::BREAKPOINT:
    case SyncException::LOAD_ACCESS_FAULT:
    case SyncException::LOAD_ADDR_MISALIGNED:
    case SyncException::STORE_ADDR_MISALIGNED:
    case SyncException::STORE_AMO_FAULT:
    case SyncException::ECALL_FROM_M_MODE:
        return info.mepc_ + 4U;

    default:
        PanicOnSyncException(info);
    }
}

void MachineTrap::Init()
{
    /* Boot path: only hart 0 runs C init today. Other harts call SetupVector() when brought up. */
    GetInstance(0).SetupVector();
}

void Init()
{
    MachineTrap::Init();
}

} // namespace Arch::Trap

extern "C" uint64_t m_trap_handler(uint64_t mepc, uint64_t mcause, uint64_t mtval)
{
    const Arch::Trap::TrapInfo info{mepc, mcause, mtval};
    return Arch::Trap::MachineTrap::GetInstance().OnTrap(info);
}

extern "C" void TrapInit(void)
{
    Arch::Trap::Init();
}

