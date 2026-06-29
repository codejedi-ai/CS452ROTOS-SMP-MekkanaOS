#!/usr/bin/env bash
# Apply this repo's overlay onto an upstream MarklinSim clone, then build.
#
# Usage:  install-overlay.sh <upstream_dir> <overlay_dir>
#
# Non-destructive: upstream source files are NOT modified or deleted. We just
# add three files alongside them:
#
#   src/server.ts                  -- Node entry: TCP :3018 + HTTP :8080 + /ws
#   src/electron-browser-shim.ts   -- browser stub for `electron` module
#   public/index.html              -- loads the bundled renderer
#
# Then:
#   * tsc                 (upstream's compile-main) emits dist/server.js
#                         alongside the upstream dist/main.js (unused).
#   * esbuild bundles src/view/index.ts -> public/bundle.js,
#     redirecting the upstream `import { ipcRenderer } from 'electron'`
#     to our shim via --alias:electron=...
set -euo pipefail

upstream="${1:?upstream MarklinSim path}"
overlay="${2:?overlay path}"

cd "$upstream"

echo "[overlay] copying additive files (upstream src/ stays untouched)"
cp "${overlay}/server.ts"                  src/server.ts
cp "${overlay}/electron-browser-shim.ts"   src/electron-browser-shim.ts
cp "${overlay}/process-shim.js"            src/process-shim.js
mkdir -p public
cp "${overlay}/index.html"                 public/index.html

echo "[overlay] installing upstream deps"
npm install --no-audit --no-fund

echo "[overlay] installing build-only deps (ws + esbuild + http-proxy + browser polyfills)"
# pixi.js v4 has a handful of Node-only require('path' | 'url' | 'querystring')
# calls in code paths we never reach, but esbuild has to resolve them anyway.
npm install --no-audit --no-fund --no-save \
    ws @types/ws esbuild http-proxy \
    path-browserify url querystring-es3 assert util buffer events

echo "[overlay] compiling all .ts in src/ with upstream tsc"
./node_modules/.bin/tsc

echo "[overlay] bundling renderer (esbuild, aliasing electron -> shim)"
./node_modules/.bin/esbuild src/view/index.ts \
    --bundle \
    --platform=browser \
    --target=es2018 \
    --alias:electron=./src/electron-browser-shim.ts \
    --alias:path=path-browserify \
    --alias:url=url \
    --alias:querystring=querystring-es3 \
    --alias:assert=assert \
    --alias:util=util \
    --alias:buffer=buffer \
    --alias:events=events \
    --define:process.env.NODE_ENV='"production"' \
    --define:global=globalThis \
    --inject:./src/process-shim.js \
    --outfile=public/bundle.js \
    --minify

echo "[overlay] done."
echo "[overlay]   start the server with:  node dist/server.js"
echo "[overlay]   browser:                http://<host>:8080/"
