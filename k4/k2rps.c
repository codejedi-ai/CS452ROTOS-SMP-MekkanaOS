#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "systimer.h"
#include "int64voodoo.h"
#include "k2rps.h"
#define DEBUG 1
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

// there would be another terminal like think here
char rock_play(int i, int tid){
    // do nothing
    char play_ret = play("rock");
    // play rock paper scissors
    uart_printf(CONSOLE, "Play: %u, player: %d have: ",i, tid);
    uart_putc(CONSOLE, (char)play_ret);
    uart_printf(CONSOLE, "\r\n");
    return play_ret;
}
char paper_play(int i, int tid){
	// play rock paper scissors
    char play_ret = play("paper");
    // play rock paper scissors
    uart_printf(CONSOLE, "Play %u, player:%d have: ",i, tid);
    uart_putc(CONSOLE, (char)play_ret);
    uart_printf(CONSOLE, "\r\n");
    return play_ret;
}

char scissors_play(int i, int tid){
    char play_ret = play("scissors");
    // play rock paper scissors
    uart_printf(CONSOLE, "Play %u, player:%d have: ",i, tid);
    uart_putc(CONSOLE, (char)play_ret);
    uart_printf(CONSOLE, "\r\n");
    return play_ret;
}
char random_play(int i, int tid){
    unsigned int time = get_timerLO();
    // if time is a multiple of 3
    char play_ret = ' ';
    char *move = " ";
    if (time % 3 == 0){
        play_ret = play("rock");
        move = "rock";
    } else if (time % 3 == 1){
        play_ret = play("paper");
        move = "paper";
    } else if (time % 3 == 2){
        play_ret = play("scissors");
        move = "scissors";
    }
    // play rock paper scissors
    uart_printf(CONSOLE, "Play: %u, Played: %s player: %d have: ", i, move, tid);
    uart_putc(CONSOLE, (char)play_ret);
    uart_printf(CONSOLE, "\r\n");
    return play_ret;
}

char function_stack(uint64_t i, uint64_t tid, uint64_t type){
    if (type == 0){
        return (i, tid);
    } else if (type == 1){
        return paper_play(i, tid);
    } else if (type == 2){
        return scissors_play(i, tid);
    } else if (type == 3){
        return random_play(i, tid);
    }
}

void player(){
    int tid = MyTid();
    // register
    int msglen = 50;
    char num[50];
    char name[50] = "player-";
    ui2a(tid, 10, num);
    strcat_cust(name, num);
    uart_printf(CONSOLE, "Player: %s\r\n", name);
    RegisterAs(name);
    // get the values of N and player_type
    Receive(&tid, name, 2);
    uint64_t N =  name[0];
    uint64_t player_type = name[1];
    Reply(tid, name, 2);
    uart_printf(CONSOLE, "%s: player_type - %u, N: %u\r\n", player_type, N);
    signup();
    tid = MyTid();
    for (uint64_t i = 0; i < N; i++)
    {
        if (function_stack(i, tid, player_type) == 'K'){
            // this means the server have turned off
            Exit();
        };
    }


    quit();
    Exit();
}


// init player takes in a void function
void initPlayer(uint64_t N, uint64_t player_type, uint64_t priority){
    // create player
    int tid = Create(priority, player);
    // register
    int msglen = 50;
    char name[50];
    name[0] = N;
    name[1] = player_type;
    uart_printf(CONSOLE, "initPlayer: %d\r\n", Send(tid, name, 2, name, 2));
}
