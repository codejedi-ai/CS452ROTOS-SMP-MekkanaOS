
#include "track_path_finder.h"
#include <stdint.h>
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
int Pathfind (const struct track_node *start, const struct track_node *end, const struct track_node *track, struct track_node *prev[TYPECOUNT][TRACK_MAX]){
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
        // printf("current->type = %d, current = %s\n",current->type, current->name);
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
        if(0){//if (current->type == NODE_SENSOR){ // disable reverse for now
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
    // printf("^ next_node->type = %d, next_node = %s\n",end_node->type, end_node->name);
    struct track_node *prev_node = previouse_node[end_node->type][end_node->num];
    if (prev_node->type == NODE_BRANCH)
    {
      branches[*branches_len] = prev_node;
      *branches_len = *branches_len + 1;
      // printf("%s", prev_node->name);
      // printf("|%d|", prev_node->num);

      if (prev_node->edge[DIR_CURVED].dest == end_node)
      {
        // we have to switch the curved branch
        // printf(":C\n");
        mode[*(mode_len)++] = 'C';
      }
      else
      {
        // we have to switch the straight branch
        // printf(":S\n");
        mode[*(mode_len)++] = 'S';
      }
    }
    // if it is reverse, we need to reverse the train
    if (prev_node->reverse == end_node)
    {

      // printf("%s:R\n", prev_node->name);
      revlist[(*revlist_len)++] = prev_node;
    }

    end_node = previouse_node[end_node->type][end_node->num];
  }
  // printf("start_node->type = %d, start_node = %s\n", end_node->type, end_node->name);
  // print total distance
  
  return 0;

}



/*
#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
  track_edge *reverse;
  track_node *src, *dest;
  int dist;              in millimetres 
};

struct track_node {
  const char *name;
  node_type type;
  int num;               sensor or switch number 
  track_node *reverse;   same location, but opposite direction 
  track_edge edge[2];
};
*/
// this function returns a set of solonoids in place of the switches
// it returns two lists, one list consist of the solonoids need to be used
// the other list consist of the states of the solonoids that are to be used are in

/*
track[0].name = "A1";
track[0].type = NODE_SENSOR;
track[0].num = 0;
track[0].reverse = &track[1];
track[0].edge[DIR_AHEAD].reverse = &track[102].edge[DIR_STRAIGHT];
track[0].edge[DIR_AHEAD].src = &track[0];
track[0].edge[DIR_AHEAD].dest = &track[103];
track[0].edge[DIR_AHEAD].dist = 231;
*/
