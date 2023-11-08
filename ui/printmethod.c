#include "../rpi.h"
#include "../util.h"
#include "../ioserver.h"
#include "../clockserver.h"
#include "../custstr.h"


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

char sw_states[255];
uint8_t trains_speed[81]; // this is the speed of the train
uint32_t sol_on_time= 0;
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
  /*
uint32_t io_TXIC_MARKLIN_server_pid;
uint32_t io_RXIC_MARKLIN_server_pid;
uint32_t io_CTS_MARKLIN_server_pid;
void init_ioserver(){
    io_TXIC_MARKLIN_server_pid = WhoIs("io_TXIC_MARKLIN_server");
    io_RXIC_MARKLIN_server_pid = WhoIs("io_RXIC_MARKLIN_server");
    io_CTS_MARKLIN_server_pid = WhoIs("io_CTS_MARKLIN_server");
}
// make functions to get the marklin IO PIDs
uint32_t MARKLIN_PUT_SERVER(){
    return io_TXIC_MARKLIN_server_pid;
}
uint32_t MARKLIN_GET_SERVER(){
    return io_RXIC_MARKLIN_server_pid;
}
uint32_t get_Marklin_CTS_pid(){
    return io_CTS_MARKLIN_server_pid;
}

*/

/*
sockets binary number
1 = 128
2 = 64
3 = 32
4 = 16
5 = 8
6 = 4
7 = 2
8 = 1
*/
// return a value from 1 to 16 depending which sensor got triggered

// the size of the ret is s88_on

/*
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id){  // Binary: 00000001)
      command_wrapper(speed, id);
      trains_speed[id] = speed;
}
void execute_reverse_command(unsigned char speed, unsigned char id){  // Binary: 00000001)
      command_wrapper(0, id);
      command_wrapper(15, id);
      command_wrapper(speed, id);
}

// define a function that takes a char array as a parameter
//void tc1(char *arr) {

  // execute here
  if (num[0][0] == 't' && num[0][1] == 'r'){
    // set train speed
    // get train number
    char *train_id_ptr = num[1];
    uint64_t train_id = atoi_64(train_id_ptr);

    char *train_command_ptr = num[2];
    uint64_t train_speed = atoi_64(train_command_ptr);
    // check is the train number valid
    if (train_id > 80 || train_id < 1){
      print_error("ERROR: INVALID TRAIN NUMBER");
      return 1;
    }
    execute_train_command(train_speed, train_id);
  }else if (num[0][0] == 'r' && num[0][1] == 'v'){
    // reverse the train
    // This command is just a prototype. It gives the user the function of reversing it at a different speed
    // get train number
    char *train_command_ptr = num[1];
    uint64_t train_id = atoi_64(train_command_ptr);
    // check is the train number valid
    if (train_id > 80 || train_id < 1){
      print_error("ERROR: INVALID TRAIN NUMBER");
      return 1;
    }
    execute_reverse_command(train_id);
  }else if (num[0][0] == 's' && num[0][1] == 'w'){
    // switch solonoids
    char *switch_number = num[1];
    if(switch_number[0] == '0' && switch_number[1] == 'x'){
      // check is it a valid solonoid id, the valid ones are only 0x99, 0x9a, 0x9b, 0x9c

      // this is a hex number
      uint64_t sol_id = str_to_hex(switch_number);

      char switch_state = num[2][0];
      // 0x99 and 0x9a cannot be both curved at once
      // if 0x99 is curved then 0x9a must be straight and vice versa
      if(sol_id == 0x99){
        if (switch_state == 'C'){
          solonoid_command(0x9a, 'S');
        }else{
          solonoid_command(0x9a, 'C');
        }
      } else if (sol_id == 0x9a){
        if (switch_state == 'C'){
          solonoid_command(0x99, 'S');
        }else{
          solonoid_command(0x99, 'C');
        }
      } else if (sol_id == 0x9b){
        if (switch_state == 'C'){
          solonoid_command(0x9c, 'S');
        }else{
          solonoid_command(0x9c, 'C');
        }
      } else if (sol_id == 0x9c){
        if (switch_state == 'C'){
          solonoid_command(0x9b, 'S');
        }else{
          solonoid_command(0x9b, 'C');
        }
      } else {
        // throw an error
        print_error("ERROR: INVALID SWITCH NUMBER");
        return 1;
      }
      solonoid_command(sol_id,  switch_state);
    } else {
      uint64_t sol_id = atoi_64(switch_number);
      if (sol_id > SWITCH_COUNT || sol_id < 1){
        print_error("ERROR: INVALID SWITCH NUMBER");
        return;
      }
      char switch_state = num[2][0];
      solonoid_command(sol_id,  switch_state);
    }
  } else {
    print_error("ERROR: INVALID COMMAND");
    return -1;
  }
  return 0;
}
*/