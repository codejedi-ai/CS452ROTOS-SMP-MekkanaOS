/*
 * MarklinSim web server (single-port: 8080).
 *
 *   Routes
 *     GET  /                 -> public/index.html  (split layout)
 *     GET  /bundle.js        -> public/bundle.js   (Pixi renderer)
 *     WS   /ws               -> tick payload stream
 *     /shell/*               -> reverse-proxy to internal ttyd at :7681
 *
 *   TCP :3018 (internal)     -> Marklin protocol bridge (kernel <-> sim)
 *
 * The repo only carries this server + a tiny browser shim; the upstream
 * MarklinSim renderer files (under src/view, src/model, ...) are cloned at
 * docker build time and never modified.
 */
import * as http from 'http';
import * as fs from 'fs';
import * as path from 'path';
import * as config from './config';
import { MarklinIO } from './marklin/marklin_io';
import { MarklinController } from './marklin/marklin_controller';
import { TrackA } from './marklin/setup/track_a';
import { ITickPayload } from './util/tick_payload';

// runtime-only deps (installed by install-overlay.sh)
// eslint-disable-next-line @typescript-eslint/no-var-requires
const WebSocketServer = require('ws').Server;
// eslint-disable-next-line @typescript-eslint/no-var-requires
const httpProxy = require('http-proxy');

const HTTP_PORT  = parseInt(process.env.HTTP_PORT  || '8080', 10);
const TTYD_PORT  = parseInt(process.env.TTYD_PORT  || '7681', 10);
const PUBLIC_DIR = path.resolve(__dirname, '..', 'public');

// --- sim runtime ------------------------------------------------------------
const io = new MarklinIO();
io.listenTCP(config.SOCKET_PORT);

const controller = new MarklinController();
io.setController(controller);
TrackA.setup(controller);

const tickIntervalMs = 1000 / config.TICK_RATE;
setInterval(() => controller.tick(tickIntervalMs), tickIntervalMs);

// --- reverse proxy to internal ttyd ----------------------------------------
const ttydProxy = httpProxy.createProxyServer({
    target: `http://127.0.0.1:${TTYD_PORT}`,
    ws: true,
    changeOrigin: true,
});
ttydProxy.on('error', (err: Error) => {
    console.warn('[proxy] ttyd error:', err.message);
});

// --- HTTP -------------------------------------------------------------------
const MIME: Record<string, string> = {
    '.html': 'text/html; charset=utf-8',
    '.js':   'application/javascript; charset=utf-8',
    '.css':  'text/css; charset=utf-8',
    '.png':  'image/png',
    '.svg':  'image/svg+xml',
    '.ico':  'image/x-icon',
    '.map':  'application/json',
};

const server = http.createServer((req, res) => {
    const url = req.url || '/';
    if (url === '/shell' || url.startsWith('/shell/') || url.startsWith('/shell?')) {
        // ttyd is started with --base-path=/shell, so it expects the prefix
        // to remain in the URL; do NOT strip it.
        ttydProxy.web(req, res);
        return;
    }
    const cleanPath = url === '/' ? '/index.html' : url.split('?')[0];
    const file = path.normalize(path.join(PUBLIC_DIR, cleanPath));
    if (!file.startsWith(PUBLIC_DIR)) {
        res.writeHead(403); res.end('forbidden'); return;
    }
    fs.readFile(file, (err, data) => {
        if (err) { res.writeHead(404); res.end('not found'); return; }
        res.writeHead(200, { 'Content-Type': MIME[path.extname(file)] || 'application/octet-stream' });
        res.end(data);
    });
});

// --- WebSocket: tick payload stream + ttyd WS proxy -------------------------
const wss = new WebSocketServer({ noServer: true });

server.on('upgrade', (req, socket, head) => {
    const url = req.url || '';
    if (url === '/ws' || url.startsWith('/ws?')) {
        wss.handleUpgrade(req, socket as any, head, (ws: any) => wss.emit('connection', ws, req));
    } else if (url.startsWith('/shell')) {
        // Keep the /shell prefix; ttyd is configured with --base-path=/shell.
        ttydProxy.ws(req, socket, head);
    } else {
        socket.destroy();
    }
});

interface Client { ws: any; sentInitial: boolean; }
const clients = new Set<Client>();

wss.on('connection', (ws: any) => {
    const c: Client = { ws, sentInitial: false };
    clients.add(c);
    ws.on('close', () => clients.delete(c));
    ws.on('error', () => clients.delete(c));
});

setInterval(() => {
    if (clients.size === 0) return;
    let fullJson: string | null = null;
    let deltaJson: string | null = null;
    for (const c of clients) {
        if (c.ws.readyState !== 1) continue;
        let msg: string;
        if (!c.sentInitial) {
            if (!fullJson)  fullJson  = JSON.stringify(controller.getTick(false));
            msg = fullJson;
            c.sentInitial = true;
        } else {
            if (!deltaJson) deltaJson = JSON.stringify(controller.getTick(true));
            msg = deltaJson;
        }
        try { c.ws.send(msg); } catch { /* will close itself */ }
    }
}, tickIntervalMs);

server.listen(HTTP_PORT, () => {
    console.log(`MarklinSim web UI on http://0.0.0.0:${HTTP_PORT}/  (WS /ws, shell /shell/)`);
});
