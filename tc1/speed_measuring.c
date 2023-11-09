#include "train.h"
#include "../rpi.h"
#include "../util.h"
#include "../nameserver.h"
#include "../ioserver.h"
#include "speed_measuring.h"
#include "../custstr.h"
#define SENSOR 0
#define TRAIN 1
#define SWITCH 2

struct track_node *next_type_node(char *sw_states, int type, struct track_node *start_node, int *dist, int *isexit)
{
  // the current node is a sensor node then the for loop would not run
  struct track_node *current_node = start_node;
  int offset = 0;
  while ((current_node == start_node) || (current_node->type != type && current_node->type != NODE_EXIT))
  {
    offset++;
    // if the current position is a switch then we need to consult the switch table. Which is managed by the switch worker
    // uart_printf(CONSOLE, "type: %d next_sensor_node: current_node->type = %d, current_node = %s\n",type, current_node->type, current_node->name);
    // uart_getc(CONSOLE);
    // int buf;
    // scanf("%d", &buf);
    if (current_node->type == NODE_BRANCH)
    {
      char sw_state = sw_states[current_node->num];
      // this is a switch
      // check the switch table
      // if the switch is set to straight then we need to return the straight node
      if (sw_state == 'S')
      {

        *dist += current_node->edge[0].dist;
        current_node = current_node->edge[0].dest;
        // print the distance current_node->edge[0].dist
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
      }
      // if the switch is set to curved then we need to return the curved node
      if (sw_state == 'C')
      {

        *dist += current_node->edge[1].dist;
        current_node = current_node->edge[1].dest;
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[1].dist = %d\n", current_node->edge[1].dist);
      }
    }
    else
    {

      *dist += current_node->edge[0].dist;
      current_node = current_node->edge[0].dest;
      // print the distance current_node->edge[0].dist
      // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
      // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
    }
  }
  if (current_node->type == NODE_EXIT)
  {
    *isexit = 1;
  }
  // this node can be exit node or not
  return current_node;
}

void get_track_node_map(struct track_node *track, struct track_node *trackmap[20][20])
{
  // iterate through all the SENSOR_NODEs and find the one that matches the s88_id and sensor_no
  // the s88_id is the s88 that is triggered in alphabet A,B,C,D
  // the sensor_no is the sensor that is triggered in the s88 from 1 - 16
  // the naming convention is A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, D1, D2, D3, D4.....
  for (int i = 0; i < TRACK_MAX; i++)
  {
    if (track[i].type == NODE_SENSOR)
    {
      // get name
      char *name = track[i].name;
      // get the s88_id the first character
      char s88_id = name[0] - 'A';
      // get the sensor_no the number that is after the first character
      int64_t sensor_no = atoi_64(&name[1]) - 1;
      // uart_printf(CONSOLE, "name:%s s88_id = %d, sensor_no = %d\r\n",name , s88_id, sensor_no);
      trackmap[s88_id][sensor_no] = &track[i];
    }
  }
}
// string -> node reverse name search
struct track_node* get_track_node_by_name(struct track_node *track, char* name){
  for(int i = 0; i < TRACK_MAX; i++){
    if(strcmp_ret(track[i].name, name)){
      return &track[i];
    }
  }
  return 0;
}
int dist_to_node(char *sw_states, struct track_node *start_node, struct track_node *end_node)
{
  int dist = 0;
  // the current node is a sensor node then the for loop would not run
  struct track_node *current_node = start_node;
  while (current_node != end_node)
  {
    // if the current position is a switch then we need to consult the switch table. Which is managed by the switch worker
    // uart_printf(CONSOLE, "dist_to_node: current_node->type = %d, current_node = %s\n",current_node->type, current_node->name);
    if (current_node->type == NODE_BRANCH)
    {
      char sw_state = sw_states[current_node->num];
      // this is a switch
      // check the switch table
      // if the switch is set to straight then we need to return the straight node
      if (sw_state == 'S')
      {

        dist += current_node->edge[0].dist;
        current_node = current_node->edge[0].dest;
        // print the distance current_node->edge[0].dist
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
      }
      // if the switch is set to curved then we need to return the curved node
      if (sw_state == 'C')
      {

        dist += current_node->edge[1].dist;
        current_node = current_node->edge[1].dest;
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[1].dist = %d\n", current_node->edge[1].dist);
      }
    }
    else
    {

      dist += current_node->edge[0].dist;
      current_node = current_node->edge[0].dest;
      // print the distance current_node->edge[0].dist
      // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
      // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
    }
  }
  return dist;
}

// PRINT FUNCTIONS BEGIN
void new_table_row(char prev_changed_s88, char prev_changed_sensor,
                   char s88_id, char sensor_no,
                   int dist, int time_diff, int *offset)
{
  uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW + *offset, TABLECOL);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, prev_changed_s88 + 'A');
  uart_printf(CONSOLE, "%d", prev_changed_sensor);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, s88_id + 'A');
  uart_printf(CONSOLE, "%d", sensor_no);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", dist);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", time_diff);
  uart_putc(CONSOLE, '|');
  uart_puts(CONSOLE, "\r\n");
  *offset += 1;
}
void print_table_headers()
{
  /*

  | Previous  Sensor | current sensor | Distance traveled | Time Elapsed (Ticks) |
| --- | --- | --- | --- |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
*/

  uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW - 2, TABLECOL);
  uart_printf(CONSOLE, "| Previous  Sensor | current sensor | Distance traveled | Time Elapsed (Ticks) |\r\n");
  uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW - 1, TABLECOL);
  uart_printf(CONSOLE, "| --- | --- | --- | --- |\r\n");
  uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW, 1);
  /*
  #define SENSORROW 50
#define SENSORCOL 0
  */
  uart_printf(CONSOLE, "\033[%u;%uH", SENSORROW - 2, SENSORCOL);
  uart_printf(CONSOLE, "Sensor Table\r\n");
  uart_printf(CONSOLE, "\033[%u;%uH", SENSORROW - 1, SENSORCOL);
  uart_printf(CONSOLE, "Recent Triggers\r\n");
  uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW - 3, 1);
  for (int i = 0; i < 200; i++)
  {
    uart_putc(CONSOLE, '-');
  }
}
// this would wake up every time a sensor is triggered or a sensor is released or a switch is changed
void speed_gather()
{
  RegisterAs("speed_gather");

  int tid_buf = 0, buf = 0;

  struct train train_list[TRAIN_MAX];
  int train_speed[TRAIN_MAX];
  int sensor_pushed[TRAIN_MAX];
  struct track_node track[TRACK_MAX];
  init_tracka(track);
  struct track_node *trackmap[20][20];
  get_track_node_map(track, trackmap);

  char sw_states[SWITCH_COUNT];
  // define the previouse changed sensor, switch, state
  uint32_t prev_time = Time(WhoIs("clock_server"));
  struct train cur_train;
  int offset = 0;
  // set the pointer to the correct location for table qith TABLEROW and TABLECOL
  int first_time = 1;
  uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW + offset, TABLECOL);
  uart_printf(CONSOLE, "measure_Server activa\r\n");
  print_table_headers();
  struct track_node *pred;
  while (1)
  {
    // this is the task that is going to wake one the node that is about to stop. 
    // it requires an wait for sensor, await for the sensor until the one waiting is reached

    char ret[4];
    // there is a sensor that is recentlly triggered
    // await for the interrupt from the task server
    uint32_t time = await_sensor(WhoIs("track_server"), ret);

    char s88_id = ret[0];
    char sensor_no = ret[1];
    char is_released = ret[2];
    // set the cursor location
        // to be fired when await sensor is triggered

    if(is_released){
      continue;
      // set the cursor location
    }
    
    uart_printf(CONSOLE, "\033[%u;%uH", TABLEROW + offset, TABLECOL);
    // trackmap[s88_id][sensor_no-1] is how you denote s88_id and sensor_no -> track_node
    uart_printf(CONSOLE, "cur_node: %d, %d, cur_node: %s|", s88_id, sensor_no, trackmap[s88_id][sensor_no-1]->name);
    int dist = 0, is_exit = 0;
    // generate the next pred node
    pred = next_type_node(sw_states, NODE_SENSOR, trackmap[s88_id][sensor_no-1], &dist, &is_exit);
    uart_printf(CONSOLE, "pred_node: %s, dist: %d, is_exit: %d\r\n", pred->name, dist, is_exit);
    offset++;
  }

  Exit();
}
