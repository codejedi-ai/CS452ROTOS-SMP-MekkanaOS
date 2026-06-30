#!/usr/bin/env bash
# Host dev wrapper — MekkanaOS (QEMU network kernel)
set -euo pipefail

OS_NAME="MekkanaOS"
TEST_TARGET="test-k1"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

PLATFORM_DEFAULT="${ROOT}/../github_codejedi-ai_CS452ROTOS-PLATFORM"
if [[ -d "${PLATFORM_DEFAULT}" ]]; then
	PLATFORM="$(cd "${PLATFORM_DEFAULT}" && pwd)"
elif [[ -n "${PLATFORM:-}" && -d "${PLATFORM}" ]]; then
	PLATFORM="$(cd "${PLATFORM}" && pwd)"
else
	echo "Cannot find CS452ROTOS-PLATFORM (expected ${PLATFORM_DEFAULT} or set PLATFORM)." >&2
	exit 1
fi

# shellcheck source=../setup-host-env.sh
source "${PLATFORM}/scripts/setup-host-env.sh"

usage() {
	cat <<EOF
Usage: ./dev.sh <command>

Commands:
  setup         Install/check host toolchain and QEMU (from CS452ROTOS-PLATFORM)
  run           Build + QEMU with console + ROTS network link (serial1 → hub)
  run-console   Build + QEMU console only (no network link)
  hub           Start rotos_link_hub.py on :7100 (shared by all SMP OSes)
  test-hub      Run rotos_link_hub Python tests (peer discovery, timeout)
  test          Run ${TEST_TARGET} under QEMU
  test-k1       Run K1 tests under QEMU
  test-k2       Run K2 tests under QEMU
  test-k3       Run K3 tests under QEMU
  test-k4       Run K4 tests under QEMU
  test-smp      Run SMP hub/mailbox tests under QEMU
  make [args]   Run make on the host (default: make all)

Environment:
  HUB_PORT      ROTS link hub TCP port (default: 7100)
  START_HUB=0   Do not auto-start hub in ./dev.sh run
EOF
}

setup_terminal() {
	if [[ -t 0 ]]; then
		stty -icanon -echo min 1 time 0 2>/dev/null || true
		trap 'stty icanon echo 2>/dev/null || true' EXIT INT TERM
	fi
}

run_net() {
	setup_terminal
	echo "Starting ${OS_NAME} under QEMU (console + ROTS network link)."
	exec make run-net
}

run_console() {
	setup_terminal
	echo "Starting ${OS_NAME} under QEMU (console only). Ctrl+A then X to exit."
	exec make run
}

cmd="${1:-run}"
shift || true

case "${cmd}" in
	setup)
		setup_host_env "${ROOT}"
		echo "Host toolchain and QEMU are ready."
		;;
	run)
		setup_host_env "${ROOT}"
		make -j"$(nproc)" all
		run_net
		;;
	run-console)
		setup_host_env "${ROOT}"
		make -j"$(nproc)" all
		run_console
		;;
	hub)
		exec python3 "${ROOT}/tools/rotos_link_hub.py" --port "${HUB_PORT:-7100}"
		;;
	lan)
		exec bash "${ROOT}/tools/rotos_lan.sh" howto
		;;
	test-hub)
		UV_BIN="${UV:-$(command -v uv 2>/dev/null || echo "${HOME}/.local/bin/uv")}"
		exec "${UV_BIN}" run python3 -m unittest discover -s "${ROOT}/tools" -p 'test_rotos_link_hub.py' -v
		;;
	test)
		setup_host_env "${ROOT}"
		make -j"$(nproc)" "${TEST_TARGET}"
		;;
	test-k1|test-k2|test-k3|test-k4|test-smp)
		setup_host_env "${ROOT}"
		make -j"$(nproc)" "${cmd}"
		;;
	make)
		setup_host_env "${ROOT}"
		if [[ "$#" -eq 0 ]]; then
			make -j"$(nproc)" all
		else
			make -j"$(nproc)" "$@"
		fi
		;;
	help|-h|--help)
		usage
		;;
	*)
		echo "Unknown command: ${cmd}" >&2
		usage >&2
		exit 1
		;;
esac
