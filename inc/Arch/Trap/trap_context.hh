/**
 * @file trap_context.hh
 * @brief GP register frame saved by trap_entry.S (layout must match assembly)
 */
#pragma once

#include <cstdint>

namespace Arch::Trap {

struct TrapContext {
    std::uint64_t ra_;
    std::uint64_t sp_;
    std::uint64_t gp_; // slot reserved, not saved by trap_entry
    std::uint64_t tp_; // slot reserved, not saved by trap_entry
    std::uint64_t t0_;
    std::uint64_t t1_;
    std::uint64_t t2_;
    std::uint64_t s0_;
    std::uint64_t s1_;
    std::uint64_t a0_;
    std::uint64_t a1_;
    std::uint64_t a2_;
    std::uint64_t a3_;
    std::uint64_t a4_;
    std::uint64_t a5_;
    std::uint64_t a6_;
    std::uint64_t a7_;
    std::uint64_t s2_;
    std::uint64_t s3_;
    std::uint64_t s4_;
    std::uint64_t s5_;
    std::uint64_t s6_;
    std::uint64_t s7_;
    std::uint64_t s8_;
    std::uint64_t s9_;
    std::uint64_t s10_;
    std::uint64_t s11_;
    std::uint64_t t3_;
    std::uint64_t t4_;
    std::uint64_t t5_;
    std::uint64_t t6_;
};

static_assert(sizeof(TrapContext) == 31U * sizeof(std::uint64_t));

} // namespace Arch
