```c
track[0].name = "A1";
track[0].type = NODE_SENSOR;
track[0].num = 0;
track[0].reverse = &track[1];
track[0].edge[DIR_AHEAD].reverse = &track[102].edge[DIR_STRAIGHT];
track[0].edge[DIR_AHEAD].src = &track[0];
track[0].edge[DIR_AHEAD].dest = &track[103];
track[0].edge[DIR_AHEAD].dist = 231;
track[1].name = "A2";
track[1].type = NODE_SENSOR;
track[1].num = 1;
track[1].reverse = &track[0];
track[1].edge[DIR_AHEAD].reverse = &track[132].edge[DIR_AHEAD];
track[1].edge[DIR_AHEAD].src = &track[1];
track[1].edge[DIR_AHEAD].dest = &track[133];
track[1].edge[DIR_AHEAD].dist = 504;
```

