#include "k4tests.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "k2rps.h"
#include "clockserver.h"
#include "k3tests.h"
#include "asm.h"
#include "ioserver.h"
#include "traincont.h"
#define delay 0
/*

code functions on
64 all off
65 #1
66 #2
67 #1,#2
68 #3
69 #3,#1
70 #3,#2
71 #3,#2,#1
72 #4
73 #4,#1
74 #4,#2
75 #4,#2,#1
76 #4,#3
77 #4,#3,#1
78 #4,#3,#2
79 #4,#3,#2,#1


*/
// this function takes in a list of functions on the train to turn on
/*
int ret_train_state_with_fun(int function){
  if(function == 0) return 64;

}
*/
/*
void read_s88_test_sensor_A(){
    uart_printf(CONSOLE, "First s88_id = 1");
    uint32_t outchar = read_one_s88(1);
    uart_printf(CONSOLE, "outchar = %u\r\n", outchar);
    uart_printf(CONSOLE, "TEST read_s88_1 Completed!!! \r\n");
    Exit();
}
*/
/*
Return 0 upon successful execution
Return 1 if the command is not valid
Return -1 if the command is not found
*/
/*
void init_trains(){
  char train_numbers[] = {1, 2, 24, 47, 54, 58, 77};
  int train_count = 7;
    // set all train speed to 0
  for (uint8_t i = 0; i < train_count; i ++){
    execute_train_command(0, train_numbers[i]);
    execute_train_command(64, train_numbers[i]);
  }
  uart_printf(CONSOLE, "init_trains: All trains are set to speed 0\r\n");
}
void trains_honk(){
  char train_numbers[] = {1, 2, 24, 47, 54, 58, 77};
  int train_count = 7;
    // set all train speed to 0
  for (uint8_t i = 0; i < train_count; i ++){
    execute_train_command(16, train_numbers[i]);
  }
  uart_printf(CONSOLE, "init_trains: All trains are set to speed 0\r\n");
}

void init_track_test(){
    // set all the turnabouts to straight
  int clock_server_tid = WhoIs("clock_server");
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'S');
    Delay(clock_server_tid, delay);
  }
  // set all the turnabouts to curved
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'C');
    Delay(clock_server_tid, delay);
  }
  solonoid_command(0x99, 'S');
  Delay(clock_server_tid, delay);
  solonoid_command(0x9a, 'C');
  Delay(clock_server_tid, delay);
  solonoid_command(0x9b, 'S');
  Delay(clock_server_tid, delay);
  solonoid_command(0x9c, 'C');
  Delay(clock_server_tid, delay);
  solonoid_command(0x99, 'C');
  Delay(clock_server_tid, delay);
  solonoid_command(0x9a, 'S');
  Delay(clock_server_tid, delay);
  solonoid_command(0x9b, 'C');
  Delay(clock_server_tid, delay);
  solonoid_command(0x9c, 'S');
  Delay(clock_server_tid, delay);
}


*/