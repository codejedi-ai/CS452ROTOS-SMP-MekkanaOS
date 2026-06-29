#!/usr/bin/env bash
# Dev wrapper — uses CS452ROTOS-PLATFORM image from Docker Hub (or local build).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM="$(cd "${ROOT}/../github_codejedi-ai_CS452ROTOS-PLATFORM" && pwd)"
IMAGE="${DARCYOS_IMAGE:-codejedi-ai/cs452rotos-platform:latest}"

usage() {
	cat <<'EOF'
Usage: ./dev.sh <command>

Commands:
  build-image Pull/build the shared platform image (Docker Hub or local PLATFORM)
  run         Build kernel and run in QEMU (raspi4b, virtual Marklin via vhw)
  test-k1     Run K1 tests under QEMU
  test-k2     Run K2 tests under QEMU
  test-k3     Run K3 tests under QEMU
  test-k4     Run K4 tests under QEMU
  shell       Open a shell in the dev container
  make [args] Run make inside the container (e.g. ./dev.sh make clean)

Environment:
  DARCYOS_IMAGE   default: codejedi-ai/cs452rotos-platform:latest
  MARKLIN         passed to qemu/run.sh (vhw|marklinsim|custom)
  START_VHW       set to 1 to auto-start tools/vhw.py when MARKLIN=vhw
EOF
}

ensure_image() {
	DARCYOS_IMAGE="${IMAGE}"
	# shellcheck source=/dev/null
	source "${PLATFORM}/scripts/ensure-image.sh"
}

docker_run() {
	ensure_image
	local -a args=(--rm -v "${ROOT}:/workspace" -w /workspace -e XDIR=/opt/toolchain -e IN_DOCKER=1)
	if [ -t 0 ] && [ -t 1 ]; then
		args+=(-it)
	else
		args+=(-i)
	fi
	if [ -e /dev/kvm ]; then
		args+=(--device /dev/kvm)
		if command -v getent >/dev/null 2>&1 && getent group kvm >/dev/null 2>&1; then
			args+=(--group-add "$(getent group kvm | cut -d: -f3)")
		fi
	fi
	args+=(--entrypoint bash "${IMAGE}")
	docker run "${args[@]}" -lc 'exec /workspace/scripts/container-run.sh "$@"' -- "$@"
}

cmd="${1:-run}"
shift || true

case "${cmd}" in
	build|build-image)
		# shellcheck source=/dev/null
		source "${PLATFORM}/scripts/ensure-image.sh"
		;;
	run)
		docker_run run
		;;
	test-k1|test-k2|test-k3|test-k4|test)
		docker_run "${cmd}"
		;;
	shell|bash)
		docker_run shell
		;;
	make)
		docker_run make -j"$(nproc)" "$@"
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
