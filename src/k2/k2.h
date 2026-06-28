#ifndef _K2_H_
#define _K2_H_ 1

/*
 * CS 452 K2 — Inter-task messaging + nameserver.
 *
 * Adds Send / Receive / Reply syscalls (synchronous, blocking message
 * passing), plus the nameserver task with RegisterAs / WhoIs.
 *
 * k2_self_tests() returns the number of failures (0 = clean).
 */

int k2_self_tests(void);

#endif
