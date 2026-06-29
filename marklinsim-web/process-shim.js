/*
 * Browser shim for Node's `process` global.
 *
 * Pixi.js v4 and a few of its polyfills sniff `process.env.NODE_ENV`,
 * `process.platform`, etc. on import. In a browser there is no `process`
 * global -- the references throw ReferenceError at load time. esbuild's
 * --inject prepends this file so a minimal stand-in is always available.
 *
 * `process.env.NODE_ENV` is also dead-code-eliminated via --define, so this
 * only needs to handle the misc. accesses that aren't statically replaced.
 */
export let process = {
    env: { NODE_ENV: 'production' },
    platform: 'browser',
    browser: true,
    version: 'v0.0.0',
    versions: { node: '0.0.0' },
    nextTick: function (fn) { setTimeout(fn, 0); },
    cwd: function () { return '/'; },
};
