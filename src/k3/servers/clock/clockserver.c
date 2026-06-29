#include "clockserver.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "asm.h"
#include "rpi.h"
#include "util.h"
#include "custstring.h"
#include "systimer.h"
#define CLOCKINTID 99

#define TICK_US 1000    /* 1 ms = 1 kHz (QEMU raspi4b's system-timer advances
                           painfully slowly in wall time; smaller increment
                           lets Delay(N) return in usable time. The kernel's
                           tick rate is still 100 Hz conceptually -- this is
                           just the polling stride). */

/*
 * clock_notifier (polling variant).
 *
 * The original notifier used AwaitEvent(CLOCKINTID) which depends on the
 * BCM2711 system-timer IRQ reaching the GIC-400 CPU interface. Under
 * QEMU raspi4b that wiring isn't reliably modelled, so AwaitEvent blocks
 * forever and Delay() never returns.
 *
 * Instead we poll the system-timer's free-running counter (SYSTIME_CLO).
 * The counter IS modelled by QEMU (get_timerLO returns advancing values),
 * and on real hardware it also works -- it just costs a Yield per check
 * instead of being IRQ-driven. Yielding gives other tasks the CPU between
 * polls.
 *
 * Priority 1 (not 0) so prio-0 servers (nameserver, display, IO) stay
 * responsive; a missed tick is recovered next pass because we drive the
 * deadline forward by exactly TICK_US even if we sampled late.
 */
void clock_notifier(){
    RegisterAs("clock_notifier");
    int clock_server_tid = -1;
    while (clock_server_tid == -1) {
        clock_server_tid = WhoIs("clock_server");
        if (clock_server_tid == -1) Yield();
    }

    uint32_t next_tick = get_timerLO() + TICK_US;

    while (1) {
        /* Wait for the free-running counter to catch up to our deadline.
           Signed subtraction handles wrap correctly. Yield each iteration
           so other tasks aren't starved. */
        while ((int32_t)(get_timerLO() - next_tick) < 0) Yield();
        next_tick += TICK_US;

        int ret;
        Send(clock_server_tid, "", 0, &ret, 0);
    }
    Exit();
}
void clock_server(){
    RegisterAs("clock_server");
    int waketicks[NUMPROCS];
    memset(waketicks, -1, NUMPROCS * sizeof(int));
    int ticks = 0;
    /* Re-resolve clock_notifier each loop until it appears: clock_notifier
       runs at a lower priority (3) than clock_server (0), so it cannot have
       registered by the time we reach this line on first call. Looking it up
       once and caching -1 was the original bug -- ticks from the notifier
       would then fall into the "unknown command" path and never get a Reply. */
    int not_tid = WhoIs("clock_notifier");
    while (1)
    {

        /* code */
        
        uint32_t tid;
        char command[50];
        command[0] = '\0';
        int rcvd_len = Receive(&tid, command, 50);
        /* Lazy-resolve notifier tid: it registers later than us. */
        if (not_tid <= 0) not_tid = WhoIs("clock_notifier");
        /* Treat any zero-length message as a tick notification regardless of
           the sender -- this lets us recover even if WhoIs("clock_notifier")
           hasn't resolved yet. */
        if (rcvd_len == 0 || tid == (uint32_t)not_tid) {
            for (int i = 0; i < NUMPROCS; i++) {
                if (waketicks[i] != -1 && waketicks[i] <= ticks) {
                    waketicks[i] = -1;
                    Reply(i, (char *)&ticks, 4);
                }
            }
            Reply(tid, (char *)&ticks, 0);
            ticks++;
            continue;
        }
        /*
        
        else if (tid != not_tid){
            // uart_printf(CONSOLE, "\033[35m");
            // uart_printf(CONSOLE, "clock_server: tid = %d called with ticks = %d\r\n", tid, ticks);
        }
        */
        char *num[10]; // array to store the numbers
		// int parse_char_arr(char *arr, char **num, int num_size)
        int command_part_count = 0;
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
        command_part_count = parse_char_arr(command, num, 10);
        if (strcmp_ret(num[0], "Time")){
            uint64_t delay = command[0];
            Reply(tid, &ticks, 4);
        } else if (strcmp_ret(num[0], "Delay")){
            // need to reply to the task that called delay after the delay count
            // print called delay from a certain PID
            int delay_ticks = atoi_64(num[1]);
            //// uart_printf(CONSOLE, "Delay called from %d delay_ticks = %d\r\n", tid, delay_ticks);
            waketicks[tid] = ticks + delay_ticks;
            //// uart_printf(CONSOLE, "waketicks[%d] = %d\r\n", tid, waketicks[tid]);
        } else if (strcmp_ret(num[0], "DelayUntil")){
            waketicks[tid] = atoi_64(num[1]);
        }
        ticks++;
    }
    Exit();
}

int Time(int tid){
    int ret = -1;
    if (Send(tid, "Time",5, &ret, 4) == -1)return -1;
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
    if (Send(tid, sendmsg, 50, &ticks, 4) == -1)return -1;
    return ticks;
}
// must provide the PID of the clock server
int DelayUntil(int tid, int ticks){
    char sendmsg[50] = "DelayUntil ";
    char str[5];
    i2a(ticks, str);
    strcat_cust(sendmsg, str);
    if (Send(tid, sendmsg, 50, &ticks, 4) == -1)return -1;
    return ticks;
}