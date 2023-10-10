#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "performancemeasure.h"
#include "systimer.h"
/*
These are the most essential terminal control sequences that you will need for your train program.

Code	Effect
"\033[2J"	Clear the screen.
"\033[H"	Move the cursor to the upper-left corner of the screen.
"\033[r;cH"	Move the cursor to row r, column c. Note that both the rows and columns are indexed starting at 1.
"\033[?25l"	Hide the cursor.
"\033[K"	Delete everything from the cursor to the end of the line.
These control sequences can help make your program's display more lively.

Code	Effect
"\033[0m"	Reset special formatting (such as colour).
"\033[30m"	Black text.
"\033[31m"	Red text.
"\033[32m"	Green text.
"\033[33m"	Yellow text.
"\033[34m"	Blue text.
"\033[35m"	Magenta text.
"\033[36m"	Cyan text.
"\033[37m"	White text.

*/
/*
5.2.1. Measure multiple times
for (i=0;i<N;i++) {
  clock()
  something()
  clock()
}
gives you N measurements instead of 1
can use this to quantify/understand variability, or smooth it
e.g, compute variance, look for outliers
doesn’t help with overhead problem
doesn’t help with precision problem
*/

void Measure_multiple_times(int N, void (*functionPtr) ()){
    int i;
    for (i = 0; i < N; i++){
        unsigned int start = get_timerLO();
        functionPtr();
        unsigned int end = get_timerLO();
        unsigned int time = end - start;
        uart_printf(CONSOLE, "Time taken for iteration %d is %u\r\n", i, time);
    }
    Exit();
}
/*
5.2.2. Measure something larger
start = clock()
for (i=0;i<N;i++) {
  something()
}
end = clock()
can smooth variability (via averaging) but does not help you measure/understand it
helps with overhead - increase N until overhead is insignificant
helps with precision - increase N until what you’re measuring is much slower than your clock
*/
void Measure_something_larger(int N, void (*functionPtr) ()){
    unsigned int start = get_timerLO();
    int i;
    for (i = 0; i < N; i++){
        functionPtr();
    }
    unsigned int end = get_timerLO();
    unsigned int time = end - start;
    uart_printf(CONSOLE, "Time taken for iteration %d is %u\r\n", i, time);
    Exit();
}