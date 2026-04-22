# RISC-V cross toolchain for RVOS (Linux userland triple or bare-metal ELF).
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/riscv64-linux-gnu.cmake -S . -B build
#
# Requires a RISC-V gcc/g++ on PATH (see fatal message below if missing).

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

find_program(RISCV_GCC
    NAMES
        riscv64-linux-gnu-gcc
        riscv64-unknown-elf-gcc
        riscv64-elf-gcc
    DOC "RISC-V C compiler")
find_program(RISCV_GXX
    NAMES
        riscv64-linux-gnu-g++
        riscv64-unknown-elf-g++
        riscv64-elf-g++
    DOC "RISC-V C++ compiler")

if(NOT RISCV_GCC OR NOT RISCV_GXX)
    message(FATAL_ERROR
        "No RISC-V cross compiler found in PATH (need both gcc and g++).\n"
        "  Debian/Ubuntu (linux-gnu): sudo apt install gcc-riscv64-linux-gnu g++-riscv64-linux-gnu\n"
        "  Bare-metal (common for embedded): install a riscv64-unknown-elf toolchain and ensure\n"
        "    riscv64-unknown-elf-gcc / riscv64-unknown-elf-g++ are on PATH, or set:\n"
        "    -DCMAKE_C_COMPILER=/full/path/to/riscv64-...-gcc\n"
        "    -DCMAKE_CXX_COMPILER=/full/path/to/riscv64-...-g++\n"
        "    -DCMAKE_ASM_COMPILER=/full/path/to/riscv64-...-gcc\n"
        "Found gcc='${RISCV_GCC}' g++='${RISCV_GXX}'")
endif()

set(CMAKE_C_COMPILER "${RISCV_GCC}")
set(CMAKE_CXX_COMPILER "${RISCV_GXX}")
# Assemble .S with the same gcc as C (GNU as).
set(CMAKE_ASM_COMPILER "${RISCV_GCC}")
