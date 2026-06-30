#include "rotos_link.h"
#include "nameserver.h"
#include "clockserver.h"
#include "syscall.h"
#include "auxuart.h"
#include "config.h"
#include "rpi.h"

#define LINK_BUF            128
#define LINK_TIMEOUT_TICKS  100   /* ~1s hub reply timeout when hub is down */
#define LINK_BOOT_TIMEOUT   100   /* wait for initial PEERS after HELLO */

typedef enum {
	LINK_MSG_RX   = 1,
	LINK_MSG_SEND = 2,
} LinkMsgType;

typedef struct {
	int  type;
	int  len;
	char buf[LINK_BUF];
} LinkMsg;

static int  rotos_link_tid = -1;
static int  hub_up = 0;
static int  boot_wait = 1;
static char peer_roster[ROTOS_LINK_REPLY_MAX];

int RotosLinkTid(void) { return rotos_link_tid; }

int LinkHubUp(void) { return hub_up; }

int LinkGetPeers(char *buf, int max)
{
	int n = 0;
	while (peer_roster[n] && n < max - 1) {
		buf[n] = peer_roster[n];
		n++;
	}
	buf[n] = 0;
	return n;
}

static int l_put(char *d, int max, const char *s)
{
	int n = 0;
	while (s[n] && n < max - 1) {
		d[n] = s[n];
		n++;
	}
	d[n] = 0;
	return n;
}

static int starts_with(const char *s, const char *pfx)
{
	for (int i = 0; pfx[i]; i++) {
		if (s[i] != pfx[i])
			return 0;
	}
	return 1;
}

static void store_peers(const char *roster)
{
	int n = 0;
	while (roster[n] && n < ROTOS_LINK_REPLY_MAX - 1) {
		peer_roster[n] = roster[n];
		n++;
	}
	peer_roster[n] = 0;
}

static void update_peers_line(const char *line)
{
	if (!starts_with(line, "PEERS "))
		return;
	hub_up = 1;
	boot_wait = 0;
	store_peers(line + 6);
}

static void link_write_line(const char *s)
{
	for (int i = 0; s[i]; i++)
		auxuart_putc((unsigned char)s[i]);
	auxuart_putc('\n');
}

static void deliver_inbound_line(const char *line)
{
	if (line[0] == '\0')
		return;
	update_peers_line(line);
	uart_printf(CONSOLE, "[LAN] %s\r\n", line);
}

static void check_boot_timeout(int clock, int boot_deadline)
{
	if (boot_wait && Time(clock) >= boot_deadline) {
		boot_wait = 0;
		hub_up = 0;
		uart_printf(CONSOLE, "[LAN] hub unavailable (timeout)\r\n");
	}
}

static int wait_clock_tid(void)
{
	for (int i = 0; i < 200; i++) {
		int tid = WhoIs("clock_server");
		if (tid > 0)
			return tid;
		Yield();
	}
	return -1;
}

static void rotos_link_notifier(void)
{
	int server = rotos_link_tid;
	int clock  = wait_clock_tid();
	LinkMsg m;
	int ack;

	for (;;) {
		int n = 0, b;
		while (n < LINK_BUF && (b = auxuart_getc_nb()) >= 0)
			m.buf[n++] = (char)b;
		m.type = LINK_MSG_RX;
		m.len  = n;
		Send(server, (const char *)&m, (int)sizeof(m), (char *)&ack, (int)sizeof(ack));
		if (n == 0)
			Delay(clock, 1);
	}
}

void rotos_link_server(void)
{
	int tid;
	LinkMsg m;
	int ack = 0;
	int clock;
	int client = -1;
	int deadline = 0;
	int boot_deadline = 0;
	char line[ROTOS_LINK_REPLY_MAX + 1];
	int llen = 0;
	char hello[64];

	peer_roster[0] = 0;
	rotos_link_tid = MyTid();
	RegisterAs(ROTOS_LINK_SERVER_NAME);
	auxuart_init();
	clock = wait_clock_tid();
	Create(ROTOS_LINK_NOTIFIER_PRIORITY, rotos_link_notifier);

	hello[0] = 0;
	{
		int n = l_put(hello, (int)sizeof(hello), "HELLO ");
		l_put(hello + n, (int)sizeof(hello) - n, ROTOS_OS_NAME);
	}
	link_write_line(hello);
	boot_deadline = Time(clock) + LINK_BOOT_TIMEOUT;

	for (;;) {
		Receive(&tid, (char *)&m, (int)sizeof(m));

		if (m.type == LINK_MSG_RX) {
			check_boot_timeout(clock, boot_deadline);
			for (int i = 0; i < m.len; i++) {
				char ch = m.buf[i];
				if (ch == '\r')
					continue;
				if (ch == '\n') {
					line[llen] = 0;
					if (client >= 0) {
						update_peers_line(line);
						Reply(client, line, llen + 1);
						client = -1;
					} else {
						deliver_inbound_line(line);
					}
					llen = 0;
				} else if (llen < ROTOS_LINK_REPLY_MAX) {
					line[llen++] = ch;
				} else {
					llen = 0;
				}
			}
			if (client >= 0 && Time(clock) >= deadline) {
				hub_up = 0;
				Reply(client, LINK_HUB_TIMEOUT_MSG, (int)sizeof(LINK_HUB_TIMEOUT_MSG));
				client = -1;
				llen = 0;
			}
			Reply(tid, &ack, (int)sizeof(ack));
		} else {
			if (client >= 0) {
				char bz[] = "(busy)";
				Reply(tid, bz, (int)sizeof(bz));
			} else if (!hub_up && !boot_wait) {
				char off[] = LINK_HUB_OFFLINE_MSG;
				Reply(tid, off, (int)sizeof(off));
			} else {
				client = tid;
				llen = 0;
				for (int i = 0; m.buf[i] && i < ROTOS_LINK_CMD_MAX; i++)
					auxuart_putc((unsigned char)m.buf[i]);
				auxuart_putc('\n');
				deadline = Time(clock) + LINK_TIMEOUT_TICKS;
			}
		}
	}
}

int LinkSend(const char *cmd, char *out, int outmax)
{
	LinkMsg req;
	int n;

	if (rotos_link_tid < 0) {
		return l_put(out, outmax, "(no link)");
	}
	req.type = LINK_MSG_SEND;
	req.len  = l_put(req.buf, (int)sizeof(req.buf), cmd);
	n = Send(rotos_link_tid, (const char *)&req, (int)sizeof(req), out, outmax);
	if (n < 0) {
		out[0] = 0;
		return 0;
	}
	if (n > 0 && n <= outmax)
		out[n - 1] = 0;
	else if (outmax > 0)
		out[outmax - 1] = 0;
	return n > 0 ? n - 1 : 0;
}
