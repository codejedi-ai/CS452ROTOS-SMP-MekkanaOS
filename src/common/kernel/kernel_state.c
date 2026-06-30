#include "kernel_state.h"

struct process PROCS_BANK[NUM_CORES][NUMPROCS];
struct state READY_QUEUE_BANK[NUM_CORES][NUMPROCS];
struct state BLOCKED_LIST_BANK[NUM_CORES][NUMPROCS];
struct interrupt AWAIT_INTERRUPT_BANK[NUM_CORES][MAXEVENT];
uint32_t PID_BANK[NUM_CORES];
void *STACKSTART_BANK[NUM_CORES];
uint32_t kernelStartTime = 0;
