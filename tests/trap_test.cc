/**
 * @file trap_test.cc
 * @brief Deliberate sync-exception triggers for trap bring-up
 */

#include <cstdio>
#include <optional>

#include "Arch/Trap/trap.hh"
#include "trap_test.hh"

namespace Arch::Trap {

namespace {

constexpr std::uintptr_t kFaultAddress= 0x200U;

void TriggerStoreAccessFault()
{
    *reinterpret_cast<volatile int *>(kFaultAddress)= 100;
}

void TriggerLoadAccessFault()
{
    (void)*reinterpret_cast<volatile int *>(kFaultAddress);
}

void TriggerStoreMisaligned()
{
    alignas(8) static std::uint8_t buffer[16] {};
    __asm__ volatile(
        "addi t0, %0, 2\n"
        "sw   zero, 0(t0)\n"
        :
        : "r"(buffer)
        : "memory", "t0");
}

void TriggerLoadMisaligned()
{
    alignas(8) static std::uint8_t buffer[16] {};
    std::uint64_t tmp;
    __asm__ volatile(
        "addi t0, %0, 3\n"
        "ld   %1, 0(t0)\n"
        : "=r"(tmp)
        : "r"(buffer)
        : "memory", "t0");
}

void TriggerIllegalInst()
{
    __asm__ volatile(".word 0x00000000");
}

void TriggerBreakpoint()
{
    __asm__ volatile("ebreak");
}

void TriggerEcallM()
{
    __asm__ volatile("ecall");
}

struct SyncTestCase {
    const char *label;
    std::optional<std::uint64_t> expect;
    void (*trigger)();
};

constexpr SyncTestCase kSyncTests[] {
    {"store access fault", static_cast<std::uint64_t>(SyncException::STORE_AMO_FAULT), TriggerStoreAccessFault},
    {"load access fault", static_cast<std::uint64_t>(SyncException::LOAD_ACCESS_FAULT), TriggerLoadAccessFault},
    {"store addr misaligned", static_cast<std::uint64_t>(SyncException::STORE_ADDR_MISALIGNED), TriggerStoreMisaligned},
    {"load addr misaligned", static_cast<std::uint64_t>(SyncException::LOAD_ADDR_MISALIGNED), TriggerLoadMisaligned},
    {"illegal instruction", static_cast<std::uint64_t>(SyncException::ILLEGAL_INST), TriggerIllegalInst},
    {"breakpoint", std::nullopt, TriggerBreakpoint},
    {"ecall M-mode", static_cast<std::uint64_t>(SyncException::ECALL_FROM_M_MODE), TriggerEcallM},
};

void RunOneCase(MachineTrap &trap, const SyncTestCase &tc)
{
    trap.Last().Clear();
    tc.trigger();

    if(!trap.Last().Hit()) {
        std::printf("[%-22s] (no trap — CPU/QEMU may handle this case)\n", tc.label);
        return;
    }

    const auto &snap= trap.Last();
    std::printf("[%-22s] code=%2lu (%s) mepc=0x%lx mtval=0x%lx\n",
                tc.label,
                snap.cause_,
                SyncName(snap.cause_).data(),
                snap.epc_,
                snap.mtval_);

    if(tc.expect.has_value() && *tc.expect != snap.cause_) {
        std::printf("  -> expected mcause=%lu (%s)\n",
                    *tc.expect,
                    SyncName(*tc.expect).data());
    }
}

} // namespace

void RunSyncTests()
{
    auto &trap= MachineTrap::GetInstance();

    std::printf("=== synchronous exception tests ===\n");
    for(const auto &tc: kSyncTests) {
        RunOneCase(trap, tc);
    }
    std::printf(
        "(skipped inst addr misaligned / inst access fault: mepc at fault\n"
        " target; mepc+=4 cannot return to caller.)\n");
    std::printf("=== all exception tests finished ===\n");
}

} // namespace Arch::Trap

extern "C" void TrapTest(void)
{
    Arch::Trap::RunSyncTests();
}
