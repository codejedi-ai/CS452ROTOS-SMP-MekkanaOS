#ifndef _K1_H_
#define _K1_H_ 1

/*
 * CS 452 K1 — Kernel Primitives.
 *
 * Required syscalls: Create, MyTid, MyParentTid, Yield, Exit.
 * Demo: the "first user task" creates four children (two at lower priority,
 * two at higher), each child prints tid/parent_tid, yields, prints again,
 * then exits.
 *
 * k1_first_user_task() implements the demo task.
 * k1_self_tests()       runs the regression tests for K1 primitives, prints
 *                       PASS/FAIL per check, returns number of failures.
 */

void k1_first_user_task(void);
int  k1_self_tests(void);

#endif
