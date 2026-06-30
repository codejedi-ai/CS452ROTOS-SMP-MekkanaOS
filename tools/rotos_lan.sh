#!/usr/bin/env bash
# One shared ROTS LAN hub for all SMP QEMU instances (serial1 → TCP :7100).
#
# Stock QEMU raspi4b has no Ethernet (GENET is not emulated), so this hub is
# the inter-OS "internet" — all OS peers connect to the same TCP port.
set -euo pipefail

WORKSPACE="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
HUB_PORT="${HUB_PORT:-7100}"
HUB_HOST="${HUB_HOST:-127.0.0.1}"

REPOS=(
	"github_codejedi-ai_CS452ROTOS-SMP-DarcyOS|DarcyOS"
	"github_codejedi-ai_CS452ROTOS-SMP-IrisOS|IrisOS"
	"github_codejedi-ai_CS452ROTOS-SMP-MekkanaOS|MekkanaOS"
	"github_codejedi-ai_CS452ROTOS-SMP-PrimeOS|PrimeOS"
)

hub_running() {
	ss -ltn 2>/dev/null | grep -q ":${HUB_PORT} "
}

start_hub() {
	if hub_running; then
		echo "ROTS LAN hub already listening on ${HUB_HOST}:${HUB_PORT}"
		return 0
	fi
	local hub_py="${WORKSPACE}/github_codejedi-ai_CS452ROTOS-SMP-DarcyOS/tools/rotos_link_hub.py"
	if [[ ! -f "$hub_py" ]]; then
		hub_py="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/rotos_link_hub.py"
	fi
	echo "Starting ROTS LAN hub on ${HUB_HOST}:${HUB_PORT} ..."
	python3 "$hub_py" --host "$HUB_HOST" --port "$HUB_PORT" &
	sleep 0.3
	if hub_running; then
		echo "Hub ready."
	else
		echo "Failed to start hub on port ${HUB_PORT}" >&2
		exit 1
	fi
}

usage() {
	cat <<EOF
Usage: ./tools/rotos_lan.sh <command>

Commands:
  hub           Start the shared LAN hub (port ${HUB_PORT})
  howto         Show how to run each OS on the same LAN
  run-bg <os>   Build + run one OS in background (console → /tmp/rotos-<os>.log)

All SMP repos use HUB_PORT=${HUB_PORT}. In a second terminal:
  cd <repo> && START_HUB=0 ./dev.sh run

Shell on each OS: linktest, peers, msg <os> <text>, broadcast <text>
EOF
}

cmd_howto() {
	start_hub
	echo ""
	echo "Open one terminal per OS (hub stays on :${HUB_PORT}):"
	for entry in "${REPOS[@]}"; do
		IFS='|' read -r dir name <<< "$entry"
		echo "  cd ${WORKSPACE}/${dir} && START_HUB=0 HUB_PORT=${HUB_PORT} ./dev.sh run   # ${name}"
	done
	echo ""
	echo "Then try: linktest | peers | msg IrisOS hello | broadcast hi all"
}

cmd_run_bg() {
	local os="${1:-}"
	if [[ -z "$os" ]]; then
		echo "usage: ./tools/rotos_lan.sh run-bg <DarcyOS|IrisOS|MekkanaOS|PrimeOS>" >&2
		exit 1
	fi
	start_hub
	local repo=""
	for entry in "${REPOS[@]}"; do
		IFS='|' read -r dir name <<< "$entry"
		if [[ "$name" == "$os" ]]; then
			repo="${WORKSPACE}/${dir}"
			break
		fi
	done
	if [[ -z "$repo" || ! -d "$repo" ]]; then
		echo "Unknown OS: ${os}" >&2
		exit 1
	fi
	local log="/tmp/rotos-${os}.log"
	echo "Building and launching ${os} (console log: ${log}) ..."
	(
		cd "$repo"
		./dev.sh make all
		START_HUB=0 HUB_PORT="$HUB_PORT" CONSOLE_SERIAL="file:${log}" ./qemu/run.sh
	) &
	echo "${os} PID $! — tail -f ${log}"
}

cmd="${1:-howto}"
shift || true
case "$cmd" in
	hub) start_hub ;;
	howto) cmd_howto ;;
	run-bg) cmd_run_bg "${1:-}" ;;
	help|-h|--help) usage ;;
	*)
		echo "Unknown command: ${cmd}" >&2
		usage >&2
		exit 1
		;;
esac
