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
int k3ExecuteCommands(char *command, char **num, int command_part_count)
{
    // the command is not found
    int mypriority = MyPriority();
    int clock_server_pid = WhoIs("clock_server");
    if (strcmp_ret(num[0], "time"))
    {
        uart_printf(CONSOLE, "\033[34m");
        uart_printf(CONSOLE, "Ticks passed: %d\r\n", Time(clock_server_pid));
    }
    else if (strcmp_ret(num[0], "delay"))
    {
        if (command_part_count != 4)
        {
            uart_printf(CONSOLE, "\033[34m");
            uart_printf(CONSOLE, "Delay command: delay <name> <delaytime> <delaycount>, argcount = %d\r\n", command_part_count);
            return 0;
        }
        int delay = atoi_64(num[2]);
        int delaycount = atoi_64(num[3]);
        init_clock_proc(mypriority - 1, num[1], delay, delaycount);
    }
    else if (strcmp_ret(num[0], "delayuntil"))
    {
        if (command_part_count != 3)
        {
            uart_printf(CONSOLE, "\033[34m");
            uart_printf(CONSOLE, "delayuntil command requires 1 argument, argcount = %d\r\n", command_part_count);
            return 0;
        }
    }
    else
    {
        // not a K3 command
        return -1;
    }
    return 1;
}

void k3_clock_proc()
{
    uint64_t tid = MyTid();
    char name[8] = "k3_clock_proc";
    int delay = 10;
    int numberOfDelays = 20;
    

    Receive(&tid, name, 8);
    uart_printf(CONSOLE, "\033[36m");
    uart_printf(CONSOLE, "k3_clock_proc-%s: tid = %d, \r\n", name, tid);
    Reply(tid, name, 8);
    RegisterAs(name);
    Receive(&tid, &delay, 4);
    Reply(tid, &delay, 4);
    uart_printf(CONSOLE, "\033[36m");
    uart_printf(CONSOLE, "k3_clock_proc-%s: tid = %d, delay = %d\r\n",name, tid, delay);

    Receive(&tid, &numberOfDelays, 4);
    Reply(tid, &numberOfDelays, 4);
    uart_printf(CONSOLE, "\033[36m");
    uart_printf(CONSOLE, "k3_clock_proc-%s: tid = %d, delay = %d, numberOfDelays = %d\r\n", name, tid, delay, numberOfDelays);

    RegisterAs(name);
    int clock_server_pid = WhoIs("clock_server");
    uart_printf(CONSOLE, "\033[36m");
    uart_printf(CONSOLE, "k3_clock_proc-%s: clock_server_pid = %d\r\n", name, clock_server_pid);
    for (int i = 0; i < numberOfDelays; i++)
    {
        uart_printf(CONSOLE, "\033[36mk3_clock_proc-%s: Goin to sleep\r\n", name);
        if (clock_server_pid == -1)
        {
            // print in red
            uart_printf(CONSOLE, "\033[31m");
            uart_printf(CONSOLE, "k3_clock_proc-%s: clock_server_pid not found\r\n");
            Exit();
        }
        int delayret = Delay(clock_server_pid, delay);
        uart_printf(CONSOLE, "\033[36m");
        uart_printf(CONSOLE, "k3_clock_proc-%s: i = %d, %s Reawakened after %d ticks\r\n", i, (char *)name, delay);
    }

    uart_printf(CONSOLE, "\033[36m");
    uart_printf(CONSOLE, "k3_clock_proc-%s: Clock proc exit \r\n",name);

    Exit();
}
int32_t init_clock_proc(uint64_t priority, char *clockname_buf, int delay, int numberOfDelays)
{
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "init_clock_proc: clockname_buf = %s\r\n", clockname_buf);
    char clockname[8];
    for (int i = 0; i < 7; i++)
    {
        clockname[i] = clockname_buf[i];
    }
    int tid = Create(priority, k3_clock_proc);
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "init_clock_proc: tid = %d\r\n", tid);
    clockname[7] = '\0';
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "init_clock_proc: %s, %d\r\n", clockname, delay);
    if (Send(tid, clockname, 8, clockname, 8) < 0)
        return -1;
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "init_clock_proc: %s, %s\r\n", clockname, clockname);
    if (Send(tid, &delay, 4, &delay, 4) < 0)
        return -1;
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "init_clock_proc: %s, %d\r\n", clockname, delay);
    if (Send(tid, &numberOfDelays, 4, &numberOfDelays, 4) < 0)
        return -1;
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "init_clock_proc: %s, %d\r\n", clockname, numberOfDelays);
    return tid;
}

void k3FirstUserTask()
{
    // First task as dictated in the reqs
    // need to set the timer interrupt
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "Timer C3: %u\r\n", get_timerC3());

    // We are assuming that FirstUserTask has a priority of 1
    // start gameserver
    RegisterAs("k3FirstUserTask");

    char clockproc1[8] = "cl10";
    int clockproc1tid = init_clock_proc(3, clockproc1, 10, 20);
    uart_printf(CONSOLE, "\033[34m%d\r\n", clockproc1tid);
    char clockproc2[8] = "cl23";
    int clockproc2tid =  init_clock_proc(4, clockproc2, 23, 9);
    uart_printf(CONSOLE, "\033[34m%d\r\n",clockproc2tid);
    char clockproc3[8] = "cl33";
    int clockproc3tid = init_clock_proc(5, clockproc3, 33, 6);
    uart_printf(CONSOLE, "\033[34m%d\r\n", clockproc3tid);
    char clockproc4[8] = "cl71";
    int clockproc4tid = init_clock_proc(6, clockproc4, 71, 3);
    uart_printf(CONSOLE, "\033[34m%d \r\n", clockproc4tid);
    // Create(2000, main);
    uart_printf(CONSOLE, "\033[34m");
    uart_printf(CONSOLE, "k3FirstUserTask: Completed\r\n");
    Exit();
}