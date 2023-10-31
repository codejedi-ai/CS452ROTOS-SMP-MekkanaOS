#include "rpi.h"
#include "util.h"
#include "ioserver.h"
#include "custstr.h"
#include <stdio.h>
#include "custstr.h"
uint32_t io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, io_CTS_MARKLIN_server_pid;
// Serial line 1 on the RPi hat is used for the console
#define SWITCH_COUNT 18

/*
read_one_s88(char s88_id) would return one byte of data from the s88
each bit in the byte would reflect the state of the sensor

*/
uint16_t read_one_s88(char s88_id);
uint16_t read_many_s88(char s88_no, uint16_t* ret);
void enqueue(unsigned char byte_1, unsigned char byte_2 );
void print_error(char *error);
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id);  // Binary: 00000001)
void execute_reverse_command(unsigned char id);  // Binary: 00000001)
void solonoid_command(unsigned char solonoid_id, // Solonoid ID. . 
                      unsigned char direction);  // S 33 go straight, C 34 go bent
void sol_off(); // Solonoid ID
// define a function that takes a char array as a parameter
void init_track();
void init_ioserver();