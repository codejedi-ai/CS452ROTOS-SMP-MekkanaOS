#include "mailbox_ipi.h"
#include "gic.h"
#include "syscall.h"
#include "kernel_state.h"
#include "config.h"

void mailbox_ipi_init(void)
{
	for (int core = 0; core < NUM_CORES; core++) {
		uint32_t sgi = (uint32_t)MAILBOX_SGI_FOR_CORE(core);
		route_interrupt(sgi, (uint8_t)core);
		enable_interrupt(sgi);
	}
}

void mailbox_ipi_signal(int dest_core)
{
	if (dest_core < 0 || dest_core >= NUM_CORES)
		return;
	uint32_t sgi = (uint32_t)MAILBOX_SGI_FOR_CORE(dest_core);
	gic_send_sgi(sgi, (uint8_t)(1u << dest_core));
}

int mailbox_ipi_handle_async(uint32_t interruptid)
{
	if (!mailbox_ipi_is_event(interruptid))
		return 0;

	if (!unblock_return(interruptid, (uint64_t)interruptid)) {
		AWAIT_INTERRUPT[interruptid].event_q[AWAIT_INTERRUPT[interruptid].eventq_tail] =
		    (uint64_t)interruptid;
		AWAIT_INTERRUPT[interruptid].eventq_tail++;
		AWAIT_INTERRUPT[interruptid].eventq_tail %= NUMPROCS;
		AWAIT_INTERRUPT[interruptid].eventq_len++;
	}
	return 1;
}
