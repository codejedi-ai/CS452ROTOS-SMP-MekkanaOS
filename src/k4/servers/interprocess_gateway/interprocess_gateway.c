#include "interprocess_gateway.h"
#include "mailbox.h"
#include "mailbox_ipi.h"
#include "syscall.h"
#include "nameserver.h"
#include "smp.h"
#include "config.h"

/*
 * Spoke gateway on cores 1–3: one inbound mailbox per core, fed by the hub.
 */
void interprocess_gateway(void)
{
	RegisterAs(INTERPROCESS_GATEWAY_NAME);

	const int mailbox_event = MAILBOX_SGI_FOR_CORE((int)smp_get_core_id());

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

		(void)src_core;
		(void)src_tid;
		(void)final_dest_core;

		if (dest_tid > 0) {
			char reply[8];
			Send(dest_tid, delivery.payload, delivery.len, reply,
			     (int)sizeof(reply));
		}
	}
}
