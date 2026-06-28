#ifndef _K4_H_
#define _K4_H_ 1

/*
 * CS 452 K4 — IO server + Marklin.
 *
 * Adds Putc/Getc syscalls backed by the io_TXIC/io_RXIC servers, plus the
 * Marklin worker that drives the train controller over UART (or AUX in
 * QEMU). For now this only sanity-checks that the display server is
 * reachable -- full Marklin integration tests run via the TC1 demo.
 */

int k4_self_tests(void);

#endif
