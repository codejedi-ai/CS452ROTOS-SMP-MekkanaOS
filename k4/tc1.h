#include "rpi.h"
#include "util.h"
#include "ioserver.h"
#include "custstr.h"
#include <stdio.h>
#include "custstr.h"

// Serial line 1 on the RPi hat is used for the console




void enqueue(unsigned char byte_1, unsigned char byte_2 );
void print_error(char *error);
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id);  // Binary: 00000001)
void execute_reverse_command(unsigned char id);  // Binary: 00000001)
void solonoid_command(unsigned char solonoid_id, // Solonoid ID. . 
                      unsigned char direction);  // S 33 go straight, C 34 go bent
void sol_off(); // Solonoid ID
// define a function that takes a char array as a parameter
//void tc1(char *arr) {
void tc1(char *command, char **num, int command_part_count);
void init_track();