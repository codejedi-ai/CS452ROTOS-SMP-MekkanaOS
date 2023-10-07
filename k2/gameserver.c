#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
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
// if program quits then the game would wait for another program to join
struct game
{
	uint32_t tid1;
	uint32_t tid2;
	char tid1_move[10];
	char tid2_move[10];
	uint32_t tid1_score;
	uint32_t tid2_score;
};
uint8_t full_game(struct game *cur_game)
{
	/* data */
	return cur_game->tid1 != 0 && cur_game->tid2 != 0;
}
uint8_t full_play(struct game *cur_game)
{
	/* data */
	return (!is_empty(cur_game->tid1_move) && !is_empty(cur_game->tid2_move));
};

int set_play(char* move, uint32_t tid, struct game *games){
	int game_no = 0;

	for (int i = 0; i < 10; i++)
	{
		if (games[i].tid1 == tid){
			games[i].tid1_move[0] = 0;
			strcpy(games[i].tid1_move, 10, move, 10);
			game_no = i;
			break;
		}
		if (games[i].tid2 == tid){
			games[i].tid2_move[0] = 0;
			strcpy(games[i].tid2_move, 10, move, 10);
			game_no = i;
			break;
		}
	}
	int tid1 = games[game_no].tid1;
	int tid2 = games[game_no].tid2;
	if (full_play(&games[game_no])){
		// print the plays
		uart_printf(CONSOLE, "gameserver: tid1_move = %s, tid2_move = %s\r\n", games[game_no].tid1_move, games[game_no].tid2_move);
		int victor = check_game(&games[game_no]);
		int repret1, repret2;
		if (victor == 1){
			// player 1 wins
			repret1 = Reply(tid1, "W", 2);
			repret2 = Reply(tid2, "L", 2);
		} else if (victor == 2){
			// player 2 wins
			repret1 = Reply(tid1, "L", 2);
			repret2 = Reply(tid2, "W", 2);
		} else{
			// draw
			repret1 = Reply(tid1, "D", 2);
			repret2 = Reply(tid2, "D", 2);
		}
		uart_printf(CONSOLE, "gameserver: repret1 = %d, repret2 = %d\r\n", repret1, repret2);
		return victor;
	}else{
		return 0;
	}
}
void reset_game(struct game *cur_game)
{
	/* data */
	cur_game->tid1_move[0] = 0;
	cur_game->tid2_move[0] = 0;

}

int check_game(struct game *cur_game){
	uart_printf(CONSOLE, "check_game: tid1_move = %s, tid2_move = %s\r\n", cur_game->tid1_move, cur_game->tid2_move);
	if (strcmp_ret(cur_game->tid1_move, "rock") && strcmp_ret(cur_game->tid2_move, "scissors")){
		reset_game(cur_game);
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "scissors") && strcmp_ret(cur_game->tid2_move, "paper")){
		reset_game(cur_game);
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "paper") && strcmp_ret(cur_game->tid2_move, "rock")){
		reset_game(cur_game);
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "rock") && strcmp_ret(cur_game->tid2_move, "paper")){
		reset_game(cur_game);
		cur_game->tid2_score++;
		return 2;
	} else if (strcmp_ret(cur_game->tid1_move, "scissors") && strcmp_ret(cur_game->tid2_move, "rock")){
		reset_game(cur_game);
		cur_game->tid2_score++;
		return 2;
	} else if (strcmp_ret(cur_game->tid1_move, "paper") && strcmp_ret(cur_game->tid2_move, "scissors")){
		reset_game(cur_game);
		cur_game->tid2_score++;
		return 2;
	}
	return 0;
}
void gameserver(){
	RegisterAs("gameserver");
	int tid;
	char msg[50];
	struct game games[10];
	int msglen = 49;
	for (int i = 0; i < 10; i++)
	{
		games[i].tid1 = 0;
		games[i].tid1_move[0] = 0;
		games[i].tid2 = 0;
		games[i].tid2_move[0] = 0;
	}
	while (1)
	{
		int recret = Receive(&tid, msg, msglen);
		// the message would be signup, quit, rock paper or scissors
		// if msg is signup
		if (strcmp_ret(msg, "signup")){
			// find a game that is not full
			uart_printf(CONSOLE, "gameserver: signup recieved\r\n");
			for (int i = 0; i < 10; i++)
			{
				
				if (!full_game(&games[i])){
					// this game is not full
					if (games[i].tid1 == 0){
						games[i].tid1 = tid;
						games[i].tid1_move[0] = 0;
					}
					else{
						games[i].tid2 = tid;
						games[i].tid2_move[0] = 0;
					}
					uart_printf(CONSOLE, "gameserver: game %d added player\r\n", i);
					break;
				}
			}
			int repret = Reply(tid, "recieved", 25);
			continue;
		} else if (strcmp_ret(msg, "quit")){
			// find a game that is not full
			uart_printf(CONSOLE, "gameserver: quit recieved\r\n");
			for (int i = 0; i < 10; i++)
			{
				if (games[i].tid1 == tid){
					games[i].tid1 = 0;
					games[i].tid1_move[0] = 0;
				}
				if (games[i].tid2 == tid){
					games[i].tid2 = 0;
					games[i].tid2_move[0] = 0;
				}
			}
			int repret = Reply(tid, "recieved", 25);
			continue;
		} else if (strcmp_ret(msg, "rock") || strcmp_ret(msg, "paper") || strcmp_ret(msg, "scissors")){
			// find a game that is not full
			uart_printf(CONSOLE, "TID: %u gameserver: %s recieved\r\n", tid, msg);
			
			// print player1 play
			uart_printf(CONSOLE, "TID: %u gameserver: player1 play: %s\r\n", games[0].tid1, games[0].tid1_move);
			// print player2 play
			uart_printf(CONSOLE, "TID: %u gameserver: player2 play: %s\r\n", games[0].tid2, games[0].tid2_move);
			//print_int(check);
			uint8_t vic = set_play(msg, tid, games);
			uart_printf(CONSOLE, "TID: %u gameserver: vic = %d\r\n", tid, vic);

			
			//int repret = Reply(tid, "W", 2);
		}
		
	}

}
void signup(){
	int pid = WhoIs("gameserver");
	char msg[25];
	Send(pid, "signup", 6, msg, 25);
}
void quit(){
	int pid = WhoIs("gameserver");
	char msg[25];
	Send(pid, "quit", 4, msg, 25);
}
char play(char* move){
	int pid = WhoIs("gameserver");
	char msg[25];

	Send(pid, move, 9, msg, 25);
	char retchar = (char* )msg[0];
	uart_printf(CONSOLE, "Result: msg = %c\r\n", retchar);
	strflush(msg, 25);
	return retchar;
}