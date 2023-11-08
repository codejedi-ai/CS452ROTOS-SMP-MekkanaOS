#include "../rpi.h"
#include "track_server.h"
// the sensor_id is ALWAYS 1 indexed
// track_node -> int int
void nodename_to_ss88id_sensor_id(char *name, char *s88_id, char *sensor_id);

void get_track_node_map(struct track_node *track, struct track_node *trackmap[20][20]);
// int, int -> track_node
struct track_node* get_track_node(struct track_node *trackmap[20][20], int s88_id, int sensor_id);