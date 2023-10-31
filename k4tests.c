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
    uint16_t ret[s88_no];
    for (int i = 0; i < s88_no; i ++){
        ret[i] = 0;
    }
    uint32_t outchar = read_many_s88(s88_no, ret);
  }
  Exit();
}
/*
Return 0 upon successful execution
Return 1 if the command is not valid
Return -1 if the command is not found
*/

void init_trains(){
  char train_numbers[] = {1, 2, 24, 47, 54, 58};
  int train_count = 6;
    // set all train speed to 0
  for (uint8_t i = 0; i < train_count; i ++){
    execute_train_command(0, train_numbers[i]);
    execute_train_command(64, train_numbers[i]);
  }
  uart_printf(CONSOLE, "init_trains: All trains are set to speed 0\r\n");
  Exit();
}

void init_track_test(){
    // set all the turnabouts to straight
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'S');
    // this command only enqueues the switches
  }
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
  Exit();
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
        int ioserver_PID = WhoIs("io_server_MARKLIN");
        Putc(ioserver_PID, MARKLIN, print_char2);
        awaitCTS(ioserver_PID, MARKLIN, 1);
        return 0;
    } else if (strcmp_ret(num[0], "k4tc")){
        int tid;
        if(strcmp_ret(num[1], "init")){
            // create process 
            // create a process that initializes the track
            // Create(1, &k4t1_init_test);
            
            tid = Create(2, init_track_test);
            uart_printf(CONSOLE, "init_track_test: tid = %d\r\n", tid);
            tid = Create(3, init_trains);
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
        execute_reverse_command(train_number);
        return 0;
      }
      // sw <switch number> <C/S>
      if (strcmp_ret(num[1], "sw")){
        char switch_number = atoi_64(num[2]);
        char direction = *num[3];
        solonoid_command(switch_number, direction);
        return 0;
      }
    
    return -1;
}
