struct game {
	int tid1;
	int tid2;
	char tid1_move[10];
	char tid2_move[10];
	int tid1_score;
	int tid2_score;
};

uint8_t full_game(struct game *cur_game);
uint8_t full_play(struct game *cur_game);
int check_game(struct game *cur_game);
int ingame(int tid, struct game *games);

void signup();
void gameserver();
char play(char* move);
void quit();
char RPCShutdown();