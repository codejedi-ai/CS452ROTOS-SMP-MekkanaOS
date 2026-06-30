#!/usr/bin/env bash
# QEMU launch: serial0 = console, serial1 = ROTS network link (TCP hub).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMG="${IMG:-${ROOT}/0-d273liu.img}"
HUB_HOST="${HUB_HOST:-127.0.0.1}"
HUB_PORT="${HUB_PORT:-7100}"
LINK_SERIAL="${LINK_SERIAL:-tcp:${HUB_HOST}:${HUB_PORT},reconnect=1}"
CONSOLE_SERIAL="${CONSOLE_SERIAL:-mon:stdio}"
START_HUB="${START_HUB:-1}"

if [[ ! -f "$IMG" ]]; then
	echo "qemu/run.sh: $IMG not found; run './dev.sh make all' first" >&2
	exit 1
fi

if [[ "$START_HUB" == "1" ]]; then
	if ! ss -ltn 2>/dev/null | grep -q ":${HUB_PORT} "; then
		python3 "${ROOT}/tools/rotos_link_hub.py" --port "$HUB_PORT" &
		HUB_PID=$!
		trap 'kill $HUB_PID 2>/dev/null || true' EXIT
		sleep 0.2
	fi
fi

QEMU="$(command -v qemu-system-aarch64 || true)"
if [[ -z "$QEMU" ]]; then
	echo "qemu-system-aarch64 not found; run ./dev.sh setup" >&2
	exit 1
fi

echo "run.sh: console=$CONSOLE_SERIAL link=$LINK_SERIAL" >&2

exec "$QEMU" \
	-machine raspi4b \
	-cpu cortex-a72 \
	-smp 4 \
	-m 2G \
	-display none \
	-serial "$CONSOLE_SERIAL" \
	-serial "$LINK_SERIAL" \
	-nic none \
	-kernel "$IMG"
