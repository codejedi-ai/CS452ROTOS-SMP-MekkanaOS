# CS452ROTOS-SMP-MekkanaOS

**OS name:** MekkanaOS  
**Git provider:** codejedi-ai  
**Origin repository:** [github.com/codejedi-ai/CS452ROTOS-SMP-MekkanaOS](https://github.com/codejedi-ai/CS452ROTOS-SMP-MekkanaOS)

Second-generation CS452 trains kernel: reorganized `src/` tree, boot self-tests, display server at startup, observer-pattern shell polish, and Marklin virtual hardware (`tools/vhw.py`).

Formerly `cs452-trains-2`.

## Quick start

```bash
./dev.sh run       # build + QEMU with virtual Marklin (vhw)
./dev.sh shell     # container shell
```

## Layout

```
src/
  k1/ k2/ k3/     milestone implementations + README per layer
  ui/              shell, display_server
  tests/           boot_tests
qemu/              run scripts
tools/             vhw.py Marklin simulator
```


## Family documentation

| Doc | Description |
|-----|-------------|
| [docs/CS452_FEATURE_MATRIX.md](docs/CS452_FEATURE_MATRIX.md) | Feature parity across all ROTOS kernels |
| [docs/CODE_EVOLUTION_TREE.md](docs/CODE_EVOLUTION_TREE.md) | Architecture evolution (not git history) |
| [docs/APU_MULTI_CORE.md](docs/APU_MULTI_CORE.md) | Multi-core / APU server design and feasibility |
| [docs/SMP_MULTIKERNEL_IPC.md](docs/SMP_MULTIKERNEL_IPC.md) | SMP target architecture (all kernels) |
| [docs/SMP_ROADMAP_ALL_KERNELS.md](docs/SMP_ROADMAP_ALL_KERNELS.md) | Per-kernel SMP implementation plans |

**Multi-core (APU):** Not yet in this K-line tree; a port from the NIX APUServer design is planned. See [docs/APU_MULTI_CORE.md](docs/APU_MULTI_CORE.md).
## Relation to family

- **MekkanaOS** — experimental refactor and test-harness line
- **IrisOS** (CS452ROTOS-SMP-IrisOS) — legacy flat-layout orphan branch (formerly OrphanOS)
- **DarcyOS** — production K4 I/O + TC1 integration

## Source layout (LAYERED_K)

Kernel sources live under `src/` (`k0`–`k4`, `common`, `common/custom`, `tc1`, `tests`). Build from the repo root: `make all` (delegates to `src/Makefile`).
