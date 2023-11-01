#include "k4tests.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "k2TimeTests.h"
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
void read_s88_test_sensor_A(){
    uart_printf(CONSOLE, "First s88_id = 1");
    uint32_t outchar = read_one_s88(1);
    uart_printf(CONSOLE, "outchar = %u\r\n", outchar);
    uart_printf(CONSOLE, "TEST read_s88_1 Completed!!! \r\n");
    Exit();
}
void read_s88_test_many(){
  set_io_logging(1);
  uart_printf(CONSOLE, "\033[2J");
  // hide cursor
  uart_printf(CONSOLE, "\033[?25l");
  while(1){
    // clear the screen
    // set cursor to 0,0
    uart_printf(CONSOLE, "\033[H");

    char s88_no = 5;
    uint64_t ret;
    ret = 0;
    uint32_t outchar = read_many_s88(s88_no, &ret);
    // print in green
    uart_printf(CONSOLE, "\033[32m");
    uart_printf(CONSOLE, "Sensor Data:");
    for (int i = 0; i < s88_no; i ++){
      uart_putc(CONSOLE, ' ');
      uart_putc(CONSOLE, 'A' + i);
      uart_printf(CONSOLE, "%d", (uint8_t)*((uint8_t*)&ret + i));
    }
  }
  Exit();
}
/*
Return 0 upon successful execution
Return 1 if the command is not valid
Return -1 if the command is not found
*/

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

int k4ExecuteCommands(char *command, char **num, int command_part_count){
    //if(tr_exec != -1){
    //  return tr_exec;
    // }
    // run the set_io_logging command

      if (strcmp_ret(num[0], "set_io_logging")){
        if (command_part_count != 2){
            uart_printf(CONSOLE, "set_io_logging command requires 1 argument, argcount = %d\r\n", command_part_count);
            return 1;
        }
        int io_logging = atoi_64(num[1]);
        set_io_logging(io_logging);
        return 0; 
      }if (strcmp_ret(num[0], "putc")){
        if (command_part_count != 2){
            uart_printf(CONSOLE, "putc command requires 1 argument, argcount = %d\r\n", command_part_count);
            return 1;
        }
        /*
        char *print_char = num[1];
        while(*print_char != '\0'){
            uart_putc(CONSOLE, *print_char);
            print_char++;
        }
        */
        char print_char2 = *num[1];
        uart_printf(CONSOLE, "print_char[0] = \"%c\" = \"%d\"\r\n", print_char2, print_char2);
        /*
            io_TXIC_MARKLIN_server_pid = WhoIs("io_TXIC_MARKLIN_server");
    io_RXIC_MARKLIN_server_pid = WhoIs("io_RXIC_MARKLIN_server");
    io_CTS_MARKLIN_server_pid = WhoIs("io_CTS_MARKLIN_server");
    */
//        int ioserver_PID = WhoIs("io_TXIC_MARKLIN_server");
        Putc(MARKLIN_PUT_SERVER(), MARKLIN, print_char2);
        return 0;
    } else if (strcmp_ret(num[0], "k4tc")){
        int tid;
        if(strcmp_ret(num[1], "init")){
            // create process 
            // create a process that initializes the track
            // Create(1, &k4t1_init_test);
            
            init_track_test();
            init_trains();
            uart_printf(CONSOLE, "init_trains: tid = %d\r\n", tid);
            return 0;
        }if(strcmp_ret(num[1], "read-many")){
            // create process 
            // create a process that initializes the track
            // Create(1, &k4t1_init_test);

            tid = Create(5, read_s88_test_many);
            uart_printf(CONSOLE, "read_s88_test_many: tid = %d\r\n", tid);
            return 0;
        }
        if(strcmp_ret(num[1], "read-A")){
            tid = Create(1, read_s88_test_sensor_A);
            uart_printf(CONSOLE, "read_s88_test_sensor_A: tid = %d\r\n", tid);
        }
      }
      if (strcmp_ret(num[1], "tr")){
        if(command_part_count != 4){
          uart_printf(CONSOLE, "tr command requires 3 arguments, argcount = %d\r\n", command_part_count);
          return 1;
        }
        int speed = atoi_64(num[3]);
        int train_number = atoi_64(num[2]);
        uart_printf(CONSOLE, "tr command: speed = %d, train_number = %d\r\n", speed, train_number);
        execute_train_command(speed, train_number);
        return 0;
        
      }
      // rv <train number>
      if (strcmp_ret(num[1], "rv")){
        if(command_part_count != 3){
          uart_printf(CONSOLE, "rv command requires 2 arguments, argcount = %d\r\n", command_part_count);
          return 1;
        }
        int train_number = atoi_64(num[2]);
        // execute_reverse_command(train_number);
        return 0;
      }
      // sw <switch number> <C/S>
      if (strcmp_ret(num[1], "sw")){
        if(command_part_count != 4){
          uart_printf(CONSOLE, "sw command requires 3 arguments, argcount = %d\r\n", command_part_count);
          return 1;
        }
        char switch_number = atoi_64(num[2]);
        char direction = *num[3];
        solonoid_command(switch_number, direction);
        return 0;
      }
      if (strcmp_ret(num[1], "honk")){
        trains_honk();
        return 0;
      }
    
    return -1;
}
