#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
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
// if program quits then the game would wait for another program to join
uint8_t full_game(struct game *cur_game)
{
	/* data */
	return cur_game->tid1 != 0 && cur_game->tid2 != 0;
}
uint8_t full_play(struct game *cur_game)
{
	/* data */
	return (!is_empty(cur_game->tid1_move) && !is_empty(cur_game->tid2_move));
}

int set_play(char* move, int tid, struct game *games){
	for (int i = 0; i < 10; i++)
	{
		if (games[i].tid1 == tid){
			games[i].tid1_move[0] = 0;
			cust_strcpy(games[i].tid1_move, 10, move, 10);
			return i;
		}
		if (games[i].tid2 == tid){
			games[i].tid2_move[0] = 0;
			cust_strcpy(games[i].tid2_move, 10, move, 10);
			return i;
		}
	}
	return -1;
}
void reset_game(struct game *cur_game)
{
	/* data */
	cur_game->tid1_move[0] = 0;
	cur_game->tid2_move[0] = 0;

}

int check_game(struct game *cur_game){

	// check for draw
	if (!full_play(cur_game)){
		return -1;
	}
	if (strcmp_ret(cur_game->tid1_move, "rock", 0) && strcmp_ret(cur_game->tid2_move, "scissors", 0)){
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "scissors", 0) && strcmp_ret(cur_game->tid2_move, "paper", 0)){
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "paper", 0) && strcmp_ret(cur_game->tid2_move, "rock", 0)){
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "rock", 0) && strcmp_ret(cur_game->tid2_move, "paper", 0)){
		cur_game->tid2_score++;
		return 2;
	} else if (strcmp_ret(cur_game->tid1_move, "scissors", 0) && strcmp_ret(cur_game->tid2_move, "rock", 0)){
		cur_game->tid2_score++;
		return 2;
	} else if (strcmp_ret(cur_game->tid1_move, "paper", 0) && strcmp_ret(cur_game->tid2_move, "scissors", 0)){
		cur_game->tid2_score++;
		return 2;
	}
	if (strcmp_ret(cur_game->tid1_move, cur_game->tid2_move, 0)){
		return 0;
	}
	return -1;
}
int ingame(int tid, struct game *games){
	for (int i = 0; i < 10; i++)
	{
		if (games[i].tid1 == tid || games[i].tid2 == tid){
			return 1;
		}
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
	const int mainpid = WhoIs("main");
	while (1)
	{
		for (int i = 0; i < 10; i++)
		{
			if (!full_game(&games[i])){
				// print the plays
				// if one of the PIDs equal to main's pid then we need to reply to main
				
				if (mainpid == games[i].tid2 || mainpid == games[i].tid1){
					(void)Reply(mainpid, "Q", 2);
				}
			}
		}
		(void)Receive(&tid, msg, msglen);
		// // uart_printf(CONSOLE, "gameserver: Message recieved: [%s], recret = %d\r\n", msg, recret);
		// the message would be signup, quit, rock paper or scissors
		// if msg is signup
		if (strcmp_ret(msg, "shutdown", 0)){
			// shutdown
			for (int i = 0; i < 10; i++)
			{
				if (games[i].tid1 != 0){
					(void)Reply(games[i].tid1, "K", 9);
				}
				if (games[i].tid2 != 0){
					(void)Reply(games[i].tid2, "K", 9);
				}
			}
			Reply(tid, "+", 9);
			Exit();
		}
		else if (strcmp_ret(msg, "signup", 0)){
			// find a game that is not full
			// print welcome message
			// uart_printf(CONSOLE, "Welcome to Rock Paper Scissors player-%u\r\n", tid, 0);

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
					if (full_game(&games[i])){
						// print the plays
						// uart_printf(CONSOLE, "Game %u: Player 1: %u, Player 2: %u\r\n", i, games[i].tid1, games[i].tid2);
					}
					break;
				}
			}
			(void)Reply(tid, "recieved", 25);
			continue;
		} else if (strcmp_ret(msg, "quit", 0)){
			// find a game that is not full
			if (!ingame(tid, games)){
				// print error message
				//// uart_printf(CONSOLE, "You are not in a game %u\r\n", tid);
				(void)Reply(tid, "E", 2);
				continue;
			}
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
				// if one of them is the main then we also need to reply hte main with the letter Q
				if (mainpid == games[i].tid2 || mainpid == games[i].tid1){
					// uart_printf(CONSOLE, "Your friend has quit the game\r\n");
					(void)Reply(mainpid, "Q", 2);
				}
			}
			(void)Reply(tid, "recieved", 25);
			continue;
		} else if (strcmp_ret(msg, "rock", 0) || strcmp_ret(msg, "paper", 0) || strcmp_ret(msg, "scissors", 0)){
			// find a game that is not full
			// check is the player is in a game
			if (!ingame(tid, games)){
				// print error message
				//// uart_printf(CONSOLE, "You are not in a game %u\r\n", tid);
				(void)Reply(tid, "E", 2);
				continue;
			}
			
			// print player1 play

			// print player2 play

			//print_int(check);
			uint8_t game_no = set_play(msg, tid, games);


			int tid1 = games[game_no].tid1;
			int tid2 = games[game_no].tid2;
			if (full_play(&games[game_no])){
				// print the plays

				int victor = check_game(&games[game_no]);

				reset_game(&games[game_no]);

				if (victor == 1){
					// player 1 wins
					(void)Reply(tid1, "W", 2);
					(void)Reply(tid2, "L", 2);
				} else if (victor == 2){
					// player 2 wins
					(void)Reply(tid1, "L", 2);
					(void)Reply(tid2, "W", 2);
				} else if (victor == 0){
					// draw
					(void)Reply(tid1, "D", 2);
					(void)Reply(tid2, "D", 2);
				}

			}
			//(void)Reply(tid, "W", 2);
		}
		
	}

}
void signup(){
	int pid = WhoIs("gameserver");
	char msg[25] = "signup";
	msg[6] = 0;
	Send(pid, msg, 7, msg, 25);
}
void quit(){
	int pid = WhoIs("gameserver");
	char msg[25] = "quit";
	msg[4] = 0;
	Send(pid, msg, 6, msg, 25);
}
char play(char* move){
	int pid = WhoIs("gameserver");
	char msg[25];
	Send(pid, move, 9, msg, 25);
	char retchar = msg[0];
	//strflush(msg, 25);
	return retchar;
}
char RPCShutdown(){
	int pid = WhoIs("gameserver");
	char msg[25] = "shutdown";
	msg[8] = 0;
	Send(pid, msg, 9, msg, 25);
	char retchar = msg[0];

	//strflush(msg, 25);
	return retchar;
}