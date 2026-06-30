#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

/* ----- Scheduler priorities (uint8_t; lower number = runs sooner) ----- */
#define SCHED_LOWEST_PRIORITY  255   /* idle — only scheduled when nothing else is ready */

/* ----- SMP multikernel (4 identical kernels, one per core) ----- */
#define BUILD_SMP              1
#define NUM_CORES              4
#define MAILBOX_COUNT          NUM_CORES   /* one spinlock-protected mailbox per core */

#define MAILBOX_PAYLOAD_MAX    64
#define MAILBOX_EMPTY          0u
#define MAILBOX_READY          1u

/* GIC SGI ids for mailbox wake — one per target core (ARM SGI 0–15). */
#define MAILBOX_SGI_CORE0      8
#define MAILBOX_SGI_CORE1      9
#define MAILBOX_SGI_CORE2      10
#define MAILBOX_SGI_CORE3      11
#define MAILBOX_SGI_FOR_CORE(c) (MAILBOX_SGI_CORE0 + (c))

/* Wheel-and-hub SMP: core 0 is the only inter-core router. */
#define SMP_HUB_CORE           0

#endif /* _CONFIG_H_ */
