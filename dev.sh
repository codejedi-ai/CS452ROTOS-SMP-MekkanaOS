#!/usr/bin/env bash
# Dev wrapper — uses CS452ROTOS-PLATFORM image from Docker Hub (or local build).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM="$(cd "${ROOT}/../github_codejedi-ai_CS452ROTOS-PLATFORM" && pwd)"
cd "$ROOT"

DC=(docker compose)

usage() {
	cat <<'EOF'
Usage: ./dev.sh <command>

Commands:
  build-image Pull/build the shared platform image (Docker Hub or local PLATFORM)
  run         Build kernel and run in QEMU (raspi4b) — host terminal
  test-k1     Run K1 tests under QEMU
  test-k2     Run K2 tests under QEMU
  test-k3     Run K3 tests under QEMU
  test-k4     Run K4 tests under QEMU
  shell       Open a shell in the dev container
  make [args] Run make inside the container (e.g. ./dev.sh make clean)

Environment:
  DARCYOS_IMAGE   default: codejedi-ai/cs452rotos-platform:latest
EOF
}

ensure_image() {
	DARCYOS_IMAGE="${DARCYOS_IMAGE:-codejedi-ai/cs452rotos-platform:latest}"
	# shellcheck source=/dev/null
	source "${PLATFORM}/scripts/ensure-image.sh"
}

kvm_args() {
	if [ -e /dev/kvm ]; then
		echo --device /dev/kvm
		if command -v getent >/dev/null 2>&1 && getent group kvm >/dev/null 2>&1; then
			echo --group-add "$(getent group kvm | cut -d: -f3)"
		fi
	fi
}

compose_run() {
	local service="$1"
	shift
	local -a tty=(-i)
	if [ -t 0 ] && [ -t 1 ]; then
		tty=(-it)
	fi
	# shellcheck disable=SC2207
	local -a kvm=( $(kvm_args) )
	"${DC[@]}" run --rm "${tty[@]}" "${kvm[@]}" "$service" "$@"
}

cmd="${1:-run}"
shift || true

case "${cmd}" in
	build|build-image)
		ensure_image
		;;
	run)
		# Foreground QEMU on stdio in THIS terminal. Use compose run -it (not up)
		# so docker allocates a real TTY tied to the host shell.
		ensure_image
		compose_run run
		;;
	test-k1|test-k2|test-k3|test-k4|test)
		ensure_image
		"${DC[@]}" run --rm -T $(kvm_args) "${cmd}"
		;;
	shell|bash)
		ensure_image
		compose_run shell
		;;
	make)
		ensure_image
		"${DC[@]}" run --rm -T $(kvm_args) make make -j"$(nproc)" "$@"
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
