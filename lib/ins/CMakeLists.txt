cmake_minimum_required(VERSION 3.20...3.27)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

project(INST
    VERSION "0.0.1"
    HOMEPAGE_URL "https://github.com/Baiyi27/RVOS.git"
    LANGUAGES CXX
)

add_library(${PROJECT_NAME} SHARED)

target_compile_options(${PROJECT_NAME} PRIVATE
    -fdiagnostics-color=always
    -ffunction-sections
    -fdata-sections
    "$<$<CONFIG:Debug>:-ggdb3;-DDEBUG>"
    $<$<CONFIG:Release,MinSizeRel>:-DNDEBUG>
)

target_include_directories(${PROJECT_NAME} PUBLIC
    inc
    ${GTKMM_INCLUDE_DIRS}
)

target_sources(${PROJECT_NAME} PRIVATE
    inc/ISA/Inst.hh
    inc/Util/Bitops.hh
    inc/Util/Regs.hh

    inc/Core/InstFactory.hh
    inc/Core/Instruction.hh
    src/Core/InstFactory.cc
    src/Core/Instruction.cc
)


add_executable(
    ${PROJECT_NAME}-test-ui
    # tests/INST.cc
    src/Gui/MainWindow.cc
)

target_link_libraries(${PROJECT_NAME}-test-ui
    PRIVATE ${PROJECT_NAME}
    PRIVATE ${GTKMM_LIBRARIES}
)