#!/usr/bin/env bash
set -euo pipefail

cd /workspace

run_os() {
	echo "Starting MekkanaOS under QEMU (raspi4b). Ctrl+C to stop."
	make -j"$(nproc)" MODE=qemu all
	exec bash qemu/run.sh
}

case "${1:-run}" in
	run)
		run_os
		;;
	shell|bash)
		exec bash
		;;
	build)
		make -j"$(nproc)" MODE=qemu all
		;;
	make)
		shift
		exec make -j"$(nproc)" "$@"
		;;
	*)
		exec "$@"
		;;
esac
