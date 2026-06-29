#include "tc1tests.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstring.h"
#include "systimer.h"
#include "clockserver.h"
#include "marklin_worker.h"
#include "train_control.h"
#include "track_data_new.h"
#include "goto.h"
#include "io_api.h"
#define delay 0
#define DJIKSTRAS_ROW 1
#define DJIKSTRAS_COL 1
// Train functions Begin
/*

code functions on
64 all off
65 #1
66 #2
67 #1,#2
68 #3
69 #3,#1
70 #3,#2
71 #3,#2,#1
72 #4
73 #4,#1
74 #4,#2
75 #4,#2,#1
76 #4,#3
77 #4,#3,#1
78 #4,#3,#2
79 #4,#3,#2,#1
*/


/*
Track node.c
typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} node_type;
*/
#define PRINT_ROW 1
#define PRINT_COL 1


/* ------------------------------------------------------------------------- *
 *  Sensor-action runner (observer pattern client)
 *
 *  Spawned by the `on` command. Receives a 100-byte message containing:
 *    msg[0]   - s88 id (already normalized to 0..4 by the shell handler)
 *    msg[1]   - sensor number (1..16)
 *    msg[2..] - NUL-terminated action string (e.g. "tr 24 8")
 *
 *  Lifecycle: Receive args, reply immediately, block on awaitTrigger() until
 *  the sensor fires, then re-parse and dispatch the action through
 *  tc1ExecuteCommands. One trigger -> one action -> Exit. Re-arm by issuing
 *  the `on` command again.
 * ------------------------------------------------------------------------- */
void sensor_action_runner(void)
{
  int sender_tid;
  char msg[100];
  int recvlen = Receive(&sender_tid, msg, sizeof(msg));
  (void)recvlen;
  char s88     = msg[0];
  int  sensor  = (unsigned char)msg[1];
  /* Copy action out before replying (caller's buffer dies the moment we Reply). */
  char action[80];
  int j = 0;
  for (int i = 2; i < (int)sizeof(msg) && msg[i] && j < (int)sizeof(action) - 1; i++)
    action[j++] = msg[i];
  action[j] = 0;
  int ack = 0;
  Reply(sender_tid, (char*)&ack, 0);

  int track_srv_tid = WhoIs("track_server");
  if (track_srv_tid <= 0) Exit();
  awaitTrigger(track_srv_tid, s88, sensor, 0);

  uart_printf(CONSOLE, "\r\n[on s88=%d sensor=%d fired] running: %s\r\n",
              s88, sensor, action);
  char *num2[20];
  int n2 = parse_char_arr(action, num2, 20);
  tc1ExecuteCommands(num2[0], num2, n2);
  Exit();
}

int tc1ExecuteCommands(char *command, char **num, int command_part_count){
  // if command is in the form of tc trainid speed

  int marklin_worker_tid = WhoIs("marklin_worker");
  if (strcmp_ret(command, "tr")){
    int trainid = atoi_64(num[1]);
    int speed = atoi_64(num[2]);
    if (trainid < 1 || trainid > 80 || speed < 0 || speed > 14){
      // set the position of cursor to DJIKSTRAS_ROW + offset and DJIKSTRAS_COL
      
      uart_printf(CONSOLE, "Invalid trainid or speed\r\n");
      return 1;
    }
    // set the speed of the train
    set_train_state(marklin_worker_tid, trainid, speed);
    return 0;
  }
  // if the command is in the form of sw switchid direction
  if (strcmp_ret(command, "sw")){
    int switchid = atoi_64(num[1]);
    int direction = num[2][0];
    if (switchid < 1 || switchid > 18 || (direction != 'C' && direction != 'S')){
      uart_printf(CONSOLE, "Invalid switchid or direction\r\n");
      return 1;
    }
    // set the direction of the switch
    set_solonoid(marklin_worker_tid, switchid, direction);
    return 0;
  }
  // if the command is in the form of rv trainid
  if (strcmp_ret(command, "rv")){
    int trainid = atoi_64(num[1]);
    int speed = atoi_64(num[2]);
    if (trainid < 1 || trainid > 80){
      uart_printf(CONSOLE, "Invalid trainid\r\n");
      return 1;
    }
    uart_printf(CONSOLE, "Reversing train ....%d\r\n", trainid);
    reverse(trainid, speed);
    
    return 0;
  }
  // testing stopping distance. If this is executed the marklin would make a task that would execute the stop commmand when the train has hit a sensor node
  if (strcmp_ret(command, "tcstd")){
    int trainid = atoi_64(num[1]);
    int speed = atoi_64(num[2]);
    if (trainid < 1 || trainid > 80){
      uart_printf(CONSOLE, "Invalid trainid\r\n");
      return 1;
    }
    uart_printf(CONSOLE, "NOT IMPLEMENTED: Testing stopping distance for train %d\r\n", trainid);
    return 0;
  }
  // void delay_until_stop(int delaytime, char train_id)
  if(strcmp_ret(command, "delaystop")){
    int trainid = atoi_64(num[1]);
    int delaytime = atoi_64(num[2]);
    if (trainid < 1 || trainid > 80){
      uart_printf(CONSOLE, "Invalid trainid\r\n");
      return 1;
    }
    uart_printf(CONSOLE, "Delaying train %d until it stops\r\n", trainid);
    delay_until_stop(delaytime, trainid);
    return 0;
  }
  // add a stop at a sensor command
  if (strcmp_ret(command, "interruptstop")){
    int trainid = atoi_64(num[1]);
    if (trainid < 1 || trainid > 80){
      uart_printf(CONSOLE, "Invalid trainid\r\n");
      return 1;
    }
    uart_printf(CONSOLE, "Stopping train %d\r\n", trainid);
    sensor_stop(trainid);
    return 0;
  }
  // add one for sensor_delay_stop
  if (strcmp_ret(command, "sensordelaystop")){
    int trainid = atoi_64(num[1]);
    int delaytime = atoi_64(num[3]);
    if (trainid < 1 || trainid > 80){
      uart_printf(CONSOLE, "Invalid trainid\r\n");
      return 1;
    }
    uart_printf(CONSOLE, "Stopping train %d\r\n", trainid);
    sensor_delay_stop(trainid, delaytime);
    return 0;
  }
  // void path_switch(char* start_str, char* end_str);
  if (strcmp_ret(command, "ps")){
    char* start_str = num[1];
    char* end_str = num[2];
    path_switch(start_str, end_str);
    return 0;
  }
  // stop_at void stop_at(int trainid, char *dest)
  if (strcmp_ret(command, "stopa")){
    int trainid = atoi_64(num[1]);
    char* dest = num[2];
    stop_at(trainid, dest);
    return 0;
  }
  /* on <s88 A-E or 0-4> <sensor 1-16> <command args...>
     Spawns a one-shot observer task. When the named sensor fires, it
     dispatches the rest of the line back through tc1ExecuteCommands.
     Example: `on A 5 tr 24 8` -> when sensor A5 trips, set train 24 speed 8.
     Multiple observers on the same sensor are allowed; each is independent. */
  if (strcmp_ret(command, "on")){
    if (command_part_count < 4) {
      uart_printf(CONSOLE, "Usage: on <s88 A-E> <sensor 1-16> <command args...>\r\n");
      return 1;
    }
    char raw_s88 = num[1][0];
    char s88;
    if (raw_s88 >= 'A' && raw_s88 <= 'E')      s88 = raw_s88 - 'A';
    else if (raw_s88 >= 'a' && raw_s88 <= 'e') s88 = raw_s88 - 'a';
    else                                       s88 = (char)atoi_64(num[1]);
    int sensor = atoi_64(num[2]);
    if (s88 < 0 || s88 > 4 || sensor < 1 || sensor > 16) {
      uart_printf(CONSOLE, "Invalid s88 (A-E or 0-4) or sensor (1-16)\r\n");
      return 1;
    }

    /* Reassemble action string from num[3..end] with spaces between tokens. */
    char action[80];
    int p = 0;
    for (int i = 3; i < command_part_count; i++) {
      for (int k = 0; num[i][k] && p < (int)sizeof(action) - 1; k++)
        action[p++] = num[i][k];
      if (i < command_part_count - 1 && p < (int)sizeof(action) - 1)
        action[p++] = ' ';
    }
    action[p] = 0;

    /* Pack & hand off to the runner. Action travels in msg[2..]. */
    int rid = Create(3, sensor_action_runner);
    if (rid <= 0) {
      uart_printf(CONSOLE, "Create(sensor_action_runner) failed\r\n");
      return 1;
    }
    char buf[100];
    buf[0] = s88;
    buf[1] = (char)sensor;
    int q = 2;
    for (int i = 0; action[i] && q < (int)sizeof(buf) - 1; i++) buf[q++] = action[i];
    buf[q] = 0;
    char rep = 0;
    Send(rid, buf, sizeof(buf), &rep, 0);
    uart_printf(CONSOLE, "Watching s88=%d sensor=%d, will run: %s\r\n",
                s88, sensor, action);
    return 0;
  }
  return 2;
}