#include "sensor_to_node.h"


// the sensor_id is ALWAYS 1 indexed
// track_node -> int int
void nodename_to_ss88id_sensor_id(char *name, char *s88_id, char *sensor_id){
  // get the s88_id the first character
  *s88_id = name[0] - 'A';
  // get the sensor_id the number that is after the first character
  *sensor_id = atoi_64(&name[1]) - 1;
}

void  get_track_node_map(struct track_node *track, struct track_node *trackmap[20][20]){
	// iterate through all the SENSOR_NODEs and find the one that matches the s88_id and sensor_id
	// the s88_id is the s88 that is triggered in alphabet A,B,C,D
	// the sensor_id is the sensor that is triggered in the s88 from 1 - 16
	// the naming convention is A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, D1, D2, D3, D4.....
	for(int i = 0; i < TRACK_MAX; i++){
		if(track[i].type == NODE_SENSOR){
			// get name
			char *name = track[i].name;
			char s88_id, sensor_id;
			nodename_to_ss88id_sensor_id(name, &s88_id, &sensor_id);
			//uart_printf(CONSOLE, "name:%s s88_id = %d, sensor_id = %d\r\n",name , s88_id, sensor_id);
			trackmap[s88_id][sensor_id] = &track[i];
		}
	}
}
// int, int -> track_node
struct track_node* get_track_node(struct track_node *trackmap[20][20], int s88_id, int sensor_id){
	// iterate through all the SENSOR_NODEs and find the one that matches the s88_id and sensor_id
	// the s88_id is the s88 that is triggered in alphabet A,B,C,D
	// the sensor_id is the sensor that is triggered in the s88 from 1 - 16
	// the naming convention is A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, D1, D2, D3, D4.....
	sensor_id--;
	//uart_printf(CONSOLE, "name:%s s88_id = %d, sensor_id = %d\r\n",trackmap[s88_id][sensor_id]->name, s88_id, sensor_id);
	return trackmap[s88_id][sensor_id];
}