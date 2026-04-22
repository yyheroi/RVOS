# RVOS 链接脚本将 FLASH 放在 0x80000000。RV64 下 medlow 的 libstdc++.a 使用 R_RISCV_HI20，
# 链接器拒绝解析 VA >= 0x7ffff800 的符号，表现为 relocation truncated to fit: R_RISCV_HI20。
# 须使用以 -mcmodel=medany 构建的 libstdc++.a（重定位多为 R_RISCV_PCREL_HI20）。

if(RVOS_SKIP_LIBSTDCXX_MODEL_CHECK)
  return()
endif()
if(NOT CMAKE_CXX_COMPILER MATCHES "riscv64-unknown-elf")
  return()
endif()

execute_process(
  COMMAND "${CMAKE_CXX_COMPILER}" -print-file-name=libstdc++.a
  OUTPUT_VARIABLE _rvos_libstdcxx_a
  OUTPUT_STRIP_TRAILING_WHITESPACE
  COMMAND_ERROR_IS_FATAL ANY
)
if(NOT IS_ABSOLUTE "${_rvos_libstdcxx_a}")
  cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH _rvos_gxx_bin)
  set(_rvos_libstdcxx_a "${_rvos_gxx_bin}/${_rvos_libstdcxx_a}")
endif()
if(NOT EXISTS "${_rvos_libstdcxx_a}")
  message(WARNING "找不到 libstdc++.a（${_rvos_libstdcxx_a}），跳过 medany 检查。")
  return()
endif()

set(_rvos_chk "${CMAKE_BINARY_DIR}/CMakeFiles/rvos-libstdcxx-model")
file(MAKE_DIRECTORY "${_rvos_chk}")
file(REMOVE "${_rvos_chk}/assert_fail.o")
execute_process(
  COMMAND ar x "${_rvos_libstdcxx_a}" assert_fail.o
  WORKING_DIRECTORY "${_rvos_chk}"
  RESULT_VARIABLE _rvos_ar
)
if(NOT _rvos_ar EQUAL 0 OR NOT EXISTS "${_rvos_chk}/assert_fail.o")
  message(WARNING "无法从 libstdc++.a 解出 assert_fail.o，跳过 medany 检查。")
  return()
endif()

cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH _rvos_gxx_bin)
set(_rvos_readelf "${_rvos_gxx_bin}/riscv64-unknown-elf-readelf")
if(NOT EXISTS "${_rvos_readelf}")
  set(_rvos_readelf readelf)
endif()

execute_process(
  COMMAND /bin/sh -c "\"${_rvos_readelf}\" -r \"${_rvos_chk}/assert_fail.o\" | grep 'R_RISCV_HI20' | grep -v PCREL | grep -q ."
  RESULT_VARIABLE _rvos_is_medlow
)

if(_rvos_is_medlow EQUAL 0)
  message(FATAL_ERROR
    "当前 \"${_rvos_libstdcxx_a}\" 中的 libstdc++ 为 medlow 重定位（assert_fail.o 含 R_RISCV_HI20），\n"
    "无法与链接到 0x80000000 的固件一起链接（R_RISCV_HI20 relocation truncated）。\n\n"
    "请用 -mcmodel=medany 重新编译并安装 target libstdc++，例如在你构建 riscv-gnu-toolchain 的目录\n"
    "（stage2 目录名可能是 build-gcc-newlib-stage2、build-gcc-picolibc-stage2 等）：\n"
    "  make -C <stage2-dir> all-target-libstdc++-v3 \\\n"
    "    CXXFLAGS_FOR_TARGET='-Os -mcmodel=medany'\n"
    "  make -C <stage2-dir> install-target-libstdc++-v3\n\n"
    "(路径以你本机为准；须与当前 GCC 版本一致。)\n"
    "若需跳过此检查：cmake -DRVOS_SKIP_LIBSTDCXX_MODEL_CHECK=ON ...")
endif()
