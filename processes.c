#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"

#include "custstr.h"
#include "nameserver.h"
#include "gameserver.h"
#include "clockserver.h"
#include "sensorserver.h"
#include "k2tests.h"
#include "systimer.h"
#include "k2rps.h"

#include "k3tests.h"
#include "k4tests.h"
#include "asm.h"
#include "ioserver.h"
#include "traincont.h"

#define DISPLAY 1
/*
These are the most essential terminal control sequences that you will need for your train program.

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
void print_error(char *error){
    // print in red
    uart_printf(CONSOLE, "\033[31m");
    uart_printf(CONSOLE, "%s\r\n", error);
    // print in white
    uart_printf(CONSOLE, "\033[37m");
}
void idle(){
	while(1){
		// uart_printf(CONSOLE, "idle: WFI <Print time here>\r\n");
		uart_printf(CONSOLE, "idle: time = %u\r\n", get_timerLO());
		asm("WFI");
		//Yield();
		uint32_t runtime = GetRuntime();
		uint32_t kernelrt = GetKernelRuntime();
		//uart_printf(CONSOLE, "idle: runprecentage = %u \% \r\n", (100 * runtime) / kernelrt);
	}
	Exit();
}
void main(){
	// print in white font
	uart_printf(CONSOLE, "\033[37m");
	// register the k2
	RegisterAs("main");
	char *logo = "            ___     ___     ___     ___   __   __   ___     ___   \r\n    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n";
  	uart_printf(CONSOLE, "%s\r\n", logo);
  	uart_printf(CONSOLE, "Modified main to busywait \r\nHello World I am d273liu\r\n");
	unsigned int counter=1;
	char command[50];
	int command_length = 0;
	command[0] = '\0';
	uart_printf(CONSOLE, "\r\nDARCY[%u]> ", counter++);
	char c = ' ';
	while (!(strcmp_ret(command, "quit"))) {
		while (!uart_getc_queue(CONSOLE)) {
			Yield();
		}
		c = uart_getc_modified(CONSOLE);
		if (c == '\r') {
			uart_printf(CONSOLE, "\r\n");
			// K2 commands
			// the parse char array changes the command 
      		
			char *num[100]; // array to store the numbers
			// int parse_char_arr(char *arr, char **num, int num_size)
			int command_part_count = parse_char_arr(command, num, 100);
			uart_printf(CONSOLE, "command = %s\r\n", command);
			for (int i = 0; i < command_part_count; i++){
				uart_printf(CONSOLE, "num[%d] = %s\r\n", i, num[i]);
			}
			if (k3ExecuteCommands(command, num, command_part_count) != -1);
			else {
				uart_printf(CONSOLE, "ERROR: command is not valid command_part_count = %d\r\n", command_part_count);
			}
			// K3 commands
			// The operating system is doomed to go to sleep or die after running the command

			command_length = 0;
			command[0] = '\0';
			Yield();
			uart_printf(CONSOLE, "\r\nDARCY[%u]> ", counter++);
			Yield();
		}else if (c == '\b'){
			if (command_length > 0){
				command_length--;
				command[command_length] = '\0';
				uart_printf(CONSOLE, "\b \b");
			}
		}else {
			command[command_length] = c;
			command_length++;
			command[command_length] = '\0';
			uart_putc(CONSOLE, c);
		}
	}
	uart_printf(CONSOLE, "\r\n");
	
	// print white font
	uart_printf(CONSOLE, "\033[37m");
	Exit();
}

void busyloop(){
	while(1) uart_printf(CONSOLE, "busyloop\r\n");
	uart_printf(CONSOLE, "busyloopexit\r\n");
	Exit();
}
#define UARTINTER 153
// new paradymn, run tests for each k# assignment (other than 3) before running the shell
void FirstUserTask() // First task as dictated in the reqs
{	// need to set the timer interrupt
	RegisterAs("FirstUserTask");
	int tid;
	uart_printf(CONSOLE, "clock_notifier: tid = %d\r\n", tid);
	uint32_t clock_server_tid = WhoIs("clock_server");
	uart_printf(CONSOLE, "clock_server: clock_server_tid = %d\r\n", clock_server_tid);
	// tid = Create(1, k2FirstUserTask);
	int idle_tid = Create(-1, idle);
    //uart_printf(CONSOLE, "\033[34midle: tid = %d\r\n", idle_tid);
	//tid = Create(-3, k3FirstUserTask);
	// Delay(clock_server_tid, 300);
	// Commented out the old code
	// uart_printf(CONSOLE, "read_s88_1 FIRST TASK INIT\r\n");
	// run the read_s88_1 test, the result of the test should have the marklin read the first s88 sensor
	// tid = Create(1, read_s88_test_many);
	// uart_printf(CONSOLE, "read_s88_test_many: tid = %d\r\n", tid);
	//tid = Create(7, k3FirstUserTask);
	//uart_printf(CONSOLE, "k3_clock_proc: tid = %d\r\n", tid);
	//execute_train_command(0, 54);
	//execute_train_command(0, 54);
	//execute_reverse_command(10, 54);
	//execute_train_command(0, 54);
	//uart_printf(CONSOLE, "DELAY: %d\r\n", Delay(clock_server_tid, 1000)); // this is the value I am curiose of
	//execute_train_command(15, 54);
	//uart_printf(CONSOLE, "DELAY: %d\r\n", Delay(clock_server_tid, 1000));
	//execute_train_command(10, 54);
	//uart_printf(CONSOLE, "DELAY: %d\r\n", Delay(clock_server_tid, 1000));
	//execute_train_command(0, 54);
	//uart_printf(CONSOLE, "main: tid = %d\r\n", tid);
	// print in green process finnished
	// uart_printf(CONSOLE, "\033[32m");
	// uart_printf(CONSOLE, "FirstUserTask: FIRST TASK FINISHED\r\n");
	// print in white
	// uart_printf(CONSOLE, "\033[37m");
	// tid = Create(-2, sensor_server_notifier);
	int RXIC_server = WhoIs("io_RXIC_MARKLIN_server");
	int TXIC_server = WhoIs("io_TXIC_MARKLIN_server");
	uart_printf(CONSOLE, "RXIC_server: tid = %d\r\n", RXIC_server);
	uart_printf(CONSOLE, "TXIC_server: tid = %d\r\n", TXIC_server);
	Putc(TXIC_server, MARKLIN, 193 );
	uart_printf(CONSOLE, "Byte 1: %x\r\n", 	Getc(RXIC_server, MARKLIN));
	uart_printf(CONSOLE, "Byte 2: %x\r\n", Getc(RXIC_server, MARKLIN));
	uart_printf(CONSOLE, "sensor_server_monitor: tid = %d\r\n", tid);
	Exit();
}