#include "track_data_new.h"
#define TYPECOUNT 10
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
// int Pathfind (int start, int end, const struct track_node *track, struct track_node *prev[TYPECOUNT][TRACK_MAX]);
int Pathfind (const struct track_node *start, const struct track_node *end, const struct track_node *track, struct track_node *prev[TYPECOUNT][TRACK_MAX]);
// Train functions Begin
int test_pathfinder(struct track_node *track,
                    struct track_node *start,
                    struct track_node *end,
                    struct track_node **branches,
                    int *branches_len,
                    char *mode,
                    int *mode_len,
                    struct track_node **revlist,
                    int *revlist_len);