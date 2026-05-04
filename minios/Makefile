CROSS_COMPILE = /mnt/riscv-ssd/opt/riscv/bin/riscv64-unknown-elf-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AS = $(CROSS_COMPILE)as
OBJCOPY = $(CROSS_COMPILE)objcopy

PREFIX  = /usr/lib/picolibc/riscv64-unknown-elf
ARCH_FLAGS = -march=rv64imafdc -mabi=lp64d -mcmodel=medany
CSPECS      = -specs=$(PREFIX)/picolibc.specs
CXXSPECS  = -specs=$(PREFIX)/picolibcpp.specs

PICOLIBC_ROOT = $(PREFIX)
PICOLIBC_LIB = -L$(PICOLIBC_ROOT)/lib

GCC_BUILTIN_INC := $(shell $(CC) -print-file-name=include)
GCC_BUILTIN_FIXED := $(shell $(CC) -print-file-name=include-fixed)
PICOLIBC_SYSINC_C = -nostdinc \
	-isystem $(PICOLIBC_ROOT)/include \
	-isystem $(GCC_BUILTIN_INC) \
	-isystem $(GCC_BUILTIN_FIXED)

TOOLCHAIN_ROOT := $(abspath $(dir $(CROSS_COMPILE))/..)
GCC_CXX_VER := $(shell $(CXX) -dumpfullversion)
LIBCXX_ROOT := $(TOOLCHAIN_ROOT)/riscv64-unknown-elf/include/c++/$(GCC_CXX_VER)
PICOLIBC_SYSINC_CXX = -nostdinc \
	-isystem $(LIBCXX_ROOT)/riscv64-unknown-elf \
	-isystem $(LIBCXX_ROOT) \
	-isystem $(PICOLIBC_ROOT)/include \
	-isystem $(GCC_BUILTIN_INC) \
	-isystem $(GCC_BUILTIN_FIXED)

COMMON_OPT = $(ARCH_FLAGS) -O2 -Wall
CFLAGS = $(COMMON_OPT) $(PICOLIBC_SYSINC_C) $(CSPECS)
CXXFLAGS = $(COMMON_OPT) $(PICOLIBC_SYSINC_CXX) -ftls-model=local-exec -std=c++17

ASFLAGS = -march=rv64imafdc -mabi=lp64d

# CXXSPECS 负责 libc 路径、libgcc、--gc-sections 等；必须 -nostartfiles，否则会把
# 「M 态」picolibc crt0 再链进来，在 OpenSBI S 态入口上会非法指令死机。
LDFLAGS = $(ARCH_FLAGS) -T os.ld $(PICOLIBC_LIB) $(CXXSPECS) -nostartfiles -Wl,-e,_start -lc -lm

MAIN_USES_STL := $(shell printf '%s\n' '#include <string>' | $(CXX) -std=c++17 -fsyntax-only -xc++ - >/dev/null 2>&1 && echo yes)
LINK := $(if $(MAIN_USES_STL),$(CXX),$(CC))

SRCS = crt0_s_entry.S crt0_s_bootstrap.c main.cpp syscall.c uart.c
OBJS := $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(OBJS:.S=.o)

all: os.bin

os.elf: $(OBJS)
	$(LINK) $(OBJS) -o $@ $(LDFLAGS)

os.bin: os.elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) -c $< -o $@

run: os.bin
	qemu-system-riscv64 -M virt -m 1G -bios default -kernel os.bin -nographic

clean:
	rm -f *.o os.elf os.bin
