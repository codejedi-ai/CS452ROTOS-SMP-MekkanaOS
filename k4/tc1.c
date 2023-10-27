#include "rpi.h"
#include "util.h"
#include "ioserver.h"
#include "custstr.h"
#include <stdio.h>
#include "tc1.h"
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
void enqueue(unsigned char byte_1, unsigned char byte_2 ){
    uint32_t io_server_pid = WhoIs("io_server");
    Putc(io_server_pid, MARKLIN, byte_1);
    Putc(io_server_pid, MARKLIN, byte_2);
}
void print_error(char *error){
    // print in red
    uart_printf(CONSOLE, "\033[31m");
    uart_printf(CONSOLE, "%s\r\n", error);
    // print in white
    uart_printf(CONSOLE, "\033[37m");
}
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id){  // Binary: 00000001)
      enqueue(speed, id);
      trains_speed[id] = speed;
}
void execute_reverse_command(unsigned char id){  // Binary: 00000001)
      enqueue(0, id);
      enqueue(15, id);
      enqueue(trains_speed[id], id);
}
void solonoid_command(unsigned char solonoid_id, // Solonoid ID. . 
                      unsigned char direction){  // S 33 go straight, C 34 go bent
    char byte_1 = 0;
    char byte_2 = 0;
    if (direction ==  'C')  byte_1 = 34;  
    else if (direction ==  'S')  byte_1 = 33;
    else{
      print_error("ERROR: INVALID DIRECTION");
      return;
    }

    byte_2 = solonoid_id;
    enqueue(byte_1, byte_2);
    sol_on_time = get_timerLO();
    sw_states[solonoid_id] = direction;
    sol_off();
}
void sol_off(){  // Solonoid ID
    enqueue(32, '\r');
}
// define a function that takes a char array as a parameter
//void tc1(char *arr) {
void tc1(char *command, char **num, int command_part_count){
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
      return;
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
      return;
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
        return;
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
  }
}
void init_track(){
  char train_numbers[] = {1, 2, 24, 47, 54, 58};
  int train_count = 6;
    // set all train speed to 0
  for (uint8_t i = 0; i < train_count; i ++){
    execute_train_command(0, train_numbers[i]);
    execute_train_command(64, train_numbers[i]);
  }
  // set all the turnabouts to straight
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'S');
    // this command only enqueues the switches
  }
  // uart_printf(CONSOLE,"\033[%u;%uHSWITCHES ALL STRAIGHT:",TOP_ROW + COMMAND_ROW, LEFT_COL + 1);
  // set all the turnabouts to curved
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'C');
  }
  solonoid_command(0x99, 'S');
  solonoid_command(0x9a, 'C');
  solonoid_command(0x9b, 'S');
  solonoid_command(0x9c, 'C');

  solonoid_command(0x99, 'C');
  solonoid_command(0x9a, 'S');
  solonoid_command(0x9b, 'C');
  solonoid_command(0x9c, 'S');
}