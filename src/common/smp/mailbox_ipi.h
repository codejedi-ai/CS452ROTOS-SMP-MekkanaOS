#ifndef MAILBOX_IPI_H
#define MAILBOX_IPI_H

#include <stdint.h>
#include "config.h"

void mailbox_ipi_init(void);
void mailbox_ipi_signal(int dest_core);
int mailbox_ipi_handle_async(uint32_t interruptid);

static inline int mailbox_ipi_is_event(uint32_t interruptid)
{
	return interruptid >= (uint32_t)MAILBOX_SGI_CORE0 &&
	       interruptid < (uint32_t)MAILBOX_SGI_CORE0 + (uint32_t)NUM_CORES;
}

#endif /* MAILBOX_IPI_H */
