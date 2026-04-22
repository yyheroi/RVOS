#!/usr/bin/env bash
# 在 QEMU riscv64 virt 上运行本工程内核（与 cmake 目标 run-multipleProjects 参数一致）。
#
# 用法：
#   ./scripts/shell/run_qemu.sh                    # 默认 build-x86_64/multipleProjects
#   BUILD_DIR=build-x86_64 ./scripts/shell/run_qemu.sh
#   ./scripts/shell/run_qemu.sh /path/to/multipleProjects
#
# 依赖：qemu-system-riscv64（apt: qemu-system-misc 或 qemu-system-riscv）
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-build-x86_64}"
DEFAULT_ELF="$ROOT/$BUILD_DIR/multipleProjects"

usage() {
  sed -n '1,20p' "$0" | tail -n +2
  exit "${1:-0}"
}

case "${1:-}" in
  -h|--help) usage 0 ;;
esac

ELF="${1:-$DEFAULT_ELF}"
if [ ! -f "$ELF" ]; then
  echo "找不到内核 ELF: $ELF" >&2
  echo "请先编译：./build.sh 或 cmake --build $BUILD_DIR" >&2
  exit 1
fi

if ! command -v qemu-system-riscv64 >/dev/null 2>&1; then
  echo "未找到 qemu-system-riscv64，请安装 qemu-system-riscv / qemu-system-misc。" >&2
  exit 1
fi

echo "QEMU kernel: $ELF"
exec qemu-system-riscv64 \
  -nographic \
  -smp 1 \
  -machine virt \
  -bios none \
  -serial mon:stdio \
  -no-reboot \
  -kernel "$ELF"
