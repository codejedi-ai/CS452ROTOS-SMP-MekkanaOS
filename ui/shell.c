#include "../rpi.h"
#include "../util.h"
#include "../ioserver.h"
#include "../clockserver.h"
#include "../syscall.h"
#include "../custstr.h"
#include "../tc1/track_server.h"
#include "shell.h"

static const size_t COMMANDMAX_LEN = 64;
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
void print_time_to_display(){
  int tid;
  int msg = 0;
  // Receive(&tid, &msg, sizeof(msg));
  // Reply(tid, &msg, 0);
  while (1)
  {
    int clock_server_tid = WhoIs("clock_server");
    Delay(clock_server_tid, 1);
    int cur_time = Time(clock_server_tid);
    // print the time on the top left corner
    uart_printf(CONSOLE,"\033[%u;%uH", 1, TICKSCOL);
    uart_printf(CONSOLE, "\033[K");
    uart_printf(CONSOLE, "\033[?25l");
    uart_printf(CONSOLE, "Ticks: %d", cur_time);
    // Hide the cursor
    
    // clear row "\033[K"
    
    // print the time in ticks
    
    // show cursor
    uart_printf(CONSOLE, "\033[?25h");
  }
}
/*
The print server would be incharge of printing the recentlly triggered sensors and the activated switches
it would also print the train states
*/
void print_server(){


}

void print_table_headers(){
  /*
  
  | Previous  Sensor | current sensor | Distance traveled | Time Elapsed (Ticks) |
| --- | --- | --- | --- |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
*/


  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW - 2,TABLECOL);
  uart_printf(CONSOLE,"| Previous  Sensor | current sensor | Distance traveled | Time Elapsed (Ticks) |\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW - 1 ,TABLECOL);
  uart_printf(CONSOLE,"| --- | --- | --- | --- |\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW,1);
  /*
  #define SENSORROW 50
#define SENSORCOL 0
  */
  uart_printf(CONSOLE,"\033[%u;%uH",SENSORROW - 2,SENSORCOL);
  uart_printf(CONSOLE,"Sensor Table\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",SENSORROW - 1,SENSORCOL);
  uart_printf(CONSOLE,"Recent Triggers\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW - 3,1);
  for (int i = 0; i < 200; i++){
    uart_putc(CONSOLE, '-');
  }
}
void print_logo(uint32_t r, uint32_t c){
  // move cursor to r1,c1
  uart_printf(CONSOLE,"\033[%u;%uH", r, c);
  char *logo = "\r\n            ___     ___     ___     ___   __   __   ___     ___   \r\n    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n";
  uart_printf(CONSOLE, "%s\r\n", logo);
  uart_printf(CONSOLE, "Fixed the loop? \r\n");
  print_table_headers();
}
#include <stdio.h>

int calculate_digits(int num) {
    int count = 0;
    while(num != 0) {
        num /= 10;
        ++count;
    }
    return count;
}
void test_new_server_functions(){
  /*
  void get_sensor_pushed(int track_server_tid, int sensor_pushed[], int train_max);
  void get_switch_states(int track_server_tid, char sw_states[], int switch_count);
  void init_track(int track_server_tid, char track_id);
  void init_train(int track_server_tid, char train_no, char s88_id, char sensor_id);
  void deregister_train(int track_server_tid, char train_no);
  uint64_t awit_sensor(int track_server_tid);
  */
  Exit();
}

// PRINT FUNCTIONS BEGIN
void print_sw_states(char *sw_states, uint32_t r, uint32_t c, uint8_t sw_ind){
  uart_printf(CONSOLE,"\033[%u;%uH",r,c);
  uart_puts(CONSOLE, "SW");
  uint8_t middle_Sw[] ={0x99,0x9a,0x9b,0x9c};
  for (uint32_t i = 0; i < 4; i ++){
    if (sw_ind == middle_Sw[i]){
      uart_printf(CONSOLE,"\033[%u;%uH",r + i + SWITCH_COUNT + 1, c);
      uart_printf(CONSOLE,"T%x: ",  middle_Sw[i]);
      uart_putc(CONSOLE, sw_states[middle_Sw[i]]);
      uart_puts(CONSOLE, "\r\n");
      return; 
    } 
  }
  uart_printf(CONSOLE,"\033[%u;%uH",r + sw_ind, c);
  uart_printf(CONSOLE,"T%u: ", sw_ind);
  uart_putc(CONSOLE, sw_states[sw_ind]);
  uart_puts(CONSOLE, "\r\n");
}
void new_table_row(char prev_changed_s88, char prev_changed_sensor,
                  char s88_id, char sensor_id, 
                  int dist, int time_diff, int *offset){
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW + *offset,TABLECOL);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, prev_changed_s88 + 'A');
  uart_printf(CONSOLE, "%d", prev_changed_sensor);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, s88_id + 'A');
  uart_printf(CONSOLE, "%d", sensor_id);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", dist);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", time_diff);
  uart_putc(CONSOLE, '|');
  uart_puts(CONSOLE, "\r\n");
  *offset += 1;
}




void command_shell(){
  
  Delay(WhoIs("clock_server"), 100);
  print_logo(SHELLROW - 2, SHELLCOL);
	// print in white font
	uart_printf(CONSOLE, "\033[37m");
	// register the k2
	RegisterAs("command_shell");
	unsigned int counter=1;
	char command[50];
	int command_length = 0;
	command[0] = '\0';
  // set cursor at SHELLROW and SHELLCOL
  
  uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW, LOGO_WIDTH + SHELLCOL);
	uart_printf(CONSOLE, "DARCY[%u]> ", counter++);
	char c = ' ';
  // int time_display_tid = Create(MyPriority(), print_time_to_display);
  // get who is track_server
  int track_server_tid = WhoIs("track_server");
  int msg;
  Send(track_server_tid, &msg, 0, &msg, 0);
	while (!(strcmp_ret(command, "quit"))) {
    int track_server_tid = WhoIs("track_server");
		while (!uart_getc_queue(CONSOLE)) {
      int sensor_pushed[10];
      get_sensor_pushed(track_server_tid, sensor_pushed, 10);
      print_sensors(sensor_pushed, 10, SENSORCOL, SENSORROW, CONSOLE);
      // print_switch(switch_no, (char)switch_state, CONSOLE);
		}
		c = uart_getc_modified(CONSOLE);
		if (c == '\r') {
			uart_printf(CONSOLE, "\r\n");
			// K2 commands
			// the parse char array changes the command 
      		
			char *num[100]; // array to store the numbers
			// int parse_char_arr(char *arr, char **num, int num_size)
			int command_part_count = parse_char_arr(command, num, 100);
      uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW, LOGO_WIDTH + SHELLCOL);
      int valid_command = 0;
			if (tc1ExecuteCommands(command, num, command_part_count) != 2){
        uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW + 1, LOGO_WIDTH +  SHELLCOL);
        uart_printf(CONSOLE, "\033[K");
        valid_command = 1;
      }
			else {
        uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW + 1, LOGO_WIDTH + SHELLCOL);
        uart_printf(CONSOLE, "ERROR: command is not valid command_part_count = %d\r\n", command_part_count);
			}
      for (int i = 0; valid_command && i < command_part_count; i++){
        uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW + i + 1, LOGO_WIDTH + SHELLCOL);
        uart_printf(CONSOLE, "\033[K");
        uart_printf(CONSOLE, "num[%d] = %s\r\n", i, num[i]);
      }
      
			// tc1(command);
			// K3 commands
			// The operating system is doomed to go to sleep or die after running the command
      uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW,  LOGO_WIDTH + SHELLCOL);
      // slear row
      
			command_length = 0;
			command[0] = '\0';
			Yield();
      char* darcy = "DARCY[%u]> ";
      uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW, LOGO_WIDTH + SHELLCOL);
      uart_printf(CONSOLE, "\033[K");
			uart_printf(CONSOLE, "DARCY[%u]> ", counter++);
			Yield();
		}else if (c == '\b'){
			if (command_length > 0){
				command_length--;
				command[command_length] = '\0';
        uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW, LOGO_WIDTH + NAMEOFFSET + calculate_digits(counter) + SHELLCOL + command_length);
				uart_printf(CONSOLE, "\b \b");
			}
		}else {
			command[command_length] = c;
      uart_printf(CONSOLE, "\033[%d;%dH", SHELLROW, LOGO_WIDTH + NAMEOFFSET + calculate_digits(counter) + SHELLCOL + command_length - 1);
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
