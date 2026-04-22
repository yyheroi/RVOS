#!/usr/bin/env bash
# 启动 QEMU（GDB stub :1234，-S 停在入口）并拉起 GDB。用法见 qemu_rvos.sh debug。
set -euo pipefail

if [ $# -lt 1 ]; then
  echo "用法: $0 <内核 ELF 绝对或相对路径>" >&2
  exit 1
fi

target="$(realpath "$1")"
if [ ! -f "$target" ]; then
  echo "找不到文件: $target" >&2
  exit 1
fi

pick_gdb() {
  if [ -n "${GDB:-}" ] && command -v "${GDB}" >/dev/null 2>&1; then
    printf '%s' "${GDB}"
    return
  fi
  for c in riscv64-unknown-elf-gdb riscv64-elf-gdb gdb-multiarch gdb; do
    if command -v "$c" >/dev/null 2>&1; then
      printf '%s' "$c"
      return
    fi
  done
  echo "未找到 GDB（可设置 GDB=... 或安装 gdb-multiarch / riscv64-unknown-elf-gdb）。" >&2
  exit 1
}

GDB_BIN="$(pick_gdb)"

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GDB_INIT="${HERE}/../gdb/qemu_kernel.gdb"

cleanup() {
  if [ -n "${QEMU_PID:-}" ]; then
    echo "结束 QEMU (pid ${QEMU_PID})" >&2
    kill -- "-${QEMU_PID}" 2>/dev/null || true
    wait "${QEMU_PID}" 2>/dev/null || true
  fi
}

trap cleanup EXIT INT TERM

GDB_HOST="${GDB_HOST:-127.0.0.1}"
GDB_PORT="${GDB_PORT:-1234}"

qemu-system-riscv64 \
  -nographic -smp 1 -machine virt -bios none \
  -serial mon:stdio -no-reboot \
  -kernel "${target}" -s -S &

QEMU_PID=$!

if ! kill -0 "${QEMU_PID}" 2>/dev/null; then
  echo "QEMU 未能保持运行（请查看上方 QEMU 报错）。" >&2
  exit 1
fi

wait_gdb_port() {
  local n=0
  while [ "${n}" -lt 80 ]; do
    if command -v nc >/dev/null 2>&1 && nc -z "${GDB_HOST}" "${GDB_PORT}" 2>/dev/null; then
      return 0
    fi
    if (exec 3<>"/dev/tcp/${GDB_HOST}/${GDB_PORT}") 2>/dev/null; then
      exec 3<&- 3>&-
      return 0
    fi
    if ! kill -0 "${QEMU_PID}" 2>/dev/null; then
      echo "QEMU 已退出，无法连接 GDB stub。" >&2
      return 1
    fi
    sleep 0.05
    n=$((n + 1))
  done
  echo "等待 ${GDB_HOST}:${GDB_PORT} 超时：请确认本机无防火墙拦截，且未占用该端口。" >&2
  return 1
}

wait_gdb_port || exit 1

echo "QEMU 已启动 (pid ${QEMU_PID})，GDB stub ${GDB_HOST}:${GDB_PORT}；使用 GDB: ${GDB_BIN}"
echo "说明：target remote 后若 PC≈0x1000 且显示 ??，属正常（QEMU 复位/ROM 区，无调试符号）。"
echo "      执行 continue 后应命中 _start / KernelInit；若长时间不停，在 GDB 里 Ctrl+C 再 info reg pc / x/8i \$pc。"
echo "      本内核 ELF 入口在 0x8000xxxx（readelf -h Entry）；C++ 可断 KernelInit（os_start.S 会 call）。"
echo "      重要：GDB 里不要用 run (r) / start，只使用 target remote + continue / stepi（否则会在本机误跑 RISC-V ELF）。"
echo "      若曾关掉 QEMU，须重新运行本脚本再起 QEMU，否则 target remote 会超时/拒绝。"

# gdb-multiarch 需显式架构；riscv 工具链 gdb 一般已带好
init_ex=()
if [[ "${GDB_BIN}" == *multiarch* ]] || [[ "${GDB_BIN}" == gdb ]]; then
  init_ex+=(-ex "set architecture riscv:rv64")
fi

exec "${GDB_BIN}" -q -x "${GDB_INIT}" "${target}" \
  "${init_ex[@]}" \
  -ex "set pagination off" \
  -ex "target remote ${GDB_HOST}:${GDB_PORT}" \
  -ex "break _start" \
  -ex "break KernelInit" \
  -ex "continue"
