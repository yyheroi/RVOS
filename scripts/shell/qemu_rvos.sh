#!/usr/bin/env bash
#
# RVOS：在 QEMU riscv64 virt 上运行 / 调试 / 指令 trace
#
# 用法：
#   ./scripts/shell/qemu_rvos.sh run              # 直接跑（串口输出到当前终端）
#   ./scripts/shell/qemu_rvos.sh debug            # QEMU 等 GDB，并自动连上（需 medany libstdc++ 编过内核）
#   ./scripts/shell/qemu_rvos.sh trace            # 将反汇编 trace 写入 qemu-in_asm.log
#   BUILD_DIR=build-x86_64 ./scripts/shell/qemu_rvos.sh run
#   ./scripts/shell/qemu_rvos.sh run /path/to/multipleProjects
#
# 依赖：qemu-system-riscv64；debug 还需 gdb-multiarch 或 riscv64-unknown-elf-gdb
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-build-x86_64}"
DEFAULT_ELF="$ROOT/$BUILD_DIR/multipleProjects"

usage() {
  cat <<'EOF'
用法: qemu_rvos.sh <子命令> [ELF路径]

子命令:
  run    正常运行（Ctrl+A X 退出 QEMU nographic）
  debug  QEMU -s -S，并启动 GDB（断点 _start、KernelInit；GDB 内勿用 run，用 continue/stepi）
  trace  记录 in_asm 到 BUILD_DIR/qemu-in_asm.log（量很大，仅短时排障）

环境变量:
  BUILD_DIR   构建目录（默认 build-x86_64）
  GDB         调试器路径（可选）

与 CMake 等价:
  cmake --build build-x86_64 -t run-multipleProjects
  cmake --build build-x86_64 -t dbg-multipleProjects
EOF
  exit "${1:-0}"
}

qemu_base() {
  local elf="$1"
  shift
  exec qemu-system-riscv64 \
    -nographic \
    -smp 1 \
    -machine virt \
    -bios none \
    -serial mon:stdio \
    -no-reboot \
    -kernel "$elf" \
    "$@"
}

case "${1:-}" in
  -h|--help|help) usage 0 ;;
esac

CMD="${1:-run}"
shift || true

ELF="${1:-$DEFAULT_ELF}"
if [ ! -f "$ELF" ]; then
  echo "找不到内核 ELF: $ELF" >&2
  echo "请先: ./build.sh 或 cmake --build $BUILD_DIR" >&2
  exit 1
fi
ELF="$(realpath "$ELF")"

if ! command -v qemu-system-riscv64 >/dev/null 2>&1; then
  echo "未安装 qemu-system-riscv64（如: sudo apt install qemu-system-riscv）" >&2
  exit 1
fi

case "$CMD" in
  run)
    echo "ELF: $ELF"
    echo "退出: 在 QEMU 监视器按 Ctrl+A 松开后按 X；或另开终端 kill qemu"
    qemu_base "$ELF"
    ;;
  debug)
    echo "ELF: $ELF"
    bash "$(dirname "$0")/qemu_debug.sh" "$ELF"
    ;;
  trace)
    LOG="$ROOT/$BUILD_DIR/qemu-in_asm.log"
    echo "ELF: $ELF"
    echo "指令 trace 写入: $LOG （文件会很大，用 Ctrl+C 停 QEMU）"
    qemu_base "$ELF" -d in_asm 2>"$LOG"
    ;;
  *)
    echo "未知子命令: $CMD" >&2
    usage 1
    ;;
esac
