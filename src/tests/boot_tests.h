#ifndef _BOOT_TESTS_H_
#define _BOOT_TESTS_H_ 1

/*
 * Boot self-tests.
 *
 * On every kernel boot we run a layered suite that exercises:
 *   1. context switch / Yield               (kernel primitive)
 *   2. send / receive ping-pong              (IPC primitive)
 *   3. nameserver round-trip                 (RegisterAs + WhoIs)
 *   4. clock server (Time + Delay)
 *   5. rock-paper-scissors with referee      (multi-task IPC)
 *   6. display_server output                 (service)
 *
 * Each test prints "[BOOT-TEST N] PASS|FAIL <reason>".
 * The runner continues past failures so the operator sees the full picture.
 */

void boot_test_runner(void);

#endif /* _BOOT_TESTS_H_ */
