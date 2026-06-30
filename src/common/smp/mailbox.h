#ifndef SMP_MAILBOX_H

#define SMP_MAILBOX_H



#include "config.h"

#include "spinlock.h"

#include <stdint.h>



typedef struct {

	spinlock_t lock;

	volatile uint32_t state;

	uint32_t src_core;

	uint32_t src_tid;

	uint32_t final_dest_core;

	uint32_t dest_tid;

	uint32_t len;

	char payload[MAILBOX_PAYLOAD_MAX];

} MailboxSlot;



typedef struct {

	int src_core;

	int src_tid;

	int final_dest_core;

	int dest_tid;

	int len;

	char payload[MAILBOX_PAYLOAD_MAX];

} CrossCoreDelivery;

/* Hub routing: spokes always reach another spoke via core SMP_HUB_CORE. */
static inline int mailbox_route_dest(int src_core, int dest_core)
{
	if (src_core == SMP_HUB_CORE || dest_core == SMP_HUB_CORE)
		return dest_core;
	return SMP_HUB_CORE;
}

void mailbox_init(void);



int MailboxSend(int dest_core, int dest_tid, int src_tid,

                const void *msg, int len);

int MailboxForward(int dest_core, int dest_tid, int src_core, int src_tid,

                   const void *msg, int len);

int MailboxTryReceive(int *src_core, int *src_tid, int *final_dest_core,

                      int *dest_tid, void *msg, int *len);



#endif /* SMP_MAILBOX_H */


