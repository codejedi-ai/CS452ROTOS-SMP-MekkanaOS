#include "clockserver.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "asm.h"
#include "rpi.h"
#include "util.h"
#include "custstr.h"
#define CLOCKINTID 99
void clock_notifier(){
    RegisterAs("clock_notifier");
    int clock_server_tid = -1;
    while (clock_server_tid == -1){
        clock_server_tid = WhoIs("clock_server");
    }
    // spin until clock server is registered
    //  print in green
    // uart_printf(CONSOLE, "\033[32m");
    // uart_printf(CONSOLE, "clock_notifier: Registered\n");
    // uart_printf(CONSOLE, "\033[37m");
    while (1)
    {
        (void)AwaitEvent(CLOCKINTID);
        int ret;
        Send(clock_server_tid, "", 0, (char *)&ret, 0);
    }
    Exit();
}
void clock_server(){
    RegisterAs("clock_server");
    // uart_printf(CONSOLE, "clock_server: Registered\n");
    int waketicks[NUMPROCS];
    memset(waketicks, -1, NUMPROCS * sizeof(int));
    int ticks = 0;
    const int not_tid = WhoIs("clock_notifier");
    while (1)
    {

        /* code */
        
        int tid;
        char command[50];
        Receive(&tid, command, 50);
        /*
        
        else if (tid != not_tid){
            // uart_printf(CONSOLE, "\033[35m");
            // uart_printf(CONSOLE, "clock_server: tid = %d called with ticks = %d\r\n", tid, ticks);
        }
        */
        char *num[10]; // array to store the numbers
        for (int i = 0; i < NUMPROCS; i++)
        {
            if (waketicks[i] != -1 && waketicks[i] <= ticks){
                //// uart_printf(CONSOLE, "\033[35mWaking up %d\r\n", i);
                waketicks[i] = -1;
                Reply(i, &ticks, 4);
            }
        }
        
         if (tid == not_tid){
            Reply(tid, &ticks, 0);
            ticks++;
            continue;
        } 
        //// uart_printf(CONSOLE, "%s called\r\n", command);
        (void)parse_char_arr(command, num, 10);
        if (strcmp_ret(num[0], "Time", 0)){
            Reply(tid, &ticks, 4);
        } else if (strcmp_ret(num[0], "Delay", 0)){
            // need to reply to the task that called delay after the delay count
            // print called delay from a certain PID
            int delay_ticks = atoi_64(num[1]);
            //// uart_printf(CONSOLE, "Delay called from %d delay_ticks = %d\r\n", tid, delay_ticks);
            waketicks[tid] = ticks + delay_ticks;
            //// uart_printf(CONSOLE, "waketicks[%d] = %d\r\n", tid, waketicks[tid]);
        } else if (strcmp_ret(num[0], "DelayUntil", 0)){
            waketicks[tid] = atoi_64(num[1]);
        }
        ticks++;
    }
    Exit();
}

int Time(int tid){
    int ret = -1;
    if (Send(tid, "Time",5, (char *)&ret, 4) == -1)return -1;
    return ret;
}
// must provide the PID of the clock server
int Delay(int tid, int ticks){
    // print in megenta
    // // uart_printf(CONSOLE, "\033[35m");
    // // uart_printf(CONSOLE, "Delay called from %d ticks = %d\r\n", tid, ticks);
    char sendmsg[50] = "Delay ";
    char str[5];
    i2a(ticks, str);
    strcat_cust(sendmsg, str);
    if (Send(tid, sendmsg, 50, (char *)&ticks, 4) == -1)return -1;
    return ticks;
}
// must provide the PID of the clock server
int DelayUntil(int tid, int ticks){
    char sendmsg[50] = "DelayUntil ";
    char str[5];
    i2a(ticks, str);
    strcat_cust(sendmsg, str);
    if (Send(tid, sendmsg, 50, (char *)&ticks, 4) == -1)return -1;
    return ticks;
}