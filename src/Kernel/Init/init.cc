#include <cstdio>

#include "Arch/Trap/trap.hh"
#include "Arch/CSR/CsrManager.hh"
#include "trap_test.hh"

extern "C" void KernelInit(void)
{
    TrapInit();
    std::printf("Hello, RVOS\n");
    TrapTest();
}

int main()
{
    KernelInit();
    return 0;
}
