#include "../rpi.h"
#include "../clockserver.h"
#include "track_data_new.h"
#include "marklin_worker.h"
#include "train_control.h"
// Delay until interrupt for stop
void path_finder_user_task(){


  // test_train_functions();
  struct track_node tracka[TRACK_MAX];
  struct track_node *revlist[TRACK_MAX];
  int revlist_len;
  init_tracka(tracka);
  struct track_node *banches[TRACK_MAX];
  int branches_len;
  char mode[TRACK_MAX];
  
  int mode_len;
  struct track_node *start = &tracka[9];
  struct track_node *end = &tracka[5];
  test_pathfinder(tracka, start, end, banches, &branches_len, mode, &mode_len, revlist, &revlist_len);
  // void set_solonoid(int marklin_worker_tid, uint8_t sol_id, char state);
  int marklin_worker_tid = WhoIs("marklin_worker");
  // set the solonoids
  for (int i = 0; i < branches_len; i++)
  {
    // printf("%s ", banches[i]->name);
    // printf("%c ", mode[i]);
    // printf("\n");
    set_solonoid(marklin_worker_tid, banches[i]->num, mode[i]);
  }
  /*
  printf("revlist_len = %d\n", revlist_len);
  for (int i = 0; i < revlist_len; i++)
  {
    printf("%s ", revlist[i]->name);
    printf("\n");
  }
*/

    
}
void delay_until_stop_task(){
    int tid;
    int delay_since_interrupt = 0;
    Receive(&tid, &delay_since_interrupt, 4);
    Reply(tid, &tid, 0);
    char train_id = 0x01;
    Receive(&tid, &train_id, 1);
    Reply(tid, &train_id, 0);
/*
    int clock_server_tid = WhoIs("clock_server");
    int track_server_tid = WhoIs("track_server");
    int marklin_worker_tid = WhoIs("marklin_worker");

    uint64_t delay_interrupt = await_sensor(track_server_tid);
    uint32_t time_interrupt = delay_interrupt & 0xFFFFFFFF;
    char *sw_states = &delay_interrupt;

    DelayUntil(clock_server_tid, time_interrupt + delay_since_interrupt);
    set_train_state(marklin_worker_tid, train_id, 0);
    */
}
void delay_until_stop(int delaytime, char train_id){
    int tid = Create(1, &delay_until_stop_task);
    Send(tid, &delaytime, 4, NULL, 0);
    Send(tid, &train_id, 1, NULL, 0);
}

void execute_reverse_command(){  // Binary: 00000001)
// void set_train_state(int marklin_worker_tid, uint8_t train_ind, char speed);
    
    char train_id = 0x01;
    char speed = 0x00;
    int tid;
    Receive(&tid, &train_id, 1);
    Reply(tid, NULL, 0);
    Receive(&tid, &speed, 1);
    Reply(tid, NULL, 0);
    /*
      command_wrapper(0, id);
      command_wrapper(15, id);
      command_wrapper(speed, id);
*/
    int marklin_worker_tid = WhoIs("marklin_worker");
    int clock_server_tid = WhoIs("clock_server");
    //print on the column 200
    int rev_delay = 100;

    uart_printf(CONSOLE, "\033[%d;%dH", 9, 200);
    uart_printf(CONSOLE, "execute_reverse_command\r\n");
    uart_printf(CONSOLE, "\033[%d;%dH", 10, 200);
    // clear the line
    uart_printf(CONSOLE, "\033[K");
    uart_printf(CONSOLE, "\033[%d;%dH", 11, 200);
    // clear the line
    uart_printf(CONSOLE, "\033[K");
    uart_printf(CONSOLE, "\033[%d;%dH", 12, 200);
    // clear the line
    uart_printf(CONSOLE, "\033[K");
    uart_printf(CONSOLE, "\033[%d;%dH", 13, 200);


    uart_printf(CONSOLE, "\033[%d;%dH", 10, 200);
    uart_printf(CONSOLE, "Stopping train ....%d\r\n", train_id);
    set_train_state(marklin_worker_tid, train_id, 0);
    Delay(clock_server_tid, rev_delay);
    uart_printf(CONSOLE, "\033[%d;%dH", 11, 200);
    uart_printf(CONSOLE, "Reversing train ....%d\r\n", train_id);
    set_train_state(marklin_worker_tid, train_id, 15);
    Delay(clock_server_tid, rev_delay);
    uart_printf(CONSOLE, "\033[%d;%dH", 12, 200);
    uart_printf(CONSOLE, "Setting speed to %d ....%d\r\n", speed, train_id);
    set_train_state(marklin_worker_tid, train_id, speed);
    uart_printf(CONSOLE, "\033[%d;%dH", 13, 200);
    uart_printf(CONSOLE, "Reversed train %d\r\n", train_id);
    
    Exit();
}
void reverse(char train_id, char speed){
    int tid = Create(1, execute_reverse_command);
    Send(tid, &train_id, 1, NULL, 0);
    Send(tid, &speed, 1, NULL, 0);
}

// this is the polling loop for each individual train task
