#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "custmath.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "gameserver.h"
#include "custstr.h"
#include "systimer.h"
#include "clockserver.h"
/*
Print 0 if it is a complete command
Print -1 if there is an error
*/
int k3ExecuteCommands(char *command, char **num, int command_part_count){
	  // the command is not found
      if (strcmp_ret(num[0], "time")){
         uart_printf(CONSOLE, "Ticks passed: %d\r\n", Time(WhoIs("clock_server")));
      } else if (strcmp_ret(num[0], "delay")){
        if (command_part_count != 4){
            uart_printf(CONSOLE, "Delay command: delay <name> <delaytime> <delaycount>, argcount = %d\r\n", command_part_count);
            return 0;
        }
        int delay = atoi_64(num[2]);
        int delaycount = atoi_64(num[3]);
        init_clock_proc(3, num[1], delay, delaycount);
      } else if (strcmp_ret(num[0], "delayuntil")) {
        if (command_part_count != 3){
            uart_printf(CONSOLE, "delayuntil command requires 1 argument, argcount = %d\r\n", command_part_count);
            return 0;
        }
      } else {
          // not a K3 command
		  return -1;
	  }
      return 1;
}


int32_t init_clock_proc(uint64_t priority, char *clockname_buf, int delay, int numberOfDelays){
    // uart_printf(CONSOLE, "init_clock_proc: clockname_buf = %s\r\n", clockname_buf);
    char clockname[8];
    for (int i = 0; i < 7; i++){
        clockname[i] = clockname_buf[i];
    }
    int tid = Create(priority, clock_proc);
    // uart_printf(CONSOLE, "init_clock_proc: tid = %d\r\n", tid);
    clockname[7] = '\0';
    // uart_printf(CONSOLE, "init_clock_proc: %s, %d\r\n", clockname, delay);
    if (Send(tid, clockname, 8, clockname, 8) < 0) return -1;
    // uart_printf(CONSOLE, "init_clock_proc: %s, %s\r\n", clockname, clockname);
    if (Send(tid, &delay, 4, &delay, 4) < 0) return -1;
    // uart_printf(CONSOLE, "init_clock_proc: %s, %d\r\n", clockname, delay);
    if (Send(tid, &numberOfDelays, 4, &numberOfDelays, 4)  < 0) return -1;
    // uart_printf(CONSOLE, "init_clock_proc: %s, %d\r\n", clockname, numberOfDelays);
    return 0;
}