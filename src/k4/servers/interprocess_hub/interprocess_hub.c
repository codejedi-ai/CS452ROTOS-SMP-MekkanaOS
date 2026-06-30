#include "interprocess_hub.h"
#include "mailbox.h"
#include "mailbox_ipi.h"
#include "syscall.h"
#include "nameserver.h"
#include "smp.h"
#include "config.h"
#include "rpi.h"

/*
 * Central hub on core 0 (wheel-and-hub SMP).
 * Cores 1–3 post to core 0's single mailbox; hub forwards or delivers locally.
 */
void interprocess_hub(void)
{
	RegisterAs(INTERPROCESS_HUB_NAME);

	const int mailbox_event = MAILBOX_SGI_FOR_CORE(SMP_HUB_CORE);

	for (;;) {
		(void)AwaitEvent(mailbox_event);

		CrossCoreDelivery delivery;
		int src_core = 0;
		int src_tid = 0;
		int final_dest_core = 0;
		int dest_tid = 0;
		int len = 0;

		if (MailboxTryReceive(&src_core, &src_tid, &final_dest_core,
		                      &dest_tid, delivery.payload, &len) != 0)
			continue;

		delivery.src_core = src_core;
		delivery.src_tid = src_tid;
		delivery.final_dest_core = final_dest_core;
		delivery.dest_tid = dest_tid;
		delivery.len = len;

		uart_printf(CONSOLE,
		            "[HUB] core%u tid%u -> core%u tid%u (%d bytes)\r\n",
		            (unsigned)delivery.src_core, (unsigned)delivery.src_tid,
		            (unsigned)delivery.final_dest_core,
		            (unsigned)delivery.dest_tid, delivery.len);

		if (final_dest_core == SMP_HUB_CORE) {
			if (dest_tid > 0) {
				char reply[8];
				Send(dest_tid, delivery.payload, delivery.len, reply,
				     (int)sizeof(reply));
			}
			continue;
		}

		(void)MailboxForward(final_dest_core, dest_tid, src_core, src_tid,
		                     delivery.payload, delivery.len);
	}
}
