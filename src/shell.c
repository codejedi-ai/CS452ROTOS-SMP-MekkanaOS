#include "rpi.h"
#include "util.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "display_server.h"
#include "clockserver.h"
#include "syscall.h"
#include "custstr.h"
#include "nameserver.h"
#include "processes.h"
#include "k4tests.h"
#include "k4_smp_tests.h"
#include "rotos_link.h"
#include "shell.h"
#include "asciiart.h"
#include "config.h"

#define CMD_MAX 50

static int wait_for_tid(const char *name)
{
	int tid = WhoIs(name);
	while (tid < 0) {
		Yield();
		tid = WhoIs(name);
	}
	return tid;
}

static void display_putc(char ch)
{
	char buf[2] = {ch, '\0'};
	DisplayPuts(buf);
}

static void display_puts(const char *str)
{
	DisplayPuts(str);
}

static void display_clear(void)
{
	DisplayPuts("\033[2J\033[H");
}

void print_logo(int unused_console_tid __attribute__((unused)))
{
	display_putc('\n');
	display_putc('\n');
	DisplayPuts(asciiart);
	display_putc('\n');
}

static void print_help_commands(void)
{
	const char *lines[] = {
		"\n" ROTOS_OS_NAME " network shell\n",
		"  help                        Show this help.\n",
		"  k4whois                     List registered server task ids.\n",
		"  k4test                      Run K4 test suite (background).\n",
		"  k4smptest                   Run SMP hub/mailbox tests (background).\n",
		"  link <cmd...>               Send a line on the ROTS network link.\n",
		"  linktest                    PING the network hub.\n",
	};
	for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++)
		display_puts(lines[i]);
}

static void cmd_link(const char *args)
{
	char reply[ROTOS_LINK_REPLY_MAX];
	LinkSend(args, reply, (int)sizeof(reply));
	display_puts(reply);
	display_putc('\n');
}

static void cmd_linktest(void)
{
	char reply[ROTOS_LINK_REPLY_MAX];
	LinkSend("PING", reply, (int)sizeof(reply));
	display_puts("linktest PING -> ");
	display_puts(reply);
	display_putc('\n');
}

static int dispatch_commands(char *command, char **num, int command_part_count)
{
	if (strcmp_ret(command, "help", 0)) {
		print_help_commands();
		return 1;
	}
	if (strcmp_ret(command, "linktest", 0)) {
		cmd_linktest();
		return 1;
	}
	if (strcmp_ret(command, "link", 0)) {
		const char *args = "";
		if (command_part_count > 1)
			args = num[1];
		cmd_link(args);
		return 1;
	}
	if (strcmp_ret(command, "k4smptest", 0)) {
		(void)Create(2, k4_smp_tests_task);
		display_puts("SMP tests started\n");
		return 1;
	}
	if (k4ExecuteCommands(command, num, command_part_count) != -1)
		return 1;
	display_puts("\nERROR: unknown command (type 'help').\n");
	return 0;
}

static void shell_repl(int console_tid, const char *prompt_tag)
{
	unsigned int counter = 1;
	char command[CMD_MAX];
	int command_index = 0;
	char c;

	command[0] = '\0';

	char prompt[40];
	int plen = 0;
	for (int i = 0; prompt_tag[i] && plen < 30; i++)
		prompt[plen++] = prompt_tag[i];
	prompt[plen++] = '[';
	prompt[plen++] = '0' + (counter - 1) % 10;
	prompt[plen++] = ']';
	prompt[plen++] = '>';
	prompt[plen++] = ' ';
	prompt[plen] = '\0';

	display_puts(prompt);

	while (1) {
		while (ConsolePoll(console_tid) < 0)
			Yield();

		c = (char)ConsoleGetc(console_tid);
		if (c == '\r' || c == '\n') {
			char *num[100];
			int command_part_count;

			display_putc('\n');
			command_part_count = parse_char_arr(command, num, 100);
			(void)dispatch_commands(command, num, command_part_count);

			command_index = 0;
			command[0] = '\0';
			counter++;
			Yield();
			plen = 0;
			for (int i = 0; prompt_tag[i] && plen < 30; i++)
				prompt[plen++] = prompt_tag[i];
			prompt[plen++] = '[';
			prompt[plen++] = '0' + (counter - 1) % 10;
			prompt[plen++] = ']';
			prompt[plen++] = '>';
			prompt[plen++] = ' ';
			prompt[plen] = '\0';
			display_puts(prompt);
			Yield();
		} else if (c == '\b' || c == 0x7F) {
			if (command_index > 0) {
				command_index--;
				command[command_index] = '\0';
				display_putc('\b');
				display_putc(' ');
				display_putc('\b');
			}
		} else if (command_index < CMD_MAX - 1) {
			command[command_index] = c;
			command_index++;
			command[command_index] = '\0';
			display_putc(c);
		}
	}
}

void commands_shell(void)
{
	int console_tid = wait_for_tid(UART1_CONSOLE_SERVER);
	(void)wait_for_tid("display_server");
	(void)wait_for_tid(ROTOS_LINK_SERVER_NAME);

	display_clear();
	RegisterAs("commands_shell");
	print_logo(console_tid);
	display_putc('\n');
	display_puts(ROTOS_OS_NAME " network shell. Type linktest or link <cmd>.\n\n");

	shell_repl(console_tid, "NET");
	Exit();
}
