#include "k2tests.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"

static int k2_pass_count = 0;
static int k2_fail_count = 0;

static void k2_pass(const char *name) {
	uart_printf(CONSOLE, "K2_PASS: %s\r\n", name);
	k2_pass_count++;
}

static void k2_fail(const char *name) {
	uart_printf(CONSOLE, "K2_FAIL: %s\r\n", name);
	k2_fail_count++;
}

static void k2_check(int ok, const char *name) {
	if (ok) {
		k2_pass(name);
	} else {
		k2_fail(name);
	}
}

static void drain_tasks(void) {
	for (int i = 0; i < 32; i++) {
		Yield();
	}
}

/* --- RPS game logic (pure functions) --- */

static void test_rps_logic(void) {
	struct game g;

	g.tid1 = 1;
	g.tid2 = 0;
	g.tid1_move[0] = 0;
	g.tid2_move[0] = 0;
	k2_check(full_game(&g) == 0, "rps_not_full_with_one_player");

	g.tid2 = 3;
	k2_check(full_game(&g) != 0, "rps_full_with_two_players");
	k2_check(full_play(&g) == 0, "rps_not_full_play_empty_moves");

	cust_strcpy(g.tid1_move, 10, "rock", 10);
	cust_strcpy(g.tid2_move, 10, "scissors", 10);
	k2_check(full_play(&g) != 0, "rps_full_play_both_moves");
	k2_check(check_game(&g) == 1, "rps_rock_beats_scissors");

	cust_strcpy(g.tid1_move, 10, "paper", 10);
	cust_strcpy(g.tid2_move, 10, "rock", 10);
	k2_check(check_game(&g) == 1, "rps_paper_beats_rock");

	cust_strcpy(g.tid1_move, 10, "scissors", 10);
	cust_strcpy(g.tid2_move, 10, "paper", 10);
	k2_check(check_game(&g) == 1, "rps_scissors_beats_paper");

	cust_strcpy(g.tid1_move, 10, "rock", 10);
	cust_strcpy(g.tid2_move, 10, "paper", 10);
	k2_check(check_game(&g) == 2, "rps_paper_beats_rock_p2");

	cust_strcpy(g.tid1_move, 10, "rock", 10);
	cust_strcpy(g.tid2_move, 10, "rock", 10);
	k2_check(check_game(&g) == 0, "rps_draw");

	k2_check(ingame(1, &g) != 0, "rps_ingame_tid1");
	k2_check(ingame(99, &g) == 0, "rps_not_ingame_unknown");
}

/* --- Name server --- */

static int k2_ns_self_tid;
static int k2_ns_lookup_tid;
static int k2_ns_overwrite_tid;

static void k2_ns_client_self(void) {
	uart_printf(CONSOLE, "K2_DBG: client start\r\n");
	RegisterAs("k2_ns_self");
	uart_printf(CONSOLE, "K2_DBG: after register\r\n");
	k2_ns_self_tid = WhoIs("k2_ns_self");
	uart_printf(CONSOLE, "K2_DBG: after whois tid=%d\r\n", k2_ns_self_tid);
	Exit();
}

static void k2_ns_client_lookup(void) {
	uart_printf(CONSOLE, "K2_DBG: lookup start\r\n");
	k2_ns_lookup_tid = WhoIs("k2_ns_self");
	uart_printf(CONSOLE, "K2_DBG: lookup got tid=%d\r\n", k2_ns_lookup_tid);
	Exit();
}

static void k2_ns_client_overwrite(void) {
	RegisterAs("k2_ns_slot");
	k2_ns_overwrite_tid = MyTid();
	Exit();
}

static void test_nameserver_register_whois(void) {
	k2_ns_self_tid = -1;
	uart_printf(CONSOLE, "K2_DBG: before create\r\n");
	int tid = Create(4, k2_ns_client_self);
	uart_printf(CONSOLE, "K2_DBG: after create tid=%d\r\n", tid);
	drain_tasks();
	uart_printf(CONSOLE, "K2_DBG: after drain self_tid=%d\r\n", k2_ns_self_tid);
	k2_check(k2_ns_self_tid == tid, "nameserver_whois_self");
}

static void test_nameserver_lookup(void) {
	k2_ns_lookup_tid = -1;
	(void)Create(4, k2_ns_client_self);
	drain_tasks();
	(void)Create(5, k2_ns_client_lookup);
	drain_tasks();
	k2_check(k2_ns_lookup_tid == k2_ns_self_tid, "nameserver_whois_other");
}

static void test_nameserver_duplicate_name(void) {
	int first = Create(4, k2_ns_client_overwrite);
	drain_tasks();
	int second = Create(4, k2_ns_client_overwrite);
	drain_tasks();
	k2_check(WhoIs("k2_ns_slot") == first, "nameserver_duplicate_name_first_wins");
	k2_check(k2_ns_overwrite_tid == second, "nameserver_duplicate_name_both_registered");
}

/* --- Send / Receive / Reply --- */

static char k2_srr_reply[256];
static int k2_srr_send_ret;
static int k2_srr_recv_ret;
static int k2_srr_sender_done;
static int k2_srr_test_msglen;

static void k2_echo_receiver(void) {
	RegisterAs("k2_echo_recv");
	int tid;
	char msg[256];
	char reply[] = "PONG";
	k2_srr_recv_ret = Receive(&tid, msg, 256);
	Reply(tid, reply, 5);
	Exit();
}

static void k2_echo_sender(void) {
	int recv_tid = WhoIs("k2_echo_recv");
	char reply[256] = {0};
	char msg[] = "PING";
	k2_srr_send_ret = Send(recv_tid, msg, 5, reply, 256);
	k2_srr_reply[0] = reply[0];
	k2_srr_reply[1] = reply[1];
	k2_srr_reply[2] = reply[2];
	k2_srr_reply[3] = reply[3];
	k2_srr_reply[4] = 0;
	k2_srr_sender_done = 1;
	Exit();
}

static void test_srr_echo(void) {
	k2_srr_sender_done = 0;
	k2_srr_send_ret = -1;
	k2_srr_recv_ret = -1;
	(void)Create(4, k2_echo_receiver);
	(void)Create(5, k2_echo_sender);
	drain_tasks();
	k2_check(k2_srr_sender_done != 0, "srr_sender_completed");
	k2_check(strcmp_ret(k2_srr_reply, "PONG", 0) != 0, "srr_reply_content");
}

static void k2_recv_first_receiver(void) {
	RegisterAs("k2_rf_recv");
	int tid;
	char msg[64];
	char reply[] = "ACK";
	Receive(&tid, msg, 64);
	Reply(tid, reply, 4);
	Exit();
}

static void k2_recv_first_sender(void) {
	int recv_tid = WhoIs("k2_rf_recv");
	char reply[64] = {0};
	char msg[] = "HELLO";
	k2_srr_send_ret = Send(recv_tid, msg, 6, reply, 64);
	cust_strcpy(k2_srr_reply, 256, reply, 64);
	k2_srr_sender_done = 1;
	Exit();
}

static void test_srr_receive_first(void) {
	k2_srr_sender_done = 0;
	(void)Create(4, k2_recv_first_receiver);
	(void)Create(5, k2_recv_first_sender);
	drain_tasks();
	k2_check(k2_srr_sender_done != 0, "srr_receive_first_completed");
	k2_check(strcmp_ret(k2_srr_reply, "ACK", 0) != 0, "srr_receive_first_reply");
}

static void k2_send_first_sender(void) {
	RegisterAs("k2_sf_send");
	int recv_tid = WhoIs("k2_sf_recv");
	char reply[64] = {0};
	char msg[] = "FIRST";
	k2_srr_send_ret = Send(recv_tid, msg, 6, reply, 64);
	cust_strcpy(k2_srr_reply, 256, reply, 64);
	k2_srr_sender_done = 1;
	Exit();
}

static void k2_send_first_receiver(void) {
	RegisterAs("k2_sf_recv");
	int tid;
	char msg[64];
	char reply[] = "GOTIT";
	Receive(&tid, msg, 64);
	Reply(tid, reply, 6);
	Exit();
}

static void test_srr_send_first(void) {
	k2_srr_sender_done = 0;
	k2_srr_reply[0] = 0;
	(void)Create(4, k2_send_first_sender);
	(void)Create(5, k2_send_first_receiver);
	drain_tasks();
	k2_check(k2_srr_sender_done != 0, "srr_send_first_completed");
	k2_check(strcmp_ret(k2_srr_reply, "GOTIT", 0) != 0, "srr_send_first_reply");
}

static void k2_size_receiver(void) {
	RegisterAs("k2_size_recv");
	int tid;
	char msg[256];
	char reply[] = "OK";
	Receive(&tid, msg, 256);
	Reply(tid, reply, 3);
	Exit();
}

static void k2_size_sender(void) {
	int len = k2_srr_test_msglen;
	int recv_tid = WhoIs("k2_size_recv");
	char msg[256];
	char reply[256] = {0};
	for (int i = 0; i < len - 1; i++) {
		msg[i] = (char)('A' + (i % 26));
	}
	msg[len - 1] = 0;
	k2_srr_send_ret = Send(recv_tid, msg, len, reply, 256);
	k2_srr_reply[0] = reply[0];
	k2_srr_reply[1] = reply[1];
	k2_srr_reply[2] = 0;
	k2_srr_sender_done = 1;
	Exit();
}

static void test_srr_message_size(int msglen, const char *name) {
	k2_srr_test_msglen = msglen;
	k2_srr_sender_done = 0;
	(void)Create(4, k2_size_receiver);
	(void)Create(5, k2_size_sender);
	drain_tasks();
	k2_check(k2_srr_sender_done != 0, name);
	k2_check(strcmp_ret(k2_srr_reply, "OK", 0) != 0, name);
}

static void test_srr_message_sizes(void) {
	test_srr_message_size(4, "srr_msglen_4");
	test_srr_message_size(64, "srr_msglen_64");
	test_srr_message_size(256, "srr_msglen_256");
}

/* --- RPS integration --- */

static char k2_rps_p1_result;
static char k2_rps_p2_result;
static char k2_rps_err_result;

static void k2_rps_player_rock(void) {
	signup();
	k2_rps_p1_result = play("rock");
	quit();
	Exit();
}

static void k2_rps_player_scissors(void) {
	signup();
	k2_rps_p2_result = play("scissors");
	quit();
	Exit();
}

static void k2_rps_play_without_signup(void) {
	k2_rps_err_result = play("rock");
	Exit();
}

static void test_rps_integration(void) {
	k2_rps_p1_result = 0;
	k2_rps_p2_result = 0;
	(void)Create(2, gameserver);
	drain_tasks();
	k2_check(WhoIs("gameserver") != NUMPROCS, "rps_gameserver_registered");

	(void)Create(4, k2_rps_player_rock);
	(void)Create(4, k2_rps_player_scissors);
	drain_tasks();

	k2_check(k2_rps_p1_result == 'W', "rps_integration_p1_wins");
	k2_check(k2_rps_p2_result == 'L', "rps_integration_p2_loses");

	k2_rps_err_result = 0;
	(void)Create(4, k2_rps_play_without_signup);
	drain_tasks();
	k2_check(k2_rps_err_result == 'E', "rps_play_without_signup_errors");

	char shutdown_ret = RPCShutdown();
	drain_tasks();
	k2_check(shutdown_ret == '+', "rps_shutdown_ack");
}

static void run_k2_tests_core(void) {
	uart_printf(CONSOLE, "\033[2J\033[H");
	uart_printf(CONSOLE, "=== K2 test suite ===\r\n");

	test_rps_logic();
	test_nameserver_register_whois();
	test_nameserver_lookup();
	test_nameserver_duplicate_name();
	test_srr_echo();
	test_srr_receive_first();
	test_srr_send_first();
	test_srr_message_sizes();
	test_rps_integration();

	uart_printf(CONSOLE, "=== K2 summary: %d passed, %d failed ===\r\n",
		k2_pass_count, k2_fail_count);
	if (k2_fail_count == 0) {
		uart_printf(CONSOLE, "K2_ALL_PASS\r\n");
	} else {
		uart_printf(CONSOLE, "K2_HAS_FAILURES\r\n");
	}
}

void run_k2_tests(void) {
	run_k2_tests_core();
	Exit();
}

void boot_k2_tests(void) {
	run_k2_tests_core();
}

int k2_self_tests(void)
{
	run_k2_tests_core();
	return k2_fail_count;
}
