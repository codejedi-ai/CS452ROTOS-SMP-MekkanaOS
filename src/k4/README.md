# K4 — IO Server + Marklin

## Implementations in this folder
- `k4.c` / `k4.h` — `k4_self_tests()` (display server reachability check)

## Kernel source files that implement K4
| File | Role |
|---|---|
| `ioserver.c` / `ioserver.h` | `io_notifier`, `io_TXIC_*_server`, `io_RXIC_*_server`, `io_CTS_*_server` + `Putc` / `Getc` clients |
| `auxuart.c` / `auxuart.h` | BCM2835 AUX mini-UART driver (QEMU Marklin transport) |
| `ui/display_server.c` / `ui/display_server.h` | Serializes user-mode UART output |
| `tc1/marklin_worker.c` | Marklin protocol bridge (uses `MK_PUTC` / `MK_GETC`) |
| `config.h` | `MARKLIN_HW_UART3` build mode (PL011 UART3 vs AUX) |

## Tests
- `k4_test_display_server` — `WhoIs("display")` + `display_puts` round-trip.

## Bigger demo
The interactive shell (`ui/shell.c`) launched after `boot_test_runner` exits
exposes the K4-era commands: `tr <id> <speed>`, `sw <id> <S|C>`, `rv <id>`,
`stopa <id> <sensor>`. These drive Marklin via the IO server -> AUX UART ->
MarklinSim (QEMU build) or PL011 UART3 (real hardware).
