#ifndef DISPLAY_SERVER_H
#define DISPLAY_SERVER_H

#include <stddef.h>

// DISPLAY_PAYLOAD_MAX = (2 << 20) — 2 MiB of payload room per message
// (room for the full ASCII logo plus any future bulk text). The message
// length is one byte more for the type tag.
#define DISPLAY_PAYLOAD_MAX (2 << 20)
#define DISPLAY_MSG_LEN     (DISPLAY_PAYLOAD_MAX + 1)

#define DISPLAY_PUTS 1
#define DISPLAY_PRINT_SENSORS 2

void display_server(void);

int DisplayServerTid(void);
int DisplayPuts(const char *s);
void DisplayPrintf(char *fmt, ...);
void DisplayPrintSensors(int arr[], size_t alen, unsigned int column, unsigned int row);

#endif
