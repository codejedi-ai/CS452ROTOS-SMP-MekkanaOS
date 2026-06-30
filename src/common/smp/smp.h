#ifndef SMP_H
#define SMP_H

#include <stdint.h>
#include "config.h"

extern volatile uint32_t smp_boot_ready;

uint32_t smp_get_core_id(void);
void smp_set_core_id(uint32_t core_id);
void smp_init_cpu(void);
void smp_boot_secondary_cores(void);
void smp_wait_boot_ready(void);
void smp_signal_boot_ready(void);
void smp_secondary_kmain(unsigned core_id);
void smp_schedule_secondary_release(void);

#endif /* SMP_H */
