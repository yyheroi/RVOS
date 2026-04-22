cmake_minimum_required(VERSION 3.20...3.27)

# 裸机 + picolibc：try_compile 若链接可执行文件，不会带 --specs / picolibc.ld，crt0 会缺 __data_*、__stack 等。
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(multipleProjects LANGUAGES CXX C ASM)

include(ExternalProject)

set(rootPath ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "project root directory")
set(rootBuilddir ${CMAKE_BINARY_DIR} CACHE PATH "project root builddir")
set(customModulesPath ${rootPath}/cmake/Modules CACHE PATH "custom modules directory")
list(APPEND CMAKE_MODULE_PATH "${customModulesPath}")
include(CodeCheck)
include(CustomTarget)
include(CheckLibstdcxxRiscvMedany)

set(ldLinkPath ${CMAKE_CURRENT_LIST_DIR}/Startup CACHE PATH "Linker script directory")

set(subProjectPath ${CMAKE_SOURCE_DIR}/lib)
set(RVOS_builddir ${CMAKE_BINARY_DIR}/build-RVOS)
set(INST_builddir ${CMAKE_BINARY_DIR}/build-INST)

set(INST_builddir ${CMAKE_BINARY_DIR}/build-INST)

# ExternalProject_Add(
# INST
# SOURCE_DIR ${subProjectPath}
# BINARY_DIR ${INST_builddir}

# CMAKE_ARGS
# -DCMAKE_MODULE_PATH=${customModulesPath}
# -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
# -DrootBuilddir=${rootBuilddir}

# # -DrootPath=${rootPath}
# INSTALL_COMMAND ""
# )
add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# add_subdirectory(asm)
target_include_directories(${PROJECT_NAME} PRIVATE
    inc
)

target_sources(${PROJECT_NAME} PRIVATE

    ${ldLinkPath}/qemu_boot_trampoline.S
    ${ldLinkPath}/os_start.S
    src/Lib/syscalls.c

    # src/Lib/sysmem.c
    # src/Lib/Runtime.cc
    src/Kernel.cc
    # src/Drivers/uart.c

    inc/Drivers/Peripheral.hh
    src/Drivers/Peripheral.cc
    inc/Drivers/Uart.hh
    src/Drivers/Uart.cc
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:--specs=picolibcpp.specs>
    $<$<COMPILE_LANGUAGE:CXX>:--specs=picolibcpp.specs>
    -march=rv64imacv
    -mabi=lp64d
    -mcmodel=medany
    -msmall-data-limit=0
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

# 链接到 0x80000000 时，medlow 的 libstdc++.a（R_RISCV_HI20）会失败；须 medany 的 libstdc++。configure 时由 CheckLibstdcxxRiscvMedany.cmake 检测。
target_link_options(${PROJECT_NAME} PRIVATE
    -nostartfiles
    --specs=picolibcpp.specs
    # -nostdlib++
    # -nodefaultlibs

    # --specs=nano.specs
    # --specs=nosys.specs
    -march=rv64imacv
    -mabi=lp64d
    -mcmodel=medany

    # -lstdc++
    # -lsupc++
    # -lc
    # -lgcc
    # -lm

    # -T ${ldLinkPath}/_os_start.ld
    -T ${ldLinkPath}/qemu-virt.ld
    -Wl,-e,rvos_qemu_entry
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    -Wl,--no-warn-rwx-segments
)
SetupCodeChecks(${PROJECT_NAME})

DefineCodeChecksTargets("fmt" "tidy")

get_filename_component(builddir ${CMAKE_BINARY_DIR} NAME)
if(ClangFormat_FOUND OR  ClangTidy_FOUND)
    message(">> Optional: build the target 'tidy' or 'fmt' ('cmake --build ${builddir} -t help' to know more) before building 'all' target")
endif()

MergeCompileCmdFiles(${CMAKE_BINARY_DIR} ${INST_builddir})
add_dependencies(${PROJECT_NAME} merge_compile_commands)
RunInQEMU(${PROJECT_NAME})
GenerateASMFile(${PROJECT_NAME} "-DCS")

# add_dependencies(INST merge_compile_commands)
