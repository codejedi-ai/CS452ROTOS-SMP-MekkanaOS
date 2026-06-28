# K1 — Kernel Primitives

CS 452 K1 spec: https://student.cs.uwaterloo.ca/~cs452/S26/assignments/k1.html

## Implementations in this folder
- `k1.c` — `k1_first_user_task()` (spec demo) + `k1_self_tests()`
- `k1.h` — exports

## Kernel source files that implement K1
Living elsewhere in the tree (not moved to preserve the build):

| File | Role |
|---|---|
| `boot.S` | EL2→EL1 transition, kernel stack, vector table install |
| `vector.S` | Exception vector table (sync + IRQ entry stubs) |
| `asm.S` | `Save` / `Begin` context switch |
| `syscall.c` / `syscall.h` | Syscall dispatcher, `Create` / `Yield` / `Exit` / `MyTid` / `MyParentTid`, scheduler |
| `processes.c` / `processes.h` | Task descriptors |
| `main.c` | `kmain` — bootstraps tasks then `Schedule()` |
| `rpi.c` / `rpi.h` | UART driver for busy-wait output |
| `linker.ld` | Image layout |

## Required syscalls demonstrated
`Create(int priority, void (*function)())`, `MyTid()`, `MyParentTid()`,
`Yield()`, `Exit()`.

## Demo
`k1_first_user_task()` creates two low-priority children, then two
high-priority children. Each child prints its tid + parent tid, yields,
prints again, and exits. Expected output order:

```
Created: <hp1_tid>
tid=<hp1> parent=<root>
Created: <hp2_tid>
tid=<hp2> parent=<root>
tid=<hp1> parent=<root>   <- after first hp1.Yield
tid=<hp2> parent=<root>
Created: <lp1_tid>
Created: <lp2_tid>
FirstUserTask: exiting
tid=<lp1> parent=<root>
…
```
