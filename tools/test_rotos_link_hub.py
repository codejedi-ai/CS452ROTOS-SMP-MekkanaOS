#!/usr/bin/env python3
"""Unit + integration tests for rotos_link_hub (peer discovery, routing, timeouts)."""

from __future__ import annotations

import socket
import threading
import time
import unittest

from rotos_link_hub import RotosHub, broadcast_roster, send_line, start_server


def recv_line(sock: socket.socket, timeout: float = 2.0) -> str:
    sock.settimeout(timeout)
    buf = b""
    while b"\n" not in buf:
        chunk = sock.recv(4096)
        if not chunk:
            raise ConnectionError("connection closed")
        buf += chunk
    return buf.split(b"\n", 1)[0].decode().strip()


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


class RotosHubLogicTest(unittest.TestCase):
    def test_hello_registers_name(self) -> None:
        hub = RotosHub()
        name, replies = hub.handle_line(None, "HELLO DarcyOS")
        self.assertEqual(name, "DarcyOS")
        self.assertEqual(replies, [])
        hub.join("DarcyOS", 1)
        self.assertEqual(hub.roster(), "DarcyOS")

    def test_ping_pong(self) -> None:
        hub = RotosHub()
        hub.join("A", "sock-a")
        _, replies = hub.handle_line("A", "PING")
        self.assertEqual(replies, ["PONG"])

    def test_peers_command(self) -> None:
        hub = RotosHub()
        hub.join("A", 1)
        hub.join("B", 2)
        _, replies = hub.handle_line("A", "PEERS")
        self.assertEqual(replies, ["PEERS A B"])

    def test_msg_ok_and_delivery(self) -> None:
        hub = RotosHub()
        hub.join("A", 1)
        hub.join("B", 2)
        _, replies = hub.handle_line("A", "MSG B hello")
        self.assertIn("__deliver__:B:A:hello", replies)
        self.assertIn("OK", replies)

    def test_msg_missing_peer(self) -> None:
        hub = RotosHub()
        hub.join("A", 1)
        _, replies = hub.handle_line("A", "MSG Missing hi")
        self.assertEqual(replies, ["ERR no peer Missing"])

    def test_broadcast(self) -> None:
        hub = RotosHub()
        hub.join("A", 1)
        hub.join("B", 2)
        hub.join("C", 3)
        _, replies = hub.handle_line("A", "BROADCAST all")
        deliver = [r for r in replies if r.startswith("__deliver__")]
        self.assertEqual(len(deliver), 2)
        self.assertIn("OK", replies)

    def test_leave_updates_roster(self) -> None:
        hub = RotosHub()
        hub.join("A", 1)
        hub.join("B", 2)
        self.assertTrue(hub.leave("A", 1))
        self.assertEqual(hub.roster(), "B")


class RotosHubTcpTest(unittest.TestCase):
    def setUp(self) -> None:
        self.port = free_port()
        self.thread, self.srv, self.running = start_server("127.0.0.1", self.port)
        time.sleep(0.15)

    def tearDown(self) -> None:
        self.running[0] = False
        self.srv.close()
        self.thread.join(timeout=2.0)
        for sock in getattr(self, "_sockets", []):
            sock.close()

    def _track(self, sock: socket.socket) -> socket.socket:
        if not hasattr(self, "_sockets"):
            self._sockets = []
        self._sockets.append(sock)
        return sock

    def connect(self, os_name: str, timeout: float = 2.0) -> socket.socket:
        sock = socket.create_connection(("127.0.0.1", self.port), timeout=timeout)
        self._track(sock)
        send_line(sock, f"HELLO {os_name}")
        line = recv_line(sock, timeout)
        self.assertTrue(line.startswith("PEERS "))
        self.assertIn(os_name, line.split()[1:])
        return sock

    def test_two_peers_discovery_broadcast(self) -> None:
        a = self.connect("DarcyOS")
        b = self.connect("IrisOS")
        # second join broadcasts updated roster to everyone
        line = recv_line(a, 2.0)
        self.assertTrue(line.startswith("PEERS "))
        names = line.split()[1:]
        self.assertEqual(sorted(names), ["DarcyOS", "IrisOS"])

    def test_ping_roundtrip(self) -> None:
        sock = self.connect("PrimeOS")
        send_line(sock, "PING")
        self.assertEqual(recv_line(sock), "PONG")

    def test_msg_between_peers(self) -> None:
        a = self.connect("DarcyOS")
        b = self.connect("MekkanaOS")
        recv_line(a)  # PEERS broadcast from B join
        send_line(a, "MSG MekkanaOS hi")
        self.assertEqual(recv_line(a), "OK")
        self.assertEqual(recv_line(b), "FROM DarcyOS hi")

    def test_connect_timeout_no_hub(self) -> None:
        dead_port = free_port()
        with self.assertRaises((TimeoutError, OSError)):
            socket.create_connection(("127.0.0.1", dead_port), timeout=0.3)


if __name__ == "__main__":
    unittest.main()
