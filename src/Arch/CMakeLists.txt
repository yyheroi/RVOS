
set(module Arch)
set(archIncDir ${CMAKE_SOURCE_DIR}/inc/Arch)

set(bootPath ${CMAKE_CURRENT_LIST_DIR}/Boot CACHE PATH "boot entry directory")
set(linkerPath ${CMAKE_CURRENT_LIST_DIR}/Linker CACHE PATH "Linker script directory")

set(ARCH_SOURCES
    Trap/trap.cc
    Trap/trap_entry.S
    CSR/CsrManager.cc
)

add_library(${module} STATIC ${ARCH_SOURCES})

target_include_directories(${module} PUBLIC ${archIncDir})

target_compile_options(${module} PRIVATE
    ${_64bitCommonOpts}
    ${specsFile}
)
