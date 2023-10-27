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

/*
Return 0 upon successful execution
Return 1 if the command is not valid
Return -1 if the command is not found

*/
int k4ExecuteCommands(char *command, char **num, int command_part_count){
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
        int ioserver_PID = WhoIs("io_server");
        Putc(ioserver_PID, MARKLIN, print_char2);
        return 0;
    } else if (strcmp_ret(num[0], "getc")){
        if (command_part_count != 1){
            uart_printf(CONSOLE, "getc command requires no argument, argcount = %d\r\n", command_part_count);
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
        int ioserver_PID = WhoIs("io_server");
        char ret_char = (char)Getc(ioserver_PID, MARKLIN);
        uart_printf(CONSOLE, "ret_char = \"%c\" = \"%d\"\r\n", ret_char, ret_char);
        return 0;
    } 
    return -1;
}
