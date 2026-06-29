#!/usr/bin/env bash
set -euo pipefail

cd /workspace

run_os() {
	# Root Makefile clean+build runs before `make run`; qemu/run.sh wires Marklin serial.
	echo "Starting MekkanaOS under QEMU (raspi4b). Ctrl+C to stop."
	export MARKLIN=${MARKLIN:-vhw}
	export START_VHW=${START_VHW:-1}
	exec bash qemu/run.sh
}

run_tests() {
	target="${1:-test-k1}"
	echo "Running ${target}..."
	make -j"$(nproc)" "${target}"
}

case "${1:-run}" in
	run)
		run_os
		;;
	test-k1|test-k2|test-k3|test-k4|test)
		run_tests "${1}"
		;;
	shell|bash)
		exec bash
		;;
	build)
		make -j"$(nproc)" all
		;;
	make)
		shift
		exec make -j"$(nproc)" "$@"
		;;
	*)
		exec "$@"
		;;
esac
