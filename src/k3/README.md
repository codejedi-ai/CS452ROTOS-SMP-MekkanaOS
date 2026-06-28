# K3 — Clock Server + Interrupts

## Implementations in this folder
- `k3.c` / `k3.h` — `k3_self_tests()` (`Time` + `Delay(5)` regression)

## Kernel source files that implement K3
| File | Role |
|---|---|
| `clockserver.c` / `clockserver.h` | Clock server task + `Time` / `Delay` / `DelayUntil` clients |
| `gic.c` / `gic.h` | GIC-400 setup, IRQ routing, IAR/EOI helpers |
| `systimer.c` / `systimer.h` | BCM2711 system timer registers (CLO/CHI/Cn/CS) |
| `syscall.c` | `AwaitEvent` syscall, `HandleASYNC` IRQ entry, `ExceptionASYNC` dispatcher |
| `vector.S` | `el1_a64_irq` vector |

## Hardware reference (from CS452 Lec 08)
- C1 interrupt → InterruptID **97**
- C3 interrupt → InterruptID **99** (used here as `CLOCKINTID`)
- C0 and C2 are reserved by VC; use C1 and/or C3
- GIC base `0xFF840000`, GICD `+0x1000`, GICC `+0x2000`

## QEMU vs real hardware

On real Pi 4 the BCM2711 system timer's compare-3 match raises an SPI
that reaches GIC-400 → CPU IRQ → `el1_a64_irq` → `HandleASYNC` → `unblock_return(CLOCKINTID, 1)`.

Under `qemu-system-aarch64 -machine raspi4b`, the system-timer compare
outputs are not wired through to GIC-400 SPIs the same way: the M3 bit
in `SYSTIME_CS` does set when `CLO` catches up to `C3`, but the GIC
never sees an asserted SPI, so `AwaitEvent(CLOCKINTID)` blocks forever
and `Delay()` hangs.

### Workaround in this kernel
`clock_notifier` polls `get_timerLO()` (which QEMU **does** model) and
sends tick messages whenever the counter crosses a `next_tick = prev +
TICK_US` deadline. Each iteration yields so other tasks get the CPU.
Works on both QEMU and real hardware (it's just less efficient than
IRQ-driven on real HW because of the polling). To re-IRQ-ify on real
hardware, swap the polling loop back to a single `AwaitEvent(CLOCKINTID)`
call.

### Notifier priority
`clock_notifier` is created at **priority 1** in `main.c` — same as the
test runner. With Yields between polls, the runner still preempts on
prio-0 server messages and the priority pair runs in lock-step.

## Tests
- `k3_test_time_delay` — `WhoIs("clock_server")` → `Time()` → `Delay(5)` →
  `Time()`. Verifies the second timestamp is at least 5 ticks past the first.
