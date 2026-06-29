# SMP Roadmap — All CS452 ROTOS Kernels

Target architecture: [SMP_MULTIKERNEL_IPC.md](SMP_MULTIKERNEL_IPC.md).
> **SmpOS repo:** `CS452ROTOS-APU-SmpOS` is an **APU-line** repository that houses **Phase A0** SMP multikernel work (core 0 only today; full SMP target before SMP-line ports).

## Scope decision (active focus)

**SMP work targets only kernels that are core-0-only today** — no APU workers, no secondary-core job loops.

| Priority | Why |
|----------|-----|
| **1 — APU-SmpOS (SmpOS)** | **Least developed** in the family (layer0–2, `layer1-thread`); greenfield multikernel without tearing out APU |
| **2 — DarcyOS** | Canonical single-core K-line; first production SMP port after SmpOS proves P1–P4 |
| **3 — IrisOS, MekkanaOS, PrimeOS, TRAINS** | Same single-core model; merge from DarcyOS |
| **— deferred — APU-line** | KatarOS, NyxOS, AtariOS — **not in initial SMP focus** (different multi-core model; P8+ after SMP-line SMP stable) |

```text
Phase A (NOW):  APU-SmpOS (SmpOS)  →  DarcyOS  →  SMP-line mirrors
Phase B (LATER): APU-line ports (replace accel workers with multikernel)
```

---

## Single-core kernels (SMP focus set)

| Kernel | Repo | K depth today | SMP priority | Notes |
|--------|------|---------------|--------------|-------|
| **SmpOS** | `CS452ROTOS-APU-SmpOS` | layer0–2 | **P0 lead** | Least developed; implement SMP here first |
| **DarcyOS** | `CS452ROTOS-SMP-DarcyOS` | k0–k4 + tc1 | **P1** | Canonical; `BUILD_SMP=0` default |
| **IrisOS** | `CS452ROTOS-SMP-IrisOS` | k0–k4 + tc1 | P2 | Track DarcyOS |
| **MekkanaOS** | `CS452ROTOS-SMP-MekkanaOS` | k0–k4 + tc1 | P2 | + boot SMP smoke |
| **PrimeOS** | `CS452ROTOS-SMP-PrimeOS` | k0–k4 + tc1 | P3 | Mirror; course SMP off |
| **TRAINS** | `uwaterloo_…-cs452-trains` | k0–k4 + tc1 | P3 | GitLab hand-in |

## Out of scope (initial SMP program)

| Kernel | Repo | Why deferred |
|--------|------|--------------|
| KatarOS | `CS452ROTOS-APU-KatarOS` | Cores 1–3 = **APU workers**, not idle — migration is a separate program |
| NyxOS | `CS452ROTOS-APU-NyxOS` | Same |
| AtariOS | `CS452ROTOS-APU-AtariOS` | Same |

---

## Summary table (all kernels)

| Kernel | Repo | Core model | SMP phase | Depends on |
|--------|------|------------|-----------|------------|
| **SmpOS** | `CS452ROTOS-APU-SmpOS` | Core 0 only (today) | **A0 lead** | — |
| **DarcyOS** | `CS452ROTOS-SMP-DarcyOS` | Single-core | A1 | SmpOS P3–P4 |
| **IrisOS** | `CS452ROTOS-SMP-IrisOS` | Single-core | A2 | DarcyOS SMP |
| **MekkanaOS** | `CS452ROTOS-SMP-MekkanaOS` | Single-core | A2 | DarcyOS SMP |
| **PrimeOS / TRAINS** | PrimeOS repos | Single-core | A3 | DarcyOS SMP |
| KatarOS | `CS452ROTOS-APU-KatarOS` | APU workers | **B1** (deferred) | SMP-line SMP stable |
| NyxOS | `CS452ROTOS-APU-NyxOS` | APU workers | B2 | APU-KatarOS |
| AtariOS | `CS452ROTOS-APU-AtariOS` | APU workers | B3 | APU-KatarOS |
| PLATFORM | `CS452ROTOS-PLATFORM` | — | A0 tooling | — |

---

## Phase A — Single-core SMP program

### A0 — APU-SmpOS (SmpOS) (least developed, **primary focus**)

**Current:** `layer0-assembly`, `layer1-thread`, `layer2-messaging` only. One scheduler, core 0 only. No UART servers, no TC1, no APU.

**Strategy:** Grow SMP on this minimal tree first — no APU removal, no NIX layer port required for first `CoreSend` proof.

| Milestone | Deliverable |
|-----------|-------------|
| M1 | Boot cores 1–3 into per-core `kmain` (`layer0-assembly/cores.S`) |
| M2 | Per-core scheduler stub + LNS on each core |
| M3 | Shared `CoreMailbox` region + spinlocks (`layer1-thread/smp/` or `layer3-services/smp/`) |
| M4 | GIC SGI **IPI** handler in `layer1-thread/syscall.c` |
| M5 | `CoreSend` / `CoreReceive` — cross-core only; **`Send`/`Receive` unchanged** locally |
| M6 | CNS + global `WhoIs` across cores |
| M7 | QEMU `raspi4b` test: two tasks on two cores exchange one message |

**Optional (after M5):** cherry-pick **IRQ UART from DarcyOS** (SMP-line single-core reference), not from APU-line — only if SMP tests need a shell.

**Success criteria:** `CoreSend` works on the **smallest** kernel in the family before touching DarcyOS.

---

### A1 — DarcyOS (canonical single-core K-line)

| Step | Action |
|------|--------|
| 1 | Import proven `smp/` + `core_notifier` from APU-SmpOS |
| 2 | `BUILD_SMP=0` (default) — unchanged production binary |
| 3 | `BUILD_SMP=1` — 4-core multikernel image |
| 4 | `test-smp` QEMU target in `src/Makefile` |
| 5 | TC1 stays on core 0 until affinity documented |

---

### A2 — IrisOS + MekkanaOS

Merge DarcyOS SMP behind `BUILD_SMP=1`. MekkanaOS: optional SMP line in `boot_test_runner`.

---

### A3 — PrimeOS + TRAINS

Sync from DarcyOS on tagged release. **Course hand-in remains `BUILD_SMP=0`.**

---

## Phase B — APU-line (deferred, not current focus)

Only after Phase A checklist passes on **DarcyOS + one mirror**.

APU repos must **disable** `secondary_worker_c` / APUServer when `BUILD_SMP=1` and run four full kernels instead. See archived notes in git history; do not start until SMP-line SMP is stable.

---

## Timeline (revised)

```text
2026 Q2  Phase A0: SMP-SmpOS M1–M6 (all single-core baseline)
2026 Q2  Phase A1: DarcyOS BUILD_SMP=1
2026 Q3  Phase A2–A3: IrisOS, MekkanaOS, PrimeOS
2026 Q4+ Phase B: APU-line (only if SMP-line SMP signed off)
```

---

## Checklist before marking SMP ● in matrix

- [ ] Proven on **SMP-SmpOS** first (least developed single-core kernel)
- [ ] 4 cores each run own `kmain` / scheduler
- [ ] `CoreSend` + IPI + mailbox under QEMU
- [ ] Global `WhoIs` via CNS
- [ ] Local `Send`/`Receive` unchanged
- [ ] `BUILD_SMP=0` default still passes all existing tests on DarcyOS
- [ ] APU-line **not** required for sign-off

---

## Related docs

- [SMP_MULTIKERNEL_IPC.md](SMP_MULTIKERNEL_IPC.md) — architecture
- [CS452_FEATURE_MATRIX.md](CS452_FEATURE_MATRIX.md) — status columns
- [APU_MULTI_CORE.md](APU_MULTI_CORE.md) — APU-line (separate from SMP focus)
