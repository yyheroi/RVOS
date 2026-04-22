#!/usr/bin/env bash
# 本工程需要：picolibc（picolibcpp.specs）+ 带 libstdc++ 头文件的 riscv64-unknown-elf-g++。
# Ubuntu 的 gcc-riscv64-unknown-elf 常禁用 libstdc++-v3，即使用 picolibc 也无法 #include <cstdint>。
# 推荐：在自建的 RISC-V 前缀上安装 picolibc（与 libstdc++ 同源），不要用 /usr 与 /opt 混链。
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

BUILD_DIR="${BUILD_DIR:-build-x86_64}"

# 可通过 RISCV_PREFIX 指定工具链安装根目录（含 bin/riscv64-unknown-elf-g++）。
# 未设置时：依次尝试新盘、/opt/riscv，并跳过「仅有 binutils、无 libstdc++ 头文件」的半成品前缀。
pick_riscv_prefix() {
  if [ -n "${RISCV_PREFIX:-}" ]; then
    if [ -x "$RISCV_PREFIX/bin/riscv64-unknown-elf-g++" ]; then
      printf '%s' "$RISCV_PREFIX"
      return 0
    fi
    echo "环境变量 RISCV_PREFIX=$RISCV_PREFIX 下未找到 bin/riscv64-unknown-elf-g++。" >&2
    return 1
  fi
  local p
  for p in /mnt/riscv-ssd/opt/riscv /opt/riscv; do
    if [ -x "$p/bin/riscv64-unknown-elf-g++" ] && have_libstdcxx_headers "$p/bin/riscv64-unknown-elf-g++"; then
      printf '%s' "$p"
      return 0
    fi
  done
  return 1
}

# 半成品工具链常只有 stage1 / binutils，g++ 找不到 <cstdint>；自动探测时必须跳过。
have_libstdcxx_headers() {
  local cxx="$1"
  echo '#include <cstdint>' | "$cxx" -fsyntax-only -xc++ - 2>/dev/null
}

RISCV_PREFIX="$(pick_riscv_prefix)" || {
  echo "无法确定可用的工具链前缀。请安装 riscv64-unknown-elf-g++，或设置正确的 RISCV_PREFIX（例如 /opt/riscv、/mnt/riscv-ssd/opt/riscv）。" >&2
  exit 1
}

OPT_CXX="$RISCV_PREFIX/bin/riscv64-unknown-elf-g++"
OPT_CC="$RISCV_PREFIX/bin/riscv64-unknown-elf-gcc"
if [ ! -x "$OPT_CXX" ] || [ ! -x "$OPT_CC" ]; then
  echo "RISCV_PREFIX=$RISCV_PREFIX 下缺少可执行的 riscv64-unknown-elf-g++ / gcc。" >&2
  exit 1
fi

# GCC 对 -print-file-name=picolibcpp.specs 常输出相对名「picolibcpp.specs」，须相对 libgcc.a 所在目录解析。
have_picolib_specs() {
  local cxx="$1"
  local s libgcc_dir
  s="$("$cxx" -print-file-name=picolibcpp.specs 2>/dev/null | tr -d '\n')"
  [ -n "$s" ] || return 1
  case "$s" in
    /*) ;;
    *)
      libgcc_dir="$(dirname "$("$cxx" -print-file-name=libgcc.a 2>/dev/null | tr -d '\n')")"
      s="${libgcc_dir}/$s"
      ;;
  esac
  [ -f "$s" ]
}

if ! have_picolib_specs "$OPT_CXX"; then
  cat >&2 <<EOF
未在「$RISCV_PREFIX」所对应的 GCC 目录中找到可用的 picolibcpp.specs（picolibc 未装入该工具链版本）。

当前编译器：$OPT_CXX
libgcc 目录：$(dirname "$("$OPT_CXX" -print-file-name=libgcc.a 2>/dev/null | tr -d '\n')")

请任选其一：
  1) 用 picolibc 官方方式装到当前前缀（sysroot-install），使下面文件存在：
       \$(dirname "\$($OPT_CXX -print-file-name=libgcc.a)")/picolibcpp.specs
     参考：https://github.com/picolibc/picolibc/blob/main/doc/getting-started.md
  2) 从源码构建 riscv-gnu-toolchain（或 riscv-toolchain-picolibc-build），--enable-picolibc，再安装到同一前缀。

说明：Makefile 里 CFLAGS=--oslib=semihost 是传给「已集成 picolibc 的 gcc」的选项；若未装 picolibc，
     仅有 --oslib 也不会出现 picolibcpp.specs。

配置示例（装好 picolibc 之后）：
  cmake -DCMAKE_C_COMPILER=$OPT_CC \\
        -DCMAKE_CXX_COMPILER=$OPT_CXX \\
        -DCMAKE_ASM_COMPILER=$OPT_CC \\
        -S . -B $BUILD_DIR
  cmake --build $BUILD_DIR

也可显式指定前缀再执行本脚本：
  RISCV_PREFIX=/opt/riscv ./build.sh
EOF
  exit 1
fi

SPECS_RAW="$("$OPT_CXX" -print-file-name=picolibcpp.specs | tr -d '\n')"
case "$SPECS_RAW" in
  /*) SPECS_PATH="$SPECS_RAW" ;;
  *) SPECS_PATH="$(dirname "$("$OPT_CXX" -print-file-name=libgcc.a | tr -d '\n')")/$SPECS_RAW" ;;
esac
echo "使用 $OPT_CXX（RISCV_PREFIX=$RISCV_PREFIX，picolibc: $SPECS_PATH）"
echo "提示：RV64 下固件链到 0x80000000 时，libstdc++.a 须为 medany（重定位多为 R_RISCV_PCREL_HI20）；"
echo "      若为 medlow（R_RISCV_HI20），会在链接时报 relocation truncated。configure 阶段会检测；修复见 cmake/Modules/CheckLibstdcxxRiscvMedany.cmake 中的说明。"
rm -rf "$BUILD_DIR"
cmake \
  -DCMAKE_C_COMPILER="$OPT_CC" \
  -DCMAKE_CXX_COMPILER="$OPT_CXX" \
  -DCMAKE_ASM_COMPILER="$OPT_CC" \
  -S . -B "$BUILD_DIR" \
  "$@"

cmake --build "$BUILD_DIR"
