#!/usr/bin/env bash
# Prod-container entrypoint. ONE externally-exposed port:
#
#   8080  -> single web UI:
#              /         -> track view (Pixi) + iframe to kernel shell
#              /bundle.js
#              /ws       -> tick payloads
#              /shell/.. -> reverse-proxied to internal ttyd
#
# Internal-only (loopback inside the container):
#   3018  -> Marklin protocol bridge (kernel <-> MarklinSim)
#   7681  -> ttyd (proxied by the Node server above)
set -euo pipefail

export MARKLIN=marklinsim
export MARKLIN_HOST=127.0.0.1
export MARKLIN_PORT=3018

cleanup() {
    pkill -P $$ 2>/dev/null || true
    pkill -f 'dist/server.js' 2>/dev/null || true
    pkill -x ttyd 2>/dev/null || true
}
trap cleanup EXIT INT TERM

echo "[start-prod] starting ttyd on 127.0.0.1:7681 (loopback only, base=/shell)"
# -b /shell  : emit URLs like /shell/token so the iframe at /shell/ works
ttyd -W -i 127.0.0.1 -p 7681 -b /shell -t fontSize=14 /work/qemu/run.sh \
    >/var/log/ttyd.log 2>&1 &

echo "[start-prod] starting MarklinSim web server on :8080 (TCP :3018 + proxy /shell -> :7681)"
cd /opt/MarklinSim
exec node dist/server.js
