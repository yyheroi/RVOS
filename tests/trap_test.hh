/**
 * @file trap_test.hh
 * @brief Synchronous trap demos (QEMU / bring-up only)
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void TrapTest(void);

#ifdef __cplusplus
}

namespace Arch::Trap {

void RunSyncTests();

} // namespace Arch::Trap
#endif
