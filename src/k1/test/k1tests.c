#include "k1tests.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "custmath.h"
#include "util.h"

extern int malloctest(void);

static int k1_pass_count = 0;
static int k1_fail_count = 0;

static void k1_pass(const char *name) {
	uart_printf(CONSOLE, "K1_PASS: %s\r\n", name);
	k1_pass_count++;
}

static void k1_fail(const char *name) {
	uart_printf(CONSOLE, "K1_FAIL: %s\r\n", name);
	k1_fail_count++;
}

static void k1_check(int ok, const char *name) {
	if (ok) {
		k1_pass(name);
	} else {
		k1_fail(name);
	}
}

static void test_custstr(void) {
	k1_check(atoi_64("42") == 42, "custstr_atoi_positive");
	k1_check(atoi_64("0") == 0, "custstr_atoi_zero");
	k1_check(strcmp_ret("abc", "abc", 0) != 0, "custstr_strcmp_equal");
	k1_check(strcmp_ret("abc", "abd", 0) == 0, "custstr_strcmp_diff");
	k1_check(is_empty("") != 0, "custstr_is_empty_true");
	k1_check(is_empty("x") == 0, "custstr_is_empty_false");
	k1_check(str_to_hex("0x1A") == 26, "custstr_str_to_hex");

	char buf[] = "a b c";
	char *parts[4];
	int count = parse_char_arr(buf, parts, 4);
	k1_check(count == 3, "custstr_parse_char_arr_count");
	k1_check(strcmp_ret(parts[0], "a", 0) != 0, "custstr_parse_char_arr_first");
	k1_check(strcmp_ret(parts[2], "c", 0) != 0, "custstr_parse_char_arr_last");
}

static void test_custmath(void) {
	k1_check(min(3, 7) == 3, "custmath_min");
	k1_check(max(3, 7) == 7, "custmath_max");
}

static void test_int64voodoo(void) {
	int a = 0;
	int b = 0;
	uint64_t packed = byint_to_int64(0x12345678, (int)0x9ABCDEF0);
	int64_to_byint(packed, &a, &b);
	k1_check((uint32_t)a == 0x12345678u, "int64voodoo_byint_high");
	k1_check((uint32_t)b == 0x9ABCDEF0u, "int64voodoo_byint_low");

	char src[9] = "ABCDEFG";
	char dst[9] = {0};
	uint64_t oc = octochar_to_int64(src);
	int64_to_octochar(oc, dst);
	k1_check(strcmp_ret(dst, "ABCDEFG", 0) != 0, "int64voodoo_octochar_roundtrip");
}

static void init_test_heap(void) {
	uint64_t *block = (uint64_t *)0x1000000;
	block[0] = 1024 * 8;
	block[1] = 0;
}

static void test_malloc(void) {
	init_test_heap();
	k1_check(malloctest() == 0, "malloc_stress");
}

static int child_tid_value;
static int child_parent_tid;
static int child_priority_value;
static int child_ran;

static void k1_child_mytid(void) {
	child_tid_value = MyTid();
	child_parent_tid = MyParentTid();
	child_priority_value = MyPriority();
	child_ran = 1;
	Exit();
}

static void test_create_mytid_parent(void) {
	child_ran = 0;
	int parent = MyTid();
	int tid = Create(5, k1_child_mytid);
	Yield();
	k1_check(tid > 0, "create_returns_tid");
	k1_check(child_ran != 0, "create_child_runs");
	k1_check(child_tid_value == tid, "mytid_matches_create");
	k1_check(child_parent_tid == parent, "myparenttid_matches");
	k1_check(child_priority_value == 5, "mypriority_matches");
}

static int yield_count_a;
static int yield_count_b;

static void k1_yield_task_a(void) {
	yield_count_a++;
	Yield();
	yield_count_a++;
	Exit();
}

static void k1_yield_task_b(void) {
	yield_count_b++;
	Yield();
	yield_count_b++;
	Exit();
}

static void test_yield(void) {
	yield_count_a = 0;
	yield_count_b = 0;
	int tid_b = Create(5, k1_yield_task_b);
	(void)Create(5, k1_yield_task_a);
	(void)tid_b;
	Yield();
	Yield();
	Yield();
	k1_check(yield_count_a == 2, "yield_task_a_completes");
	k1_check(yield_count_b == 2, "yield_task_b_completes");
}

static int priority_trace[2];
static int priority_trace_len;

static void k1_low_priority(void) {
	priority_trace[priority_trace_len++] = MyPriority();
	Exit();
}

static void k1_high_priority(void) {
	priority_trace[priority_trace_len++] = MyPriority();
	Exit();
}

static void test_priority(void) {
	priority_trace_len = 0;
	(void)Create(10, k1_low_priority);
	(void)Create(1, k1_high_priority);
	Yield();
	Yield();
	k1_check(priority_trace_len == 2, "priority_both_ran");
	k1_check(priority_trace[0] == 1, "priority_higher_runs_first");
	k1_check(priority_trace[1] == 10, "priority_lower_runs_second");
}

static int adder_args_sum;

static int64_t k1_adder(
	int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
	int64_t f, int64_t g, int64_t h, int64_t i, int64_t j) {
	return a + b + c + d + e + f + g + h + i + j;
}

static void k1_adder_task(
	int64_t a, int64_t b, int64_t c, int64_t d, int64_t e,
	int64_t f, int64_t g, int64_t h, int64_t i, int64_t j) {
	adder_args_sum = (int)k1_adder(a, b, c, d, e, f, g, h, i, j);
	Exit();
}

static void test_createargs(void) {
	adder_args_sum = 0;
	uint64_t args[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	(void)CreateArgs(5, (void (*)(void))k1_adder_task, 10, args);
	Yield();
	k1_check(adder_args_sum == 55, "createargs_passes_arguments");
}

static int nameserver_tid_value;

static void k1_nameserver_client(void) {
	RegisterAs("k1_test_client");
	nameserver_tid_value = WhoIs("k1_test_client");
	Exit();
}

static void test_nameserver(void) {
	nameserver_tid_value = -1;
	int tid = Create(5, k1_nameserver_client);
	Yield();
	k1_check(tid > 0, "nameserver_client_created");
	k1_check(nameserver_tid_value == tid, "nameserver_whois_self");
}

static void run_k1_tests_core(void) {
	uart_printf(CONSOLE, "=== K1 test suite (QEMU raspi4b) ===\r\n");

	test_custstr();
	test_custmath();
	test_int64voodoo();
	test_malloc();

	uart_printf(CONSOLE, "K1: starting kernel tests\r\n");
	test_create_mytid_parent();
	test_yield();
	test_priority();
	test_createargs();
	test_nameserver();

	uart_printf(CONSOLE, "=== K1 summary: %d passed, %d failed ===\r\n",
		k1_pass_count, k1_fail_count);
	if (k1_fail_count == 0) {
		uart_printf(CONSOLE, "K1_ALL_PASS\r\n");
	} else {
		uart_printf(CONSOLE, "K1_HAS_FAILURES\r\n");
	}
}

void run_k1_tests(void) {
	run_k1_tests_core();
	Exit();
}

void boot_k1_tests(void) {
	run_k1_tests_core();
}
