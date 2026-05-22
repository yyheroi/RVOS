/**
 * @file trap.hh
 * @brief Machine-mode trap (M-mode, vectored)
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include "trap_context.hh"

namespace Arch::Trap {

enum class SyncException : uint64_t {
    INST_ADDR_MISALIGNED = 0,
    INST_ACCESS_FAULT    = 1,
    ILLEGAL_INST        = 2,
    BREAKPOINT         = 3,
    LOAD_ADDR_MISALIGNED = 4,
    LOAD_ACCESS_FAULT    = 5,
    STORE_ADDR_MISALIGNED= 6,
    STORE_AMO_FAULT      = 7,
    ECALL_FROM_U_MODE     = 8,
    ECALL_FROM_S_MODE     = 9,
    ECALL_FROM_M_MODE     = 11,
};

/** mcause CSR: bit63 = interrupt, bits[62:0] = exception / interrupt code */
union Mcause {
    uint64_t raw_ = 0;
    struct {
        uint64_t code_ : 63;
        uint64_t interrupt_ : 1;
    };
};

struct TrapInfo {
    uint64_t mepc_  = 0;
    uint64_t mtval_ = 0;
    Mcause mcause_ {};

    TrapInfo()= default;

    TrapInfo(uint64_t mepc, uint64_t mcauseRaw, uint64_t mtval= 0) noexcept
        : mepc_(mepc)
        , mtval_(mtval)
    {
        mcause_.raw_= mcauseRaw;
    }

    [[nodiscard]] bool IsInterrupt() const noexcept { return mcause_.interrupt_; }

    [[nodiscard]] uint64_t Code() const noexcept { return mcause_.code_; }
};

struct TrapRecord {
    static constexpr uint64_t G_kNone= ~0ULL;

    uint64_t cause_ = G_kNone;
    uint64_t epc_   = 0;
    uint64_t mtval_ = 0;

    void Clear() noexcept
    {
        cause_= G_kNone;
        epc_   = 0;
        mtval_ = 0;
    }

    [[nodiscard]] bool Hit() const noexcept { return cause_ != G_kNone; }

    void Save(const TrapInfo &info) noexcept
    {
        cause_= info.Code();
        epc_  = info.mepc_;
        mtval_= info.mtval_;
    }
};

std::string_view SyncName(uint64_t code) noexcept;

/**
 * Per-hart trap state. QEMU virt supports up to 8 harts; bump when the SoC changes.
 * Boot hart calls Init(); other harts must run SetupVector() on themselves before taking traps.
 */
inline constexpr size_t G_K_MAX_HARTS= 8;

class MachineTrap {
public:
    /** Current hart (csrr mhartid). */
    static MachineTrap &GetInstance() noexcept;

    /** Explicit hart id — for init on secondary harts or tests. */
    static MachineTrap &GetInstance(uint64_t hartid) noexcept;

    /** mscratch <- this hart's TrapContext; mtvec <- shared vector table. */
    void SetupVector();

    uint64_t OnTrap(const TrapInfo &info);

    [[nodiscard]] TrapRecord &Last() noexcept { return LastTrapRecord_; }

    static void Init();

private:
    alignas(16) TrapContext TrapContext_ {};
    TrapRecord LastTrapRecord_ {};
};
} // namespace Arch::Trap

extern "C" void TrapInit(void);