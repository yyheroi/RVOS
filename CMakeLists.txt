cmake_minimum_required(VERSION 3.20...3.27)

project(multipleProjects)

include(ExternalProject)

set(rootPath ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "project root directory")
set(rootBuilddir ${CMAKE_BINARY_DIR} CACHE PATH "project root builddir")
set(customModulesPath ${rootPath}/cmake/Modules CACHE PATH "custom modules directory")

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

# add_subdirectory(asm)
target_include_directories(${PROJECT_NAME} PRIVATE
    inc
)

target_sources(${PROJECT_NAME} PRIVATE

    ${ldLinkPath}/os_start.S
    # src/Lib/syscalls.c

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
    -march=rv64imacv
    -mabi=lp64d
    -mcmodel=medany
    -msmall-data-limit=0
    -D__PICOLIBC_NO_THREADS=1
    -fPIC
    # -mcmodel=large
)

target_link_options(${PROJECT_NAME} PRIVATE

    # -nostdlib
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
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
)
SetupCodeChecks(${PROJECT_NAME})

DefineCodeChecksTargets("fmt" "tidy")

get_filename_component(builddir ${CMAKE_BINARY_DIR} NAME)
if(ClangFormat_FOUND OR  ClangTidy_FOUND)
    message(">> Optional: build the target 'tidy' or 'fmt' ('cmake --build ${builddir} -t help' to know more) before building 'all' target")
endif()

MergeCompileCmdFiles(${CMAKE_BINARY_DIR} ${INST_builddir})
add_dependencies(RVOS merge_compile_commands)
RunInQEMU(${PROJECT_NAME})
GenerateASMFile(${PROJECT_NAME} "-DCS")

# add_dependencies(INST merge_compile_commands)
