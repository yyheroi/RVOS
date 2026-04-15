// #include <cstdio>
#include "Drivers/Uart.hh"
// #include <stdio.h>
extern "C" {
    extern void uart_init(void);
    extern void uart_puts(char *s);

    int KernelInit()
    {
        // puts("Hello, RVOS");
        // auto uart0= RVOSHal::Uart::Default();
        // if(uart0) {
        //     (void)RVOSHal::InitPeripheral(*uart0);
        // }
        RVOSHal::Uart uart0;
        uart0.WriteString("Hello RISC-V!\n");
        // printf("Hello RISC-V!\n");
        // (void)uart_puts("Hello RISC-V!\n");
        // uart_init();
        // uart_puts("Hello, RVOS!\n");
        return 0;
    }
}

int main()
{
    KernelInit();
    return 0;
}
