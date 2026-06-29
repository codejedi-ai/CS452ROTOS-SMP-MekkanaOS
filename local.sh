#!/usr/bin/env bash
# Build the kernel inside the Docker dev image and drop you straight into the
# QEMU kernel CLI on this terminal. One command, end to end.
#
# Usage:
#   ./local.sh              -> build (if needed) + boot QEMU; exposes kernel
#                              stdio + the bundled virtual Marklin (tools/vhw.py)
#   ./local.sh build        -> just (re)build the dev image
#   ./local.sh shell        -> bash inside the container, source bind-mounted
#   ./local.sh clean        -> drop the dev image
#
# Source is bind-mounted at /work, so edits on the host show up immediately:
# `make MODE=qemu` inside the container picks up your local changes without
# rebuilding the image.
set -euo pipefail
cd "$(dirname "$0")"

IMAGE=cs452-trains:dev

ensure_image() {
    if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
        echo "[local.sh] building $IMAGE (first run downloads ~120 MB toolchain)"
        docker build --target dev -t "$IMAGE" .
    fi
}

cmd=${1:-run}
case "$cmd" in
run)
    ensure_image
    echo "[local.sh] make + qemu inside container; Ctrl-A then X to exit QEMU,"
    echo "           or Ctrl-C in this shell to tear it all down."
    exec docker run --rm -it \
        -e MARKLIN=vhw -e START_VHW=1 \
        -v "$PWD":/work \
        "$IMAGE" \
        bash -c 'make MODE=qemu && ./qemu/run.sh'
    ;;
build)
    docker build --target dev -t "$IMAGE" .
    ;;
shell)
    ensure_image
    exec docker run --rm -it -v "$PWD":/work "$IMAGE" bash
    ;;
clean)
    docker image rm -f "$IMAGE" >/dev/null 2>&1 || true
    echo "[local.sh] removed $IMAGE"
    ;;
*)
    echo "usage: ./local.sh [run|build|shell|clean]" >&2
    exit 1
    ;;
esac
