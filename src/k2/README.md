# K2 — Synchronous Messaging + Nameserver

## Implementations in this folder
- `k2.c` / `k2.h` — `k2_self_tests()` (Send/Receive ping-pong, nameserver round-trip, rock-paper-scissors)

## Kernel source files that implement K2
| File | Role |
|---|---|
| `syscall.c` | `Send`, `Receive`, `Reply` cases + `send_helper`, `recieve_helper`, `reply_helper` |
| `nameserver.c` / `nameserver.h` | Nameserver task + client-side `RegisterAs` / `WhoIs` |

## Tests
- `k2_test_send_receive` — child receives, adds 100, replies; parent verifies result.
- `k2_test_nameserver` — `RegisterAs("k2_test_owner")` + `WhoIs` round-trip.
- `k2_test_rps` — three players Send to one referee that Receives all three.
