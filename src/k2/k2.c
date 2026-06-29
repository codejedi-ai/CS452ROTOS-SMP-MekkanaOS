#include "k2.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"

/* --- send/receive ping-pong -------------------------------------------- */
static volatile int k2_echo_result;

static void k2_echo_child(void)
{
    int from;
    int msg = 0;
    Receive(&from, (char *)&msg, sizeof(msg));
    msg += 100;
    Reply(from, (char *)&msg, sizeof(msg));
    Exit();
}

static int k2_test_send_receive(void)
{
    k2_echo_result = 0;
    int child = Create(0, k2_echo_child);   /* higher prio than us */
    if (child <= 0) {
        uart_printf(CONSOLE, "\033[31mK2: Create echo_child=%d\033[37m\r\n", child);
        return 1;
    }
    int snd = 42, rep = 0;
    Send(child, (char *)&snd, sizeof(snd), (char *)&rep, sizeof(rep));
    k2_echo_result = rep;
    if (k2_echo_result != 142) {
        uart_printf(CONSOLE, "\033[31mK2: ping-pong got %d expected 142\033[37m\r\n",
                    k2_echo_result);
        return 1;
    }
    return 0;
}

/* --- nameserver round-trip --------------------------------------------- */
static int k2_test_nameserver(void)
{
    RegisterAs("k2_test_owner");
    int found = WhoIs("k2_test_owner");
    if (found <= 0) {
        uart_printf(CONSOLE, "\033[31mK2: WhoIs returned %d\033[37m\r\n", found);
        return 1;
    }
    return 0;
}

/* --- rock paper scissors (multi-Send to one Receive queue) ------------- */
static volatile int k2_rps_replies;
static int k2_rps_referee_tid;

struct k2_rps_msg { int player_id; int move; };

static void k2_rps_player(void)
{
    struct k2_rps_msg m;
    m.player_id = MyTid();
    m.move = m.player_id % 3;
    int verdict = -1;
    Send(k2_rps_referee_tid, (char *)&m, sizeof(m), (char *)&verdict, sizeof(verdict));
    Exit();
}

static void k2_rps_referee(void)
{
    int got = 0;
    int from = 0;
    struct k2_rps_msg m;
    int verdict = 0;
    for (int i = 0; i < 3; i++) {
        Receive(&from, (char *)&m, sizeof(m));
        got++;
        Reply(from, (char *)&verdict, sizeof(verdict));
    }
    k2_rps_replies = got;
    Exit();
}

static int k2_test_rps(void)
{
    k2_rps_replies = 0;
    k2_rps_referee_tid = Create(0, k2_rps_referee);
    Create(0, k2_rps_player);
    Create(0, k2_rps_player);
    Create(0, k2_rps_player);
    for (int i = 0; i < 50; i++) Yield();
    if (k2_rps_replies != 3) {
        uart_printf(CONSOLE, "\033[31mK2: RPS got %d/3 messages\033[37m\r\n",
                    k2_rps_replies);
        return 1;
    }
    return 0;
}

int k2_self_tests(void)
{
    int fails = 0;
    fails += k2_test_send_receive();
    fails += k2_test_nameserver();
    fails += k2_test_rps();
    return fails;
}
