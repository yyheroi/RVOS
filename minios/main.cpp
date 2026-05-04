#include <cstdio>
#include <string>
#include <vector>

extern "C" void uart_init();
extern "C" void uart_puts(const char *s);

int main() {
    uart_init();

    std::vector<int> const v{1, 2, 3};
    // std::array<int, 3> const v{{1, 2, 3}};
    std::string line = "STL: ";
    for (int x : v) {
        line.push_back(static_cast<char>('0' + x));
        line.push_back(' ');
    }
    line += "\n";
    uart_puts(line.c_str());

    printf("Hello, printf -> write(1) -> UART\n");
    return 0;
}

