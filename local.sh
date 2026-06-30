#!/usr/bin/env bash
# Legacy shortcut — prefer ./dev.sh
set -euo pipefail
cd "$(dirname "$0")"

cmd=${1:-run}
case "$cmd" in
run|test|make|setup|test-k1|test-k2|test-k3|test-k4)
	exec ./dev.sh "$@"
	;;
shell)
	exec bash
	;;
*)
	echo "usage: ./local.sh [run|test|make|setup|shell]" >&2
	exit 1
	;;
esac
