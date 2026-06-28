#include "../rpi.h"
#include "track_server.h"
#include "marklin_worker.h"
#include "train_control.h"
#include "track_data_new.h"
#include "train_velocity.h"
#include "speed_measuring.h"
#include "goto.h"
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
// find_all_ahead_sensors returns a list of sensors that are ahead of the train
// they may be hit by the train
int find_all_ahead_sensors(const struct track_node *start,
                           const struct track_node *track,
                           const struct track_node **sensors, // it is an array
                           int *sensors_len,
                           int distance[TYPECOUNT][TRACK_MAX])
{
  struct track_node *queue[TRACK_MAX]; // this is the queue of the tracks

  int queue_size = 0;
  int queue_head = 0;
  int queue_tail = 0;
  *sensors_len = 0;
  int visited[TYPECOUNT][TRACK_MAX];

  for (int i = 0; i < TRACK_MAX; i++)
  {
    for (int j = 0; j < TYPECOUNT; j++)
    {
      visited[j][i] = 0;
      distance[j][i] = 9999999;
    }
  }
  distance[start->type][start->num] = 0;
  queue[queue_tail] = start; // start from one track node
  queue_tail++;
  queue_size++;
  // start from one track node
  while (queue_size > 0)
  {
    
    struct track_node *current = queue[queue_head];
    // // printf("current: %s\n", current->name);
    queue_head++;
    queue_size--;
    
    if (current->type == NODE_SENSOR)
    { 
      // DO NOT ENQUEUE THE STRAIGHT NODE IT IS THE END IT IS TO BE ADDED TO THE LIST
      sensors[*sensors_len] = current;
      *sensors_len = *sensors_len + 1;
      continue;
    }
    struct track_node *reverse = current->reverse;

    if (current->type == NODE_BRANCH)
    {
      // we have found a branch
      struct track_node *curved = current->edge[DIR_CURVED].dest;
      // we have not visited the curved branch
      if (distance[curved->type][curved->num] > distance[current->type][current->num] + current->edge[DIR_CURVED].dist)
      {
        // we have found a shorter path to the curved branch
        distance[curved->type][curved->num] = distance[current->type][current->num] + current->edge[DIR_CURVED].dist;
        queue[queue_tail] = curved;
        queue_tail++;
        queue_size++;
      }
    }
    if (current->type != NODE_EXIT)
    {
      struct track_node *straight = current->edge[DIR_STRAIGHT].dest;
      if (distance[straight->type][straight->num] > distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist)
      {
        // we have found a shorter path to the straight branch
        distance[straight->type][straight->num] = distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist;
        queue[queue_tail] = straight;
        queue_tail++;
        queue_size++;
      }
    }
  }
  return 0;
}
// path find returns the distance from the start to the end
int Pathfind(const struct track_node *start, const struct track_node *end,
             const struct track_node *track,
             struct track_node *prev[TYPECOUNT][TRACK_MAX],
             int distance[TYPECOUNT][TRACK_MAX])
{
  struct track_node *queue[TRACK_MAX]; // this is the queue of the tracks

  int queue_size = 0;
  int queue_head = 0;
  int queue_tail = 0;

  int visited[TYPECOUNT][TRACK_MAX];

  for (int i = 0; i < TRACK_MAX; i++)
  {
    for (int j = 0; j < TYPECOUNT; j++)
    {
      visited[j][i] = 0;
      distance[j][i] = 9999999;
    }
  }
  distance[start->type][start->num] = 0;
  queue[queue_tail] = start; // start from one track node
  queue_tail++;
  queue_size++;
  // start from one track node
  while (queue_size > 0)
  {
    struct track_node *current = queue[queue_head];
    queue_head++;
    queue_size--;
    //
    struct track_node *reverse = current->reverse;

    if (current->type == NODE_BRANCH)
    {
      // we have found a branch
      struct track_node *curved = current->edge[DIR_CURVED].dest;
      // we have not visited the curved branch
      if (distance[curved->type][curved->num] > distance[current->type][current->num] + current->edge[DIR_CURVED].dist)
      {
        // we have found a shorter path to the curved branch
        prev[curved->type][curved->num] = current;
        distance[curved->type][curved->num] = distance[current->type][current->num] + current->edge[DIR_CURVED].dist;
        queue[queue_tail] = curved;
        queue_tail++;
        queue_size++;
      }
    }
    if (current->type != NODE_EXIT)
    {
      struct track_node *straight = current->edge[DIR_STRAIGHT].dest;
      if (distance[straight->type][straight->num] > distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist)
      {
        // we have found a shorter path to the straight branch
        prev[straight->type][straight->num] = current;
        distance[straight->type][straight->num] = distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist;
        queue[queue_tail] = straight;
        queue_tail++;
        queue_size++;
      }
    }
    if (current->type == NODE_SENSOR)
    {
      if (distance[reverse->type][reverse->num] > distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist)
      {
        // we have found a shorter path to the straight branch
        prev[reverse->type][reverse->num] = current;
        distance[reverse->type][reverse->num] = distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist;
        queue[queue_tail] = reverse;
        queue_tail++;
        queue_size++;
      }
    }
  }
  return (distance[end->type][end->num]);
}

// Train functions Begin
int parse_path(struct track_node *track,
                    struct track_node *start,
                    struct track_node *end,
                    struct track_node **branches,
                    int *branches_len,
                    char *mode,
                    int *mode_len,
                    struct track_node **sen_list,
                    int *sen_list_sz,
                    struct track_node **revlist,
                    int *revlist_len,
                    int distance[TYPECOUNT][TRACK_MAX])
{
  struct track_node *previouse_node[TYPECOUNT][TRACK_MAX];
  Pathfind(start, end, track, previouse_node, distance);
  struct track_node *start_node = start;
  struct track_node *end_node = end;
  // get the path in two char arrays
  *branches_len = 0;
  *mode_len = 0;
  *sen_list_sz = 0;
  *revlist_len = 0;       /* was uninitialized; consumers read stack garbage */
  // compute the path
  while (end_node != start_node)
  {
    //
    struct track_node *prev_node = previouse_node[end_node->type][end_node->num];
    if (prev_node->type == NODE_BRANCH)
    {
      branches[*branches_len] = prev_node;
      *branches_len = *branches_len + 1;

      if (prev_node->edge[DIR_CURVED].dest == end_node)
      {
        // we have to switch the curved branch

        mode[*(mode_len)] = 'C';
        *(mode_len) = *(mode_len) + 1;
      }
      else
      {
        // we have to switch the straight branch

        mode[*(mode_len)] = 'S';
        *(mode_len) = *(mode_len) + 1;
      }
    }
    if (prev_node->type == NODE_SENSOR)
    {
      // we have to switch the straight branch
      sen_list[*(sen_list_sz)] = prev_node;
      *(sen_list_sz) = *(sen_list_sz) + 1;
    }
    // if it is reverse, we need to reverse the train
    if (0) //(prev_node->reverse == end_node)
    {

      revlist[(*revlist_len)] = prev_node;
      (*revlist_len)++;
    }

    end_node = previouse_node[end_node->type][end_node->num];
  }

  // print total distance

  return 0;
}

void solonoid_switches_helper(char track_id, char* start_str, char* end_str)
{


  struct track_node track[TRACK_MAX];
  struct track_node *revlist[TRACK_MAX];
  int revlist_len;
  if(track_id == 'a'){
    init_tracka(track);
  }else{
    init_trackb(track);
  }
  struct track_node *banches[TRACK_MAX];
  int branches_len;
  char mode[TRACK_MAX];

  int mode_len;
  struct track_node *sen_list[TRACK_MAX];
  int sen_list_sz;
  // printf("start_str = %s, end_str = %s\n", start_str, end_str);
  int distance[TYPECOUNT][TRACK_MAX];
  struct track_node *start = get_track_node_by_name(track, start_str);
  struct track_node *end = get_track_node_by_name(track, end_str);
  parse_path(track, start, end,
             banches, &branches_len,
             mode, &mode_len,
             sen_list, &sen_list_sz,
             revlist, &revlist_len,
             distance);
  // iterate through all the branch nodes
  // printf("branches_len = %d\n", branches_len);
  // solonoid command time get from the marklin_worker
  // set the cursor to it's approperate location

  uart_printf(CONSOLE, "\033[%d;%dH", GOTC_ROW, GOTC_COL);
  // print the nodes in string format
  uart_printf(CONSOLE, "start_str = %s, end_str = %s\n", start_str, end_str);
  int marklin_worker_tid = WhoIs("marklin_worker");
  for (int i = 0; i < branches_len; i++)
  {
   // solonoid(banches[i]->num, mode[i]);
   uart_printf(CONSOLE, "\033[%d;%dH", GOTC_ROW + i + 1, GOTC_COL);
   // print name of the branch node
    uart_printf(CONSOLE, "%s", banches[i]->name);
    uart_printf(CONSOLE, "Switch %d \r\n", banches[i]->num);
    uart_putc(CONSOLE, mode[i]);
    set_solonoid(marklin_worker_tid, banches[i]->num, mode[i]);
  }
}
int lookahead(char track_id, char* nodename, struct track_node *nodes[TRACK_MAX],int nodes_len,int distance[TYPECOUNT][TRACK_MAX])
{
  // init start node
  struct track_node track[TRACK_MAX];
  if(track_id == 'a'){
    init_tracka(track);
  }else{
    init_trackb(track);
  }
  // printf("init_tracka\n");
  struct track_node *start = get_track_node_by_name(track, "BR3");
  // init a list of nodes pointers
  
  
  // init a list of nodes pointers
  // find_all_ahead_sensors
  // printf("find_all_ahead_sensors\n");
  
  find_all_ahead_sensors(start, track, nodes, &nodes_len, distance);
  for (int i = 0; i < nodes_len; i++)
  {
    // printf("name:%s dist: %d\n", nodes[i]->name, distance[nodes[i]->type][nodes[i]->num]);
  }
  return 0;
}

int solonoid_switches_task(){
  char start_str[10];
  char end_str[10];
  char track_id;
  int tid;
  int track_server_tid = WhoIs("track_server");
  track_id = get_track_id(track_server_tid);
  Receive(&tid, start_str, 10);
  Reply(tid, start_str, 0);
  Receive(&tid, end_str, 10);
  Reply(tid, end_str, 0);

  solonoid_switches_helper(track_id, start_str, end_str);

  return 0;
}
// working on STOP AT
// in which the train need to stop at a location. This requires delicate tuning of delay_until_stop and sensor stop
void path_switch(char* start_str, char* end_str)
{
  int tid = Create(1, solonoid_switches_task);
  Send(tid, start_str, 10, NULL, 0);
  Send(tid, end_str, 10, NULL, 0);
}



/*
 * Walk forward from `start` toward `dest` along the path the train will take
 * given the current switch state snapshot, accumulating distance.
 * Returns 0 on success with *out_dist set; -1 if dest is unreachable.
 */
static int dist_along_route(struct track_node *start, struct track_node *dest,
                            char *sw_states, int *out_dist)
{
  struct track_node *cur = start;
  int dist = 0;
  int steps = 0;
  while (cur != dest && cur->type != NODE_EXIT && steps < TRACK_MAX * 4) {
    int idx = 0;
    if (cur->type == NODE_BRANCH) {
      idx = (sw_states[cur->num] == 'C') ? DIR_CURVED : DIR_STRAIGHT;
    }
    dist += cur->edge[idx].dist;
    cur = cur->edge[idx].dest;
    steps++;
  }
  if (cur == dest) { *out_dist = dist; return 0; }
  return -1;
}

/*
 * Parse "BR3", "BR3+50", "BR3-25" into name (in-place truncation) and offset.
 * offset_mm may be negative.
 */
static void parse_dest_with_offset(char *dest, int *offset_mm)
{
  *offset_mm = 0;
  char *p = dest;
  while (*p && *p != '+' && *p != '-') p++;
  if (*p == '\0') return;
  int sign = (*p == '-') ? -1 : 1;
  char *digits = p + 1;
  int v = 0;
  while (*digits >= '0' && *digits <= '9') {
    v = v * 10 + (*digits - '0');
    digits++;
  }
  *offset_mm = sign * v;
  *p = '\0';                     /* truncate at sign so name lookup works */
}

void stop_at_task()
{
  int tid;
  char trainid;
  Receive(&tid, &trainid, 1);
  Reply(tid, NULL, 0);
  char dest[16];
  for (int i = 0; i < 16; i++) dest[i] = 0;
  Receive(&tid, dest, 16);
  Reply(tid, NULL, 0);

  int offset_mm = 0;
  parse_dest_with_offset(dest, &offset_mm);

  int track_server_tid    = WhoIs("track_server");
  int clock_server_tid    = WhoIs("clock_server");
  int marklin_worker_tid  = WhoIs("marklin_worker");

  struct track_node track[TRACK_MAX];
  struct track_node *trackmap[20][20];
  if (get_track_id(track_server_tid) == 'a') init_tracka(track);
  else                                       init_trackb(track);
  get_track_node_map(track, trackmap);

  struct track_node *end_node = get_track_node_by_name(track, dest);
  if (end_node == 0) {
    uart_printf(CONSOLE, "\033[%d;%dHstop_at: dest %s not found\r\n", GOTC_ROW, GOTC_COL, dest);
    Exit();
    return;
  }

  int stopping_dist[TRAIN_MAX][SPEED_MAX];
  struct train_velocity vel_list[TRAIN_MAX][SPEED_MAX];
  init_stoppint_dist(stopping_dist);
  init_vel(vel_list);

  int speed = getspeed_train(track_server_tid, trainid);
  if (speed < 0 || speed >= SPEED_MAX) speed = 10;   /* sane default */
  int stop_dist = stopping_dist[(int)trainid][speed];
  struct train_velocity vel = vel_list[(int)trainid][speed];

  char sw_states[SWITCH_COUNT];
  get_switches(track_server_tid, sw_states, SWITCH_COUNT);

  uart_printf(CONSOLE, "\033[%d;%dHstop_at: train %d -> %s%+d  speed=%d stop_dist=%dmm\r\n",
              GOTC_ROW, GOTC_COL, trainid, dest, offset_mm, speed, stop_dist);

  int row = GOTC_ROW + 1;
  char ret[4];
  while (1) {
    uint32_t fire_time = (uint32_t)await_sensor(track_server_tid, ret);
    char s88_id      = ret[0];
    char sensor_no   = ret[1];
    char is_released = ret[2];
    if (is_released) continue;

    struct track_node *cur = trackmap[(int)s88_id][(int)sensor_no];
    if (cur == 0) continue;

    int remaining = 0;
    if (dist_along_route(cur, end_node, sw_states, &remaining) < 0) {
      uart_printf(CONSOLE, "\033[%d;%dHstop_at: %s unreachable from %s\r\n",
                  row++, GOTC_COL, end_node->name, cur->name);
      continue;
    }

    int effective = remaining + offset_mm;
    int slack_mm  = effective - stop_dist;
    uart_printf(CONSOLE, "\033[%d;%dH\033[Kstop_at: hit %s, %dmm to dest (eff %dmm, slack %dmm)",
                row, GOTC_COL, cur->name, remaining, effective, slack_mm);

    if (slack_mm <= 0) {
      /* Already inside braking radius -- brake immediately. */
      set_train_state(marklin_worker_tid, trainid, 0);
      uart_printf(CONSOLE, "\033[%d;%dH\033[Kstop_at: brake immediate at %s (overshoot ~%dmm)",
                  row + 1, GOTC_COL, cur->name, -slack_mm);
      break;
    }
    /* Will another sensor fire before we hit the brake point? If so, wait
       for it and re-evaluate -- the next reading lets us correct for any
       drift in the velocity model. Otherwise lock in a DelayUntil and brake. */
    int dist_to_next = 0, isexit = 0;
    next_type_node(sw_states, NODE_SENSOR, cur, &dist_to_next, &isexit);
    if (!isexit && dist_to_next < slack_mm) continue;

    int ticks_until_brake = compute_time(slack_mm, &vel);
    DelayUntil(clock_server_tid, (int)(fire_time + ticks_until_brake));
    set_train_state(marklin_worker_tid, trainid, 0);
    uart_printf(CONSOLE, "\033[%d;%dH\033[Kstop_at: brake at %s + %dt (slack %dmm)",
                row + 1, GOTC_COL, cur->name, ticks_until_brake, slack_mm);
    break;
  }
  Exit();
}

void stop_at(int trainid, char *dest)
{
  int tid = Create(1, stop_at_task);
  char id = (char)trainid;
  Send(tid, &id, 1, NULL, 0);
  char buf[16];
  int i = 0;
  for (; i < 15 && dest[i]; i++) buf[i] = dest[i];
  for (; i < 16; i++) buf[i] = 0;
  Send(tid, buf, 16, NULL, 0);
}