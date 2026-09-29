cmake_minimum_required(VERSION 3.20...3.27)

project(multipleProjects)

include(ExternalProject)

set(rootPath ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "project root directory")
set(rootBuilddir ${CMAKE_BINARY_DIR} CACHE PATH "project root builddir")
set(customModulesPath ${rootPath}/cmake/Modules CACHE PATH "custom modules directory")

set(subProjectPath ${CMAKE_SOURCE_DIR}/lib)
set(RVOS_builddir ${CMAKE_BINARY_DIR}/build-RVOS)
set(INST_builddir ${CMAKE_BINARY_DIR}/build-INST)

# RVOS is cross-compiled with the riscv64-elf toolchain; turn this OFF on hosts
# where that toolchain is missing so the host-built INST subproject still builds.
option(buildRVOS "Build the cross-compiled (riscv64-elf) RVOS subproject" ON)

list(APPEND CMAKE_MODULE_PATH "${customModulesPath}")
include(CustomTarget)

if(buildRVOS)
    ExternalProject_Add(
        RVOS
        SOURCE_DIR ${subProjectPath}/os
        BINARY_DIR ${RVOS_builddir}
        CMAKE_ARGS
        -DCMAKE_C_COMPILER=riscv64-elf-gcc
        -DCMAKE_CXX_COMPILER=riscv64-elf-g++
        -DCMAKE_MODULE_PATH=${customModulesPath}
        -DrootPath=${rootPath}
        -DrootBuilddir=${rootBuilddir}
        INSTALL_COMMAND ""
    )
endif()

ExternalProject_Add(
    INST
    SOURCE_DIR ${subProjectPath}/ins
    BINARY_DIR ${INST_builddir}
    CMAKE_ARGS
    #-DCMAKE_CXX_COMPILER=clang++
    -DCMAKE_MODULE_PATH=${customModulesPath}
    -DrootPath=${rootPath}
    -DrootBuilddir=${rootBuilddir}
    INSTALL_COMMAND ""
)

if(buildRVOS)
    MergeCompileCmdFiles(${RVOS_builddir} ${INST_builddir})
    add_dependencies(RVOS merge_compile_commands)
else()
    MergeCompileCmdFiles(${INST_builddir})
endif()
add_dependencies(INST merge_compile_commands)
