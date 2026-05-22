set(incDir ${CMAKE_SOURCE_DIR}/inc)

add_executable(${CMAKE_PROJECT_NAME})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${incDir}
    ${incDir}/Arch
    ${incDir}/HAL
    ${incDir}/Kernel
    ${CMAKE_SOURCE_DIR}/tests
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Init/init.cc
    ${CMAKE_SOURCE_DIR}/tests/trap_test.cc
)

# add_subdirectory(${CMAKE_SOURCE_DIR}/src/Arch/Trap arch_trap)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Arch)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    ${_64bitCommonOpts}
    ${specsFile}
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    ${_64bitCommonOpts}
    ${specsFile}
    --oslib=semihost

    # -nostdlib
    # -nostartfiles
    # -nodefaultlibs
    -T ${linkerPath}/qemu-virt_rv.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.map
)
SetupCodeChecks(${CMAKE_PROJECT_NAME})
RunInQEMU(${CMAKE_PROJECT_NAME})
GenerateASMFile(${CMAKE_PROJECT_NAME} "-DCS")

add_dependencies(RVOS merge_compile_commands)
