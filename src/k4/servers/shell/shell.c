#include "rpi.h"
#include "util.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "clockserver.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstring.h"
#include "track_server.h"
#include "tc1tests.h"
#include "shell.h"

static int wait_for_tid(const char *name)
{
	int tid = WhoIs(name);
	while (tid < 0) { Yield(); tid = WhoIs(name); }
	return tid;
}

#define UNINT_MAX 0xffffffff
#define OVERFLOW_MINUTES = (UNINT_MAX / 1e6) / 60;
#define OVERFLOW_SECONDS = UNINT_MAX / 1e6;
#define OVERFLOW_TENTH_OF_SECOND = UNINT_MAX / 1e5;
#define TOP_ROW 4
#define LEFT_COL 1
#define WINDOW_HEIGHT 39
#define WINDOW_WIDTH 90
#define COMMAND_ROW 41
#define SW_ROW 1
#define MARKLIN_ROW 1
#define SENSORS_ROW 1
#define ACTIVATED_SWITCHES_ROW 9
#define SECOND_COL 16
#define THIRD_COL 48
#define FOURTH 1
#define POLL_TIME 150000
#define SENSOR_LIST_MAXLEN 100
#define QUEUE_MAX_LEN 200
#define SWITCH_COUNT 18
#define ERROR_ROW COMMAND_ROW + 1
#define QUEUE_MAX_ROW COMMAND_ROW + 2
#define SENSOR_QUERRY COMMAND_ROW + 3
// 240 bytes per second
#define S88_NOS 5
// Serial line 1 on the RPi hat is used for the console
/*

Code	Effect
"\033[2J"	Clear the screen.
"\033[H"	Move the cursor to the upper-left corner of the screen.
"\033[r;cH"	Move the cursor to row r, column c. Note that both the rows and columns are indexed starting at 1.
"\033[?25l"	Hide the cursor.
"\033[K"	Delete everything from the cursor to the end of the line.
These control sequences can help make your program's display more lively.

Code	Effect
"\033[0m"	Reset special formatting (such as colour).
"\033[30m"	Black text.
"\033[31m"	Red text.
"\033[32m"	Green text.
"\033[33m"	Yellow text.
"\033[34m"	Blue text.
"\033[35m"	Magenta text.
"\033[36m"	Cyan text.
"\033[37m"	White text.


*/
void print_logo(uint32_t r, uint32_t c)
{
  uart_printf(CONSOLE, "\033[%u;%uH", r, c);
  uart_printf(CONSOLE,
    "            ___     ___     ___     ___   __   __   ___     ___   \r\n"
    "    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n"
    "   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n"
    "  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n"
    " {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n"
    "./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n");
}

/* Count decimal digits in num. Used to advance the cursor past
   "DARCY[%u]> " when rewriting positionally over the prompt. */
int calculate_digits(int num)
{
  int count = 0;
  if (num == 0) return 1;
  while (num != 0) { num /= 10; ++count; }
  return count;
}

/* Refresh the "Ticks: N" status read in the top-right corner. Cheap
   single-shot call; safe to invoke from the prompt loop -- no Delay(),
   nothing blocking past the Time syscall + four printfs. */
void print_time_to_display(void)
{
  int clock_server_tid = WhoIs("clock_server");
  if (clock_server_tid <= 0) return;
  int cur_time = Time(clock_server_tid);
  uart_printf(CONSOLE, "\033[?25l");                       /* hide cursor */
  uart_printf(CONSOLE, "\033[%u;%uH\033[K", 1, TICKSCOL);  /* park + clear */
  uart_printf(CONSOLE, "Ticks: %d", cur_time);
  uart_printf(CONSOLE, "\033[?25h");                       /* show cursor */
}

/* Render all switch positions in the status panel. sw_states must be the
   144-byte array returned by get_switches(): index = switch id, value = 'S'
   or 'C'. Uses print_switch() (util.c) for the per-switch row/col math. */
void print_sw_states(char *sw_states)
{
  uint8_t middle_sw[] = {0x99, 0x9a, 0x9b, 0x9c};
  for (uint32_t i = 0; i < 4; i++)
    print_switch(middle_sw[i], (char)sw_states[middle_sw[i]], CONSOLE);
  for (uint32_t i = 0; i < 18; i++)
    print_switch(i, (char)sw_states[i], CONSOLE);
}

static void print_help(void)
{
  uart_printf(CONSOLE,
    "\r\nAvailable commands:\r\n"
    "  tr <train> <speed>            Set train speed (train 1-80, speed 0-14)\r\n"
    "  sw <switch> <S|C>             Throw switch (1-18; S=straight, C=curved)\r\n"
    "  rv <train> <speed>            Reverse train and resume at <speed>\r\n"
    "  delaystop <train> <ticks>     Stop train after <ticks> tick delay\r\n"
    "  interruptstop <train>         Stop train on its next sensor trigger\r\n"
    "  sensordelaystop <t> <_> <d>   Stop train after sensor + <d> tick delay\r\n"
    "  ps <fromNode> <toNode>        Switch path between two track nodes\r\n"
    "  stopa <train> <dest>          Stop train at destination sensor node\r\n"
    "  on <s88 A-E> <s 1-16> <cmd>   Run <cmd> when sensor fires (e.g. on A 5 tr 24 8)\r\n"
    "  help                          Show this menu\r\n"
    "\r\n");
}

void command_shell()
{
  // print in white font
  uart_printf(CONSOLE, "\033[37m");
  RegisterAs("command_shell");
  int console_tid = wait_for_tid(UART1_CONSOLE_SERVER);

  /* Banner positioned at top-left so it anchors regardless of prior output
     (idle task, server boot messages) that scrolled in before we ran. */
  print_logo(1, 1);
  uart_printf(CONSOLE, "Welcome to the test shell. Type 'help' for commands.\r\n");

  /* Track-id selection (carried over from the old command_shell). The
     track_server holds the node graph and needs to know which layout to
     load before path/stop-at/sensor commands have anything to look up. */
  uart_printf(CONSOLE, "\r\nSelect track (A or B): ");
  char track_id = ' ';
  while (track_id != 'A' && track_id != 'B')
  {
    char in = (char)ConsoleGetc(console_tid);
    if (in == 'a') in = 'A';
    else if (in == 'b') in = 'B';
    if (in == 'A' || in == 'B')
    {
      track_id = in;
      uart_putc(CONSOLE, track_id);
    }
  }
  uart_printf(CONSOLE, "\r\n");
  int track_srv_tid = WhoIs("track_server");
  if (track_srv_tid > 0) init_track(track_srv_tid, track_id);

  /* One-time status panel after track init: ticks, "Track: X", switch states.
     The tick line is refreshed each prompt cycle below; the switch panel sits
     under the logo (rows 4-13 per print_switch's hard-coded math). */
  uart_printf(CONSOLE, "\033[8;1HTrack: %c", track_id);
  print_time_to_display();
  {
    /* Initial render uses 'S' for every switch. Track server only learns
       actual states once an `sw N S|C` command has been issued; before then
       its view is zeros, which would print as nulls. We keep the optimistic
       default and let subsequent `sw` commands repaint individual cells. */
    char sw_states[256];
    for (int i = 0; i < 256; i++) sw_states[i] = 'S';
    print_sw_states(sw_states);
  }

  unsigned int counter = 1;
  char command[50];
  int command_length = 0;
  command[0] = '\0';
  /* Anchor prompt at SHELLROW so the input stays put even when above-the-line
     output (logo + status) keeps growing across reboots / resets. */
  uart_printf(CONSOLE, "\033[%d;%dH\033[K", SHELLROW, SHELLCOL);
  uart_printf(CONSOLE, "DARCY[%u]> ", counter++);
  char c = ' ';
  while (1)
  {
    c = (char)ConsoleGetc(console_tid);
    /* Real terminals (QEMU stdio, screen, picocom) send CR; piped stdin
       sends LF. Accept either as the line terminator. */
    if (c == '\r' || c == '\n')
    {
      uart_printf(CONSOLE, "\r\n");

      char *num[100];
      int command_part_count = parse_char_arr(command, num, 100);

      if (strcmp_ret(command, "help")) {
        print_help();
      } else {
        uart_printf(CONSOLE, "command = %s\r\n", command);
        for (int i = 0; i < command_part_count; i++)
        {
          uart_printf(CONSOLE, "num[%d] = %s\r\n", i, num[i]);
        }
        if (tc1ExecuteCommands(command, num, command_part_count) == 2)
        {
          uart_printf(CONSOLE, "ERROR: unknown command '%s'. Type 'help' for the command list.\r\n", command);
        }
      }
      // K3 commands
      // The operating system is doomed to go to sleep or die after running the command

      command_length = 0;
      command[0] = '\0';
      Yield();
      /* Refresh tick clock + reanchor prompt at SHELLROW each cycle so
         command output above doesn't push the prompt off-screen. */
      print_time_to_display();
      uart_printf(CONSOLE, "\033[%d;%dH\033[K", SHELLROW, SHELLCOL);
      uart_printf(CONSOLE, "DARCY[%u]> ", counter++);
      Yield();
    }
    else if (c == '\b' || c == 0x7f)
    {
      /* Terminals send DEL (0x7f) for the Backspace key; classic \b (0x08)
         comes from Ctrl-H. Treat both as "erase last char": pop from the
         in-memory command buffer and erase the glyph on screen via `\b \b`. */
      if (command_length > 0)
      {
        command_length--;
        command[command_length] = '\0';
        uart_printf(CONSOLE, "\b \b");
      }
    }
    else if (command_length < (int)sizeof(command) - 1)
    {
      command[command_length] = c;
      command_length++;
      command[command_length] = '\0';
      uart_putc(CONSOLE, c);
    }
  }
  /* unreachable -- while(1) never exits; the shell runs for the life of the OS */
}
