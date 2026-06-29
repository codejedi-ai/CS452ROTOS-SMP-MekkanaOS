/*
 * Browser-side shim for the `electron` module.
 *
 * The unmodified upstream renderer (src/view/index.ts) imports
 * { ipcRenderer } from 'electron' and calls
 *   ipcRenderer.sendSync('getTickDelta' | 'getTickFull')
 * to pull synchronous JSON tick payloads from the Electron main process.
 *
 * In our web build the server is plain Node + WebSocket -- no Electron, no
 * IPC. So we alias 'electron' to this file at bundle time (see esbuild
 * --alias:electron=...) and implement ipcRenderer.sendSync against an in-
 * memory cache that's continuously refreshed from a WebSocket stream.
 *
 *   server.ts pushes one JSON tick payload per frame on /ws.
 *   sendSync returns the most recent JSON received.
 *
 * The renderer happily re-renders the cached payload each animation frame --
 * one frame of staleness is invisible at 120 Hz.
 */
let latestJson: string = JSON.stringify({
    time: 0,
    objectChanged: false,
    trains: [],
    drawTrack: false,
    straightTracks: [],
    bezierTracks: [],
    onlineSwitchStraightTracks: [],
    offlineSwitchStraightTracks: [],
    onlineSwitchBezierTracks: [],
    offlineSwitchBezierTracks: [],
    switches: [],
    sensors: [],
});
let sawFull = false;

function connect(): void {
    const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
    const ws = new WebSocket(`${proto}//${location.host}/ws`);
    ws.onopen = () => console.log('[electron-shim] ws connected');
    ws.onmessage = (ev) => {
        latestJson = ev.data as string;
        sawFull = true;
    };
    ws.onclose = () => {
        console.warn('[electron-shim] ws closed; reconnecting in 1s');
        setTimeout(connect, 1000);
    };
    ws.onerror = () => { /* close fires next */ };
}
connect();

class IpcRenderer {
    sendSync(channel: string, ..._args: any[]): string {
        // upstream uses 'getTickDelta' and 'getTickFull'. Both return JSON
        // strings; the server picks delta vs full per-client, so the channel
        // value doesn't matter here.
        if (!sawFull) {
            // First frame, no data yet -- return the harmless default seed.
        }
        return latestJson;
    }
    // Kept as no-ops in case future upstream code uses them.
    on(_channel: string, _listener: (...args: any[]) => void): this { return this; }
    once(_channel: string, _listener: (...args: any[]) => void): this { return this; }
    send(_channel: string, ..._args: any[]): void { /* no-op */ }
}

export const ipcRenderer = new IpcRenderer();
