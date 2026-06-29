#!/usr/bin/env bash
# Run the QEMU-built kernel against a virtual Marklin.
#
# Serial wiring (matches the MyNIX scheme):
#   serial0  -> CONSOLE   (PL011 UART0, kernel shell)
#   serial1  -> MARKLIN   (BCM2835 AUX mini-UART; QEMU only)
#
# Marklin link options, in order of preference:
#
#   1) Martin1994/MarklinSim  (https://github.com/Martin1994/MarklinSim)
#      Electron GUI; the canonical CS452 multi-train simulator. Run it on
#      the HOST (it can't run inside this Linux container) — it listens on
#      tcp://127.0.0.1:3018. From the container, reach the host via
#      host.docker.internal. Pick this with:
#          MARKLIN=marklinsim ./qemu/run.sh
#
#   2) tools/vhw.py  -- the bundled Python single/multi-train fallback.
#      Runs inside the container automatically when START_VHW=1.
#          MARKLIN=vhw  START_VHW=1 ./qemu/run.sh
#
#   3) Custom TCP endpoint. Set MARKLIN_SERIAL directly:
#          MARKLIN_SERIAL=tcp:my-host:9000 ./qemu/run.sh

set -euo pipefail

IMG=${IMG:-0-d273liu.img}
MARKLIN=${MARKLIN:-vhw}

if [[ ! -f "$IMG" ]]; then
    echo "run.sh: $IMG not found; run 'make all' first" >&2
    exit 1
fi

case "$MARKLIN" in
    marklinsim)
        MARKLIN_HOST=${MARKLIN_HOST:-host.docker.internal}
        MARKLIN_PORT=${MARKLIN_PORT:-3018}
        # reconnect=1: if MarklinSim isn't up yet, QEMU retries every 1s
        # instead of failing immediately. Start MarklinSim before OR after
        # this container -- whichever order is convenient.
        MARKLIN_SERIAL=${MARKLIN_SERIAL:-"tcp:${MARKLIN_HOST}:${MARKLIN_PORT},reconnect=1"}
        echo "run.sh: targeting MarklinSim at ${MARKLIN_HOST}:${MARKLIN_PORT} (Track A)" >&2
        echo "        clone + run on the host: git clone https://github.com/Martin1994/MarklinSim" >&2
        echo "                                  cd MarklinSim && npm install && npm start" >&2
        ;;
    vhw)
        MARKLIN_PORT=${MARKLIN_PORT:-6011}
        MARKLIN_SERIAL=${MARKLIN_SERIAL:-"tcp:127.0.0.1:${MARKLIN_PORT}"}
        if [[ -n "${START_VHW:-}" ]]; then
            python3 tools/vhw.py --port "$MARKLIN_PORT" &
            VHW_PID=$!
            trap 'kill $VHW_PID 2>/dev/null || true' EXIT
            sleep 0.3
        fi
        ;;
    custom)
        : "${MARKLIN_SERIAL:?MARKLIN=custom requires MARKLIN_SERIAL to be set}"
        ;;
    *)
        echo "run.sh: unknown MARKLIN=$MARKLIN (expected marklinsim|vhw|custom)" >&2
        exit 1
        ;;
esac

CONSOLE_SERIAL=${CONSOLE_SERIAL:-stdio}

exec qemu-system-aarch64 \
    -machine raspi4b \
    -cpu cortex-a72 \
    -m 2G \
    -display none \
    -monitor none \
    -serial "$CONSOLE_SERIAL" \
    -serial "$MARKLIN_SERIAL" \
    -nic none \
    -kernel "$IMG"
