#include "Trap/Trap.hh"
#include "Trap.hh"
#include "CSR/CSRManager.hh"
#include <cwchar>
#include <cstdio>
volatile static int g_started= 0;

extern "C" void test_trap(void)
{
    auto &csr= arch::csr::Manager::Instance();

    if(csr.Cpuid() == 0) {
        arch::trap::SupervisorTrap::Init();
        std::printf("Hello, RVOS\n");
        arch::trap::RunSyncTests();
        g_started= 1;
    } else {
        while(g_started == 0)
            ;
        arch::trap::SupervisorTrap::Init();
        // printf("hart %d starting \n", arch::csr::Manager::Instance().Cpuid());
    }
}

int main()
{
    arch::csr::Manager::Instance().StartSupervisorMode(&test_trap);
    return 0;
}
