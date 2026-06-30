#!/usr/bin/env python3
"""
ROTS network hub — multiplexes QEMU serial1 links between OS instances.

Each kernel sends HELLO <os_name> on boot. The hub routes line-based messages:

  PING                          -> PONG
  PEERS                         -> PEERS <roster>
  MSG <dst_os> <text...>        -> delivered to named peer
  BROADCAST <text...>           -> all other peers

On join/leave the hub broadcasts PEERS <roster> to every connected peer.

Run:  python3 tools/rotos_link_hub.py [--port 7100]
QEMU: -serial tcp:127.0.0.1:7100  (one connection per OS instance)
"""

from __future__ import annotations

import argparse
import socket
import threading
import sys
from typing import Callable

SendFn = Callable[[socket.socket, str], None]


class RotosHub:
    """In-memory hub state + line protocol (testable without sockets)."""

    def __init__(self) -> None:
        self._peers: dict[str, object] = {}
        self._lock = threading.Lock()

    def roster(self) -> str:
        with self._lock:
            return " ".join(sorted(self._peers.keys()))

    def join(self, name: str, token: object) -> str:
        with self._lock:
            self._peers[name] = token
        return f"PEERS {self.roster()}"

    def leave(self, name: str, token: object) -> bool:
        with self._lock:
            if self._peers.get(name) is token:
                del self._peers[name]
                return True
        return False

    def get_token(self, name: str) -> object | None:
        with self._lock:
            return self._peers.get(name)

    def other_tokens(self, name: str) -> list[object]:
        with self._lock:
            return [t for n, t in self._peers.items() if n != name]

    def all_tokens(self) -> list[object]:
        with self._lock:
            return list(self._peers.values())

    def handle_line(self, name: str | None, text: str) -> tuple[str | None, list[str]]:
        """
        Process one inbound line from a client.
        Returns (new_name, reply_lines_to_sender).
        Side-effect replies to other peers are returned via broadcast_roster().
        """
        if name is None:
            if text.startswith("HELLO "):
                new_name = text[6:].strip()
                if not new_name:
                    return None, ["ERR bad HELLO"]
                return new_name, []
            return None, []

        if text == "PING":
            return name, ["PONG"]
        if text == "PEERS":
            return name, [f"PEERS {self.roster()}"]
        if text.startswith("MSG "):
            parts = text.split(" ", 2)
            if len(parts) < 3:
                return name, ["ERR bad MSG"]
            dst, payload = parts[1], parts[2]
            if self.get_token(dst) is None:
                return name, [f"ERR no peer {dst}"]
            return name, [f"__deliver__:{dst}:{name}:{payload}", "OK"]
        if text.startswith("BROADCAST "):
            payload = text[10:]
            targets = self.other_tokens(name)
            lines = [f"__deliver__:*:{name}:{payload}"] * len(targets)
            lines.append("OK")
            return name, lines
        targets = self.other_tokens(name)
        lines = [f"__deliver__:*:{name}:{text}"] * len(targets)
        return name, lines

    def expand_deliveries(
        self, name: str, replies: list[str], send: SendFn, sock: socket.socket
    ) -> list[str]:
        """Turn internal __deliver__ markers into real sends; return user-visible replies."""
        out: list[str] = []
        for line in replies:
            if line.startswith("__deliver__:"):
                _, dst, src, payload = line.split(":", 3)
                if dst == "*":
                    for token in self.other_tokens(name):
                        peer_sock = token  # type: ignore[assignment]
                        send(peer_sock, f"FROM {src} {payload}")
                else:
                    peer_sock = self.get_token(dst)
                    if peer_sock is not None:
                        send(peer_sock, f"FROM {src} {payload}")  # type: ignore[arg-type]
            else:
                out.append(line)
        return out


def send_line(sock: socket.socket, line: str) -> None:
    sock.sendall((line + "\n").encode())


def broadcast_roster(hub: RotosHub, send: SendFn = send_line) -> None:
    line = f"PEERS {hub.roster()}"
    for sock in hub.all_tokens():
        send(sock, line)  # type: ignore[arg-type]


def handle_client(hub: RotosHub, conn: socket.socket, addr) -> None:
    name = None
    buf = b""
    try:
        while True:
            chunk = conn.recv(4096)
            if not chunk:
                break
            buf += chunk
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                text = line.decode(errors="replace").strip()
                if not text:
                    continue
                prev = name
                name, replies = hub.handle_line(name, text)
                if name is not None and prev is None:
                    hub.join(name, conn)
                    print(f"hub: {name} joined from {addr}", file=sys.stderr)
                    send_line(conn, f"PEERS {hub.roster()}")
                    broadcast_roster(hub)
                    continue
                replies = hub.expand_deliveries(name or "", replies, send_line, conn)
                for r in replies:
                    send_line(conn, r)
    finally:
        if name and hub.leave(name, conn):
            print(f"hub: {name} left", file=sys.stderr)
            broadcast_roster(hub)
        conn.close()


def start_server(host: str, port: int) -> tuple[threading.Thread, socket.socket, list[bool]]:
    """Start accept loop in a background thread; caller closes srv to stop."""
    hub = RotosHub()
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((host, port))
    srv.listen(16)
    running = [True]

    def loop() -> None:
        srv.settimeout(0.25)
        while running[0]:
            try:
                conn, addr = srv.accept()
            except socket.timeout:
                continue
            except OSError:
                break
            threading.Thread(
                target=handle_client, args=(hub, conn, addr), daemon=True
            ).start()

    thread = threading.Thread(target=loop, daemon=True)
    thread.start()
    return thread, srv, running


def serve(host: str, port: int) -> None:
    thread, srv, running = start_server(host, port)
    print(f"rotos_link_hub: listening on {host}:{port}", file=sys.stderr)
    try:
        thread.join()
    except KeyboardInterrupt:
        running[0] = False
        srv.close()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=7100)
    args = ap.parse_args()
    serve(args.host, args.port)
    return 0


if __name__ == "__main__":
    sys.exit(main() or 0)
