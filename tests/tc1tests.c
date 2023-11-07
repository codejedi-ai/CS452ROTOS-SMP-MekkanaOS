#include "tc1tests.h"
#include "../rpi.h"
#include "../syscall.h"
#include "../nameserver.h"
#include "../custstr.h"
#include "../systimer.h"
#include "../clockserver.h"
#include "../marklin_worker.h"
#include "../ioserver.h"
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
int tc1ExecuteCommands(char *command, char **num, int command_part_count){
  // if command is in the form of tc trainid speed
  int MCC_tid = WhoIs("MCW");
  if (strcmp_ret(command, "tc")){
    int trainid = atoi_64(num[1]);
    int speed = atoi_64(num[2]);
    if (trainid < 1 || trainid > 80 || speed < 0 || speed > 14){
      uart_printf(CONSOLE, "Invalid trainid or speed\r\n");
      return 1;
    }
    // set the speed of the train
    set_train_state(MCC_tid, trainid, speed);
    return 0;
  }
  // if the command is in the form of sw switchid direction
  if (strcmp_ret(command, "sw")){
    int switchid = atoi_64(num[1]);
    int direction = num[2][0];
    if (switchid < 1 || switchid > 18 || (direction != 'C' && direction != 'S')){
      uart_printf(CONSOLE, "Invalid switchid or direction\r\n");
      return 1;
    }
    // set the direction of the switch
    set_solonoid(MCC_tid, switchid, direction);
    return 0;
  }
  // if the command is in the form of rv trainid
  if (strcmp_ret(command, "rv")){
    int trainid = atoi_64(num[1]);
    int speed = atoi_64(num[2]);
    if (trainid < 1 || trainid > 80){
      uart_printf(CONSOLE, "Invalid trainid\r\n");
      return 1;
    }
    set_reverse(MCC_tid, trainid, speed);
    return 0;
  }
  return 2;
}