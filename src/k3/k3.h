#ifndef _K3_H_
#define _K3_H_ 1

/*
 * CS 452 K3 — Clock server + interrupts.
 *
 * Adds AwaitEvent for interrupt handling, plus the clock_server with
 * Time / Delay / DelayUntil. Verifies the timer interrupt path works.
 *
 * NOTE: as of writing, the BCM2711 system-timer IRQ does not route through
 * the GIC under our QEMU raspi4b setup, so k3_self_tests() will FAIL until
 * the interrupt layer is fixed. K1+K2 still pass independently.
 */

int k3_self_tests(void);

#endif
