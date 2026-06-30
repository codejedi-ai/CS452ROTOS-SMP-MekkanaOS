#include "rpi.h"
#include "util.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "display_server.h"
#include "clockserver.h"
#include "syscall.h"
#include "custstr.h"
#include "track_server.h"
#include "nameserver.h"
#include "processes.h"
#include "tc1tests.h"
#include "marklin_worker.h"
#include "k4tests.h"
#include "shell.h"
#include "asciiart.h"

#define SWITCH_COUNT 18
#define CMD_MAX 50

static int wait_for_tid(const char *name)
{
	int tid = WhoIs(name);
	while (tid < 0)
	{
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

static char normalize_track_id(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

static int calculate_digits(int num) __attribute__((unused));
static int calculate_digits(int num)
{
	int count = 0;
	while (num != 0)
	{
		num /= 10;
		++count;
	}
	return count;
}

void print_logo(int unused_console_tid __attribute__((unused)))
{
	display_putc('\n');
	display_putc('\n');
	DisplayPuts(asciiart);
	display_putc('\n');
}

static void run_solenoid_init(void)
{
	(void)Create(2, init_solonoids);
	for (int i = 0; i < 50; i++)
		Yield();
}

static void ensure_train_servers(void)
{
	if (WhoIs("track_server") >= 0)
		return;
	Create(1, marklin_worker_read_notifier);
	Create(1, marklin_worker);
	Create(1, track_server);
	for (int i = 0; i < 32; i++)
		Yield();
}

static void print_help_commands(void)
{
	const char *lines[] = {
		"\nMekkanaOS commands shell\n",
		"  help                        Show this help.\n",
		"  k4whois                     List the registered I/O server task ids.\n",
		"  k4smptest                   Run SMP hub/mailbox regression tests.\n",
		"  k4test                      Run the K4 test suite (background task).\n",
		"  trains                      Enter the train-control subshell.\n",
	};
	for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++)
		display_puts(lines[i]);
}

static void print_help_train(void)
{
	const char *lines[] = {
		"\nMekkanaOS train-control subshell\n",
		"  help                        Show this help.\n",
		"  tr <train> <speed>          Set train speed. train 1-80, speed 0-14.\n",
		"  sw <switch> <C|S>           Set switch direction. switch 1-18, dir C or S.\n",
		"  rv <train> <speed>          Reverse train, resume at <speed>.\n",
		"  tcstd <train> <speed>       Stopping-distance test (calibration).\n",
		"  delaystop <train> <ms>      Stop train after <ms> milliseconds.\n",
		"  interruptstop <train>       Stop train at next sensor hit.\n",
		"  sensordelaystop <train> _ <ms>\n",
		"                              Stop <ms> after the next sensor hit.\n",
		"  ps <start> <end>            Path-switch: line up switches from start to end.\n",
		"  stopa <train> <dest>        Drive train and stop at sensor node <dest>.\n",
		"  quit | q | Q                Return to the commands shell.\n",
	};
	for (size_t i = 0; i < sizeof(lines) / sizeof(lines[0]); i++)
		display_puts(lines[i]);
}

static int dispatch_commands(char *command, char **num, int command_part_count, int train_mode)
{
	// `help` works in both shells and shows the relevant command set.
	if (strcmp_ret(command, "help", 0)) {
		if (train_mode)
			print_help_train();
		else
			print_help_commands();
		return 1;
	}

	if (k4ExecuteCommands(command, num, command_part_count) != -1)
		return 1;

	if (train_mode)
	{
		if (tc1ExecuteCommands(command, num, command_part_count) != 2)
			return 1;
		display_puts("\nERROR: invalid command (type 'help').\n");
		return 0;
	}

	// Top-level commands shell: `trains` switches into the train subshell.
	if (strcmp_ret(command, "trains", 0)) {
		train_shell();
		return 1;
	}

	display_puts("\nERROR: unknown command (type 'help').\n");
	return 0;
}

static void shell_repl(int console_tid, const char *prompt_tag, int train_mode,
		       int poll_sensors)
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

	// Top-level commands shell (train_mode == 0) loops forever — there's
	// nowhere to return to. Train subshell exits via an early-return
	// inside the '\r'/'\n' handler below when `quit`, `q`, or `Q` is
	// typed (checked *before* `command` is reset to "").
	while (1)
	{
		int track_server_tid = WhoIs("track_server");

		while (ConsolePoll(console_tid) < 0)
		{
			if (poll_sensors && track_server_tid > 0)
			{
				int sensor_pushed[10];
				get_sensor_pushed(track_server_tid, sensor_pushed, 10);
				DisplayPrintSensors(sensor_pushed, 10, SENSORCOL, SENSORROW);
			}
			Yield();
		}

		c = (char)ConsoleGetc(console_tid);
		if (c == '\r' || c == '\n')
		{
			char *num[100];
			int command_part_count;
			int valid_command;

			display_putc('\n');
			command_part_count = parse_char_arr(command, num, 100);

			// In train mode, recognize quit aliases *before* dispatch
			// so the user doesn't see a stale "ERROR: invalid command"
			// flash before the subshell returns. strcmp_ret with
			// case_agnostic=1 folds 'q'/'Q' and 'quit'/'Quit'/'QUIT'
			// alike.
			if (train_mode
			    && (strcmp_ret(command, "quit", 1)
			        || strcmp_ret(command, "q", 1))) {
				display_puts("\n");
				return;
			}

			valid_command = dispatch_commands(command, num, command_part_count, train_mode);

			if (valid_command && train_mode)
			{
				for (int i = 0; i < command_part_count; i++)
				{
					display_putc('\n');
				}
			}

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
		}
		else if (c == '\b' || c == 0x7F)
		{
			if (command_index > 0)
			{
				command_index--;
				command[command_index] = '\0';
				display_putc('\b');
				display_putc(' ');
				display_putc('\b');
			}
		}
		else if (command_index < CMD_MAX - 1)
		{
			command[command_index] = c;
			command_index++;
			command[command_index] = '\0';
			display_putc(c);
		}
	}
	DisplayPrintf("\r\n");
}

void train_shell(void)
{
	char track_id = 0;
	int console_tid = wait_for_tid(UART1_CONSOLE_SERVER);
	int track_server_tid;

	(void)wait_for_tid("display_server");

	ensure_train_servers();

	// `normalize_track_id` upper-cases lowercase input, so 'a' / 'b' are
	// accepted as 'A' / 'B' — the prompt advertises that.
	DisplayPrintf("Please enter the track id (A/a or B/b): ");
	while (track_id != 'A' && track_id != 'B')
		track_id = normalize_track_id((char)ConsoleGetc(console_tid));
	DisplayPrintf("\r\n");

	run_solenoid_init();

	track_server_tid = wait_for_tid("track_server");
	init_track(track_server_tid, track_id);

	print_logo(console_tid);
	DisplayPrintf("\033[37m");
	RegisterAs("train_shell");
	DisplayPrintf("Track %c ready. Type quit to return to commands shell.\r\n", track_id);

	shell_repl(console_tid, "TRAIN", 1, 1);
	Deregister();
}

void commands_shell(void)
{
	int console_tid = wait_for_tid(UART1_CONSOLE_SERVER);
	(void)wait_for_tid("display_server");

	display_clear();
	RegisterAs("commands_shell");
	print_logo(console_tid);
	display_putc('\n');
	display_puts("MekkanaOS commands shell. Type trains to start train control.\n\n");

	shell_repl(console_tid, "DARCY", 0, 0);
	Exit();
}
