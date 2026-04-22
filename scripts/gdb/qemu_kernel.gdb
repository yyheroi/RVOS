# RVOS：通过 QEMU GDB stub 调试时，不要用 run/start（会在 x86 宿主上启动 RISC-V ELF）。
# 下面覆盖 run / start，避免误按 r 导致架构冲突或 GDB 崩溃。正确：target remote 127.0.0.1:1234，再 c / si。

set confirm off

define run
  echo \n[RVOS] 已禁止 run。请执行：target remote 127.0.0.1:1234\n  然后 continue (c)、stepi (si)、break 等。\n
end

define start
  echo \n[RVOS] 已禁止 start。请执行：target remote 127.0.0.1:1234\n  然后 continue / stepi。\n
end

set confirm on
