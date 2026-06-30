#include "mailbox.h"
#include "mailbox_ipi.h"
#include "spinlock.h"
#include "smp.h"
#include <stddef.h>

/*
 * One mailbox per core (4 total).  Spinlock serializes concurrent writers
 * to the same inbound mailbox.  Wheel-and-hub: spokes post to core 0;
 * hub forwards to the destination core's mailbox.
 */
static MailboxSlot g_mailboxes[MAILBOX_COUNT] __attribute__((aligned(64)));

static void dmb_ish(void)
{
	__asm__ volatile("dmb ish" ::: "memory");
}

static void dsb_sy(void)
{
	__asm__ volatile("dsb sy" ::: "memory");
}

static int mailbox_valid_core(int core_id)
{
	return core_id >= 0 && core_id < NUM_CORES;
}

static int mailbox_post(int route_core, int final_dest_core, int dest_tid,
                        int src_core, int src_tid, const void *msg, int len)
{
	if (!mailbox_valid_core(route_core) || msg == NULL || len <= 0 ||
	    len > MAILBOX_PAYLOAD_MAX)
		return -2;
	if (route_core == src_core)
		return -3;

	MailboxSlot *mb = &g_mailboxes[route_core];

	spinlock_lock(&mb->lock);
	if (mb->state != MAILBOX_EMPTY) {
		spinlock_unlock(&mb->lock);
		return -1;
	}

	mb->src_core = (uint32_t)src_core;
	mb->src_tid = (uint32_t)src_tid;
	mb->final_dest_core = (uint32_t)final_dest_core;
	mb->dest_tid = (uint32_t)dest_tid;
	mb->len = (uint32_t)len;
	for (int i = 0; i < len; i++)
		mb->payload[i] = ((const char *)msg)[i];

	dmb_ish();
	mb->state = MAILBOX_READY;
	spinlock_unlock(&mb->lock);
	dsb_sy();

	mailbox_ipi_signal(route_core);
	return len;
}

void mailbox_init(void)
{
	for (int i = 0; i < MAILBOX_COUNT; i++) {
		spinlock_init(&g_mailboxes[i].lock);
		g_mailboxes[i].state = MAILBOX_EMPTY;
		g_mailboxes[i].len = 0;
		g_mailboxes[i].final_dest_core = 0;
		g_mailboxes[i].dest_tid = 0;
	}
	dsb_sy();
}

int MailboxSend(int dest_core, int dest_tid, int src_tid, const void *msg, int len)
{
	int src_core = (int)smp_get_core_id();
	int route_core = mailbox_route_dest(src_core, dest_core);

	return mailbox_post(route_core, dest_core, dest_tid, src_core, src_tid, msg, len);
}

int MailboxForward(int dest_core, int dest_tid, int src_core, int src_tid,
                   const void *msg, int len)
{
	if ((int)smp_get_core_id() != SMP_HUB_CORE)
		return -4;

	return mailbox_post(dest_core, dest_core, dest_tid, src_core, src_tid, msg, len);
}

int MailboxTryReceive(int *src_core, int *src_tid, int *final_dest_core,
                      int *dest_tid, void *msg, int *len)
{
	int core = (int)smp_get_core_id();
	if (!mailbox_valid_core(core))
		return -2;

	MailboxSlot *mb = &g_mailboxes[core];

	spinlock_lock(&mb->lock);
	if (mb->state != MAILBOX_READY) {
		spinlock_unlock(&mb->lock);
		return -1;
	}

	if (src_core)
		*src_core = (int)mb->src_core;
	if (src_tid)
		*src_tid = (int)mb->src_tid;
	if (final_dest_core)
		*final_dest_core = (int)mb->final_dest_core;
	if (dest_tid)
		*dest_tid = (int)mb->dest_tid;
	if (len)
		*len = (int)mb->len;
	if (msg && len && *len > 0) {
		int n = *len;
		if (n > MAILBOX_PAYLOAD_MAX)
			n = MAILBOX_PAYLOAD_MAX;
		for (int i = 0; i < n; i++)
			((char *)msg)[i] = mb->payload[i];
	}

	dmb_ish();
	mb->state = MAILBOX_EMPTY;
	spinlock_unlock(&mb->lock);
	dmb_ish();
	return 0;
}
