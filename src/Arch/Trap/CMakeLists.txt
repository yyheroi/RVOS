target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/trap.cc
    ${CMAKE_CURRENT_LIST_DIR}/trap_entry.S
    ${CMAKE_SOURCE_DIR}/src/Arch/CSR/CsrManager.cc
)
