#include "rpi.h"


void route_interrupt(uint32_t interrupt_id, uint8_t cpu_target);
void enable_interrupt(uint32_t interrupt_id);
uint32_t readInterruptId();
void endInterrupt(uint16_t interrupt_id);