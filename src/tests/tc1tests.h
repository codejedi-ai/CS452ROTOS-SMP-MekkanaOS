int tc1ExecuteCommands(char *command, char **num, int command_part_count);

int awaitTrigger(int track_server_tid, int s88_id, int sensor_no, int state);
void delay_until_stop(int delaytime, char train_id);
