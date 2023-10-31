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
  // clear screen
  uart_printf(CONSOLE, "\033[2J");
  // remove cursor
  uart_printf(CONSOLE, "\033[?25l");
  
  while(1){
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

void test_trains(){
  int clock_server_tid = WhoIs("clock_server");
  char train_numbers[] = {1, 2, 24, 47, 54, 58};
  char *num[3];
  char *command[100];//= "tr 54 10";
  strcpy(command, 100, "tr 54 10", 8);
  int command_part_count = parse_char_arr(command, train_numbers, 3);
  train_controller(command, num, command_part_count);
  uart_printf(CONSOLE, "test_trains: train 54 is set to speed 10\r\n");
  Delay(clock_server_tid, 100);
  // reverse the train
  strcpy(command, 100, "rv 54",7);
  parse_char_arr(command, train_numbers, 3);
  uart_printf(CONSOLE, "test_trains: train 54 is reversed\r\n");
  Delay(clock_server_tid, 100);
  // stop the train
  //command = "tr 54 0";
  strcpy(command, 100, "tr 54 0",7);
  parse_char_arr(command, train_numbers, 3);
  uart_printf(CONSOLE, "test_trains: train 54 is stopped\r\n");
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
    //int tr_exec = train_controller(command, num, command_part_count);
    //if(tr_exec != -1){
    //  return tr_exec;
    // }
    if (strcmp_ret(num[0], "putc")){
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
        
        if(strcmp_ret(num[1], "init")){
            // create process 
            // create a process that initializes the track
            // Create(1, &k4t1_init_test);

            return 0;
        }
        else if(strcmp_ret(num[1], "54") && strcmp_ret(num[2], "10")){
            // create process 
            // create a process that initializes the track
            execute_train_command(10, 54);
            return 0;
        }
        if(strcmp_ret(num[1], "54") && strcmp_ret(num[2], "0")){
            // create process 
            // create a process that initializes the track
            execute_train_command(0, 54);
            return 0;
        }
        
        return 0;
    } 
    return -1;
}
