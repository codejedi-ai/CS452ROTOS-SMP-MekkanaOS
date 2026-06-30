#ifndef KERNEL_STATE_H
#define KERNEL_STATE_H

#include "syscall.h"
#include "config.h"
#include "smp.h"

extern struct process PROCS_BANK[NUM_CORES][NUMPROCS];
extern struct state READY_QUEUE_BANK[NUM_CORES][NUMPROCS];
extern struct state BLOCKED_LIST_BANK[NUM_CORES][NUMPROCS];
extern struct interrupt AWAIT_INTERRUPT_BANK[NUM_CORES][MAXEVENT];
extern uint32_t PID_BANK[NUM_CORES];
extern void *STACKSTART_BANK[NUM_CORES];
extern uint32_t kernelStartTime;

#define PROCS          (PROCS_BANK[smp_get_core_id()])
#define READY_QUEUE    (READY_QUEUE_BANK[smp_get_core_id()])
#define BLOCKED_LIST   (BLOCKED_LIST_BANK[smp_get_core_id()])
#define AWAIT_INTERRUPT (AWAIT_INTERRUPT_BANK[smp_get_core_id()])
#define PID            (PID_BANK[smp_get_core_id()])
#define STACKSTART     (STACKSTART_BANK[smp_get_core_id()])

#endif
