#include "rpi.h"

void init_gic(void);
void gic_init(void);
uint32_t checkActiveInterrupt(uint32_t interrupt_id);
void setActiveInterrupt(uint32_t interrupt_id);
void INTERRUPT_CLEAR_ACTIVE_REGS(uint32_t interrupt_id);
void route_interrupt(uint32_t interrupt_id, uint8_t cpu_target);
void enable_interrupt(uint32_t interrupt_id);
uint32_t readInterruptId();
void clear_GICC_EOIR(uint16_t interrupt_id);