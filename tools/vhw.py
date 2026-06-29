#!/usr/bin/env python3
"""
Virtual Marklin 6051 control box.

Listens on a TCP socket that QEMU connects to as its second serial port
(`-serial tcp:127.0.0.1:6011`). Speaks just enough of the Marklin protocol
that the kernel's marklin_worker can drive trains and read s88 sensor
bytes against synthetic train physics.

Protocol summary (Marklin 6051 over 8N2 @ 2400 baud):
  * train speed:    [speed_byte (0..14, +16 for lights), train_id]
  * train reverse:  [15, train_id]    (host stops, sends 15, resends speed)
  * switch:         [33|34, sw_id] then [32] to cut power
  * read all s88:   [128 + n_modules]  -> 2*n_modules bytes back (two halves)
  * read one s88:   [192 + s88_id]     -> 2 bytes back

The state model here is a single ring of length LOOP_LEN_MM that the train
runs around at a speed proportional to its setting. Sensors fire on
crossing fixed positions. Good enough to drive the shell and the stop-at
demo without real hardware.
"""

from __future__ import annotations
import argparse, socket, threading, time, sys

DEFAULT_PORT      = 6011
TICK_SEC          = 0.01           # 100 Hz physics step
LOOP_LEN_MM       = 4894
SPEED_MM_PER_S    = {              # synthetic; matches the kernel's seed
     0:0,  1: 30, 2: 60, 3: 95, 4:135, 5:175, 6:220, 7:265,
     8:305,9:335,10:350,11:380,12:410,13:440,14:470,
}
SENSOR_POSITIONS_MM = [             # s88 0, sensor 1..16 round the loop
    int(LOOP_LEN_MM * i / 16) for i in range(16)
]

class Train:
    def __init__(self, tid: int):
        self.tid = tid
        self.speed = 0
        self.pos_mm = 0.0
        self.lights = False

    def step(self):
        v = SPEED_MM_PER_S.get(self.speed, 0)
        self.pos_mm = (self.pos_mm + v * TICK_SEC) % LOOP_LEN_MM


class World:
    """Holds train + sensor state; produces s88 bytes on demand."""
    def __init__(self):
        self.trains: dict[int, Train] = {}
        self.lock = threading.Lock()
        self.sensor_state = [0] * 16
        self.prev_sensor_pos = [0.0] * 16

    def get_train(self, tid: int) -> Train:
        if tid not in self.trains:
            self.trains[tid] = Train(tid)
        return self.trains[tid]

    def physics(self):
        while True:
            time.sleep(TICK_SEC)
            with self.lock:
                # advance all trains
                for t in self.trains.values():
                    prev = t.pos_mm
                    t.step()
                    # sensor crossings: edge-triggered, single pulse
                    for i, sp in enumerate(SENSOR_POSITIONS_MM):
                        if self._crossed(prev, t.pos_mm, sp):
                            self.sensor_state[i] = 1

    @staticmethod
    def _crossed(a: float, b: float, target: float) -> bool:
        if b >= a:
            return a < target <= b
        # wrapped
        return target > a or target <= b

    def read_s88_bytes(self, modules: int) -> bytes:
        """Two bytes per module (high, low). Single-module emulation here."""
        with self.lock:
            buf = bytearray()
            for m in range(modules):
                hi = 0
                lo = 0
                if m == 0:
                    for i in range(8):
                        if self.sensor_state[i]:
                            hi |= 1 << (7 - i)
                    for i in range(8, 16):
                        if self.sensor_state[i]:
                            lo |= 1 << (15 - i)
                buf.append(hi)
                buf.append(lo)
            # Marklin clears sensors after a read.
            for i in range(16):
                self.sensor_state[i] = 0
            return bytes(buf)


def handle_client(conn: socket.socket, world: World):
    buf = bytearray()
    pending_switch_off = False
    while True:
        chunk = conn.recv(1024)
        if not chunk:
            return
        buf.extend(chunk)
        while buf:
            b = buf[0]
            if 0 <= b <= 14 or b == 16 or (16 <= b <= 30):
                # speed byte; need a train-id byte to follow
                if len(buf) < 2: break
                speed, tid = buf[0], buf[1]
                del buf[:2]
                tr = world.get_train(tid)
                tr.speed = speed & 0x0F
                tr.lights = bool(speed & 0x10)
                print(f"vhw: train {tid} -> speed {tr.speed}{' L' if tr.lights else ''}",
                      file=sys.stderr)
            elif b == 15:
                if len(buf) < 2: break
                _, tid = buf[0], buf[1]
                del buf[:2]
                print(f"vhw: train {tid} -> reverse", file=sys.stderr)
            elif b == 32:
                del buf[:1]                # solenoid off; nothing to do
            elif b in (33, 34):
                if len(buf) < 2: break
                cmd, sw = buf[0], buf[1]
                del buf[:2]
                print(f"vhw: switch {sw} -> {'S' if cmd==33 else 'C'}",
                      file=sys.stderr)
            elif 128 <= b <= 159:
                n = b - 128
                del buf[:1]
                conn.sendall(world.read_s88_bytes(n))
            elif 192 <= b <= 223:
                del buf[:1]
                conn.sendall(world.read_s88_bytes(1))
            else:
                print(f"vhw: unknown byte 0x{b:02x}", file=sys.stderr)
                del buf[:1]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=DEFAULT_PORT)
    ap.add_argument("--host", default="127.0.0.1")
    args = ap.parse_args()

    world = World()
    threading.Thread(target=world.physics, daemon=True).start()

    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((args.host, args.port))
    srv.listen(1)
    print(f"vhw: listening on {args.host}:{args.port}", file=sys.stderr)
    while True:
        conn, addr = srv.accept()
        print(f"vhw: connection from {addr}", file=sys.stderr)
        try:
            handle_client(conn, world)
        except (ConnectionResetError, BrokenPipeError):
            pass
        finally:
            conn.close()
            print("vhw: client disconnected", file=sys.stderr)


if __name__ == "__main__":
    sys.exit(main() or 0)
