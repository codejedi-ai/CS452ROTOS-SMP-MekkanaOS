#include "tc1tests.h"
#include "../rpi.h"
#include "../syscall.h"
#include "../nameserver.h"
#include "../custstr.h"
#include "../systimer.h"
#include "../clockserver.h"
#include "../tc1/marklin_worker.h"
#include "../tc1/train_control.h"
#include "../tc1/track_data_new.h"
#include "../ioserver.h"
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
int Pathfind (const struct track_node *start, const struct track_node *end, const struct track_node *track, struct track_node *prev[TYPECOUNT][TRACK_MAX]){
  // set cursor oto the correct location
  uart_printf(CONSOLE, "\033[%d;%dH", DJIKSTRAS_ROW, DJIKSTRAS_COL);
    struct track_node *queue[TRACK_MAX]; // this is the queue of the tracks
    
    int queue_size = 0;
    int queue_head = 0;
    int queue_tail = 0;

    int visited[TYPECOUNT][TRACK_MAX];
    int distance[TYPECOUNT][TRACK_MAX];
    for (int i = 0; i < TRACK_MAX; i++){
        for (int j = 0; j < TYPECOUNT; j++){
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
        
        if (current->type == NODE_BRANCH){
            // we have found a branch
            struct track_node *curved = current->edge[DIR_CURVED].dest;
                // we have not visited the curved branch
            if  (distance[curved->type][curved->num] > distance[current->type][current->num] + current->edge[DIR_CURVED].dist){
                // we have found a shorter path to the curved branch
                prev[curved->type][curved->num] = current;
                distance[curved->type][curved->num] = distance[current->type][current->num] + current->edge[DIR_CURVED].dist;
                queue[queue_tail] = curved;
                queue_tail++;
                queue_size++;
            }
        }
        if (current->type != NODE_EXIT){
          struct track_node *straight = current->edge[DIR_STRAIGHT].dest;
          if(distance[straight->type][straight->num] > distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist){
            // we have found a shorter path to the straight branch
            prev[straight->type][straight->num] = current;
            distance[straight->type][straight->num] = distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist;
            queue[queue_tail] = straight;
            queue_tail++;
            queue_size++;
          }
        }
        if (current->type == NODE_SENSOR){
          if(distance[reverse->type][reverse->num] > distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist){
            // we have found a shorter path to the straight branch
            prev[reverse->type][reverse->num] = current;
            distance[reverse->type][reverse->num] = distance[current->type][current->num] + current->edge[DIR_STRAIGHT].dist;
            queue[queue_tail] = reverse;
            queue_tail++;
            queue_size++;
          }
        }
    }   
  return( distance[end->type][end->num]);
}

// Train functions Begin
int test_pathfinder(struct track_node *track,
                    struct track_node *start,
                    struct track_node *end,
                    struct track_node **branches,
                    int *branches_len,
                    char *mode,
                    int *mode_len,
                    struct track_node **revlist,
                    int *revlist_len)
{
  struct track_node *previouse_node[TYPECOUNT][TRACK_MAX];
  Pathfind(start, end, track, previouse_node);
  struct track_node *start_node = start;
  struct track_node *end_node = end;
  // get the path in two char arrays
  *branches_len = 0;
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
    // if it is reverse, we need to reverse the train
    if (0)//(prev_node->reverse == end_node)
    {

      
      revlist[(*revlist_len)] = prev_node;
      (*revlist_len)++;
    }

    end_node = previouse_node[end_node->type][end_node->num];
  }
  
  // print total distance
  
  return 0;

}

struct track_node* get_track_node_by_name(struct track_node *track, char* name){
  for(int i = 0; i < TRACK_MAX; i++){
    if(strcmp_ret(track[i].name, name)){
      return &track[i];
    }
  }
  return 0;
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
  }
  if(strcmp_ret(command, "pathfinder")){
    char *start_node_name = num[1];
    char *end_node_name = num[2];
    // find the start node pointer address
    struct track_node tracka[TRACK_MAX];
    init_tracka(tracka);
    struct track_node *start_node = get_track_node_by_name(tracka, start_node_name);
    struct track_node *end_node = get_track_node_by_name(tracka, end_node_name);
    if(start_node == 0 || end_node == 0){
      uart_printf(CONSOLE, "\033[%d;%dH", DJIKSTRAS_ROW, DJIKSTRAS_COL);
      uart_printf(CONSOLE, "Invalid start %s or end node %s\r\n", start_node_name, end_node_name);
      return 1;
    }
    int revlist_len;
    struct track_node *banches[TRACK_MAX];
    int branches_len;
    char mode[TRACK_MAX];
    int mode_len;
    struct track_node *revlist[TRACK_MAX];

    test_pathfinder(tracka, start_node, end_node, banches, &branches_len, mode, &mode_len, revlist, &revlist_len);
    uart_printf(CONSOLE, "\033[%d;%dH", DJIKSTRAS_ROW, DJIKSTRAS_COL);
    uart_printf(CONSOLE,"branches_len = %d\n", branches_len);
    int offset = 1;
    for (int i = 0; i < branches_len; i++)
    {
      uart_printf(CONSOLE, "\033[%d;%dH", DJIKSTRAS_ROW + offset, DJIKSTRAS_COL);
      uart_printf(CONSOLE,"%s ", banches[i]->name);
      uart_printf(CONSOLE,"%c ", mode[i]);
      uart_printf(CONSOLE,"\n");
      offset+=1;
    }
    /*
    // Train functions Begin
    int test_pathfinder(struct track_node *track,
                        struct track_node *start,
                        struct track_node *end,
                        struct track_node **branches,
                        int *branches_len,
                        char *mode,
                        int *mode_len,
                        struct track_node **revlist,
                        int *revlist_len)
    */
  }
  return 2;
}