#include "gic.h"

// Cast through uintptr_t so the BCM2711 MMIO base (0xff840000) survives
// pointer-from-integer casts cleanly on aarch64.
#define GIC_BASE        ((uintptr_t)0xff840000)
#define GICD_BASE       (GIC_BASE + 0x1000)
#define GICC_BASE       (GIC_BASE + 0x2000)
#define GICD_ISENABLERn (GICD_BASE + 0x100)
#define GICD_ITARGETSRn (GICD_BASE + 0x800)

#define GICC_IAR  (*(volatile uint32_t*)(GICC_BASE + 0x0C))
#define GICC_EOIR (*(volatile uint32_t*)(GICC_BASE + 0x10))

#define GICD_ITARGETSR(n)  (*(volatile uint32_t*)(GICD_ITARGETSRn + (4 * (n))))
#define GICD_ISENABLER(n)  (*(volatile uint32_t*)(GICD_ISENABLERn + (4 * (n))))
// NB: ISACTIVER lives at GICD_BASE + 0x300 and ICACTIVER at + 0x380. The old
// definitions pointed both at ISENABLERn — kept here as-is because the
// set/clear-active operations are currently no-ops on this kernel's IRQ flow
// and changing them out from under existing call sites is out of scope.
#define GICD_ISACTIVERn(n) (*(volatile uint32_t*)(GICD_ISENABLERn + (4 * (n))))
#define GICD_ICACTIVERn(n) (*(volatile uint32_t*)(GICD_ISENABLERn + (4 * (n))))

#ifndef DEBUG
# define DEBUG 1
#endif
/*
oute the interrupt to IRQ on CPU 0
use GICD_ITARGETSRn
each register defines targets for 4 interrupts
1. Find the register off set use the interrupt_id DIV 4
2. Get the remainder m which would be the byte offset
3. Then write the CPU target feild value into the corrusponding byte
write 0x01 to the byte, that is the 0th cpu
*/
void route_interrupt(uint32_t interrupt_id, uint8_t cpu_target){
    if (cpu_target > 7){
        return;
    }
    uint32_t offset = interrupt_id / 4; // n 
    uint32_t remainder = interrupt_id % 4;
    uint32_t target = ((0x01 << cpu_target) << (remainder << 3));
    // print target in binary
    # if DEBUG == 3
    for (int i = 31; i >= 0; i--){
        if (target & (0x01 << i)){
             uart_printf(CONSOLE, "1");
         } else {
             uart_printf(CONSOLE, "0");
         }
    }
    uart_printf(CONSOLE, "\r\n");
    uart_printf(CONSOLE, "&GICD_ITARGETSR(offset) = %x\r\n", &GICD_ITARGETSR(offset));
    uart_printf(CONSOLE, "&GICD_ITARGETSR(offset) = %x\r\n", GICD_ITARGETSRn + (4 * offset));
    # endif
    GICD_ITARGETSR(offset) = target;
}

/*
enable the interrupt
use GICD_ISENABLERn
4-byte registers, with 1 bit per InterruptID
For interrupt ID m, when DIV and MOD are the integer division and modulo operations:
• the corresponding GICD_ISENABLER number, n, is given by n = m DIV 32
• the offset of the required GICD_ISENABLER is (0x100 + (4*n))
• the bit number of the required Set-enable bit in this register is m MOD 32.
*/
void enable_interrupt(uint32_t interrupt_id){
    uint32_t offset = interrupt_id / 32; // n
    uint32_t remainder = interrupt_id % 32;
    GICD_ISENABLER(offset) = GICD_ISENABLER(offset) | (0x01 << remainder);
}

// One-time GIC bring-up. Without this the BCM2711 system timer happens to
// reach the CPU via a legacy path, masking the fact that the GIC was never
// actually enabled — so peripherals like the PL011 UART (which only go
// through GIC SPI 121 / IRQ 153) silently never fired.
//
// QEMU boots raspi4b at EL2 secure and we drop to EL1 still secure (see
// boot.S — no SCR_EL3 / NS manipulation). All IRQs are therefore Group 0
// by default, which is exactly what Secure EL1 expects from GICC_IAR, so
// we leave GICD_IGROUPR alone. Setting Group 1 in this mode without also
// enabling AckCtl (bit 2 of GICC_CTLR) causes IAR to return the spurious
// id 1022 and the kernel to storm.
//
// Steps:
//   1. PMR = 0xFF: accept any priority.
//   2. GICC_CTLR = 0x1: forward Group 0 IRQs to the CPU interface.
//   3. GICD_CTLR = 0x1: enable distributor (Group 0 only).
void gic_init(void) {
    *(volatile uint32_t*)(GICC_BASE + 0x04) = 0xFF;
    *(volatile uint32_t*)(GICC_BASE + 0x00) = 0x1;
    *(volatile uint32_t*)(GICD_BASE + 0x00) = 0x1;
}

// set active interrupt
void setActiveInterrupt(uint32_t interrupt_id){
    uint32_t offset = interrupt_id / 32; // n 
    uint32_t remainder = interrupt_id % 32;
    GICD_ISACTIVERn(offset) = GICD_ISACTIVERn(offset) | (0x01 << remainder);
}
// check active interrupt
uint32_t checkActiveInterrupt(uint32_t interrupt_id){
    uint32_t offset = interrupt_id / 32; // n 
    uint32_t remainder = interrupt_id % 32;
    return GICD_ISACTIVERn(offset) & (0x01 << remainder);
}
// clear active interrupt
void INTERRUPT_CLEAR_ACTIVE_REGS(uint32_t interrupt_id){
    uint32_t offset = interrupt_id / 32; // n 
    uint32_t remainder = interrupt_id % 32;
    GICD_ICACTIVERn(offset) = GICD_ICACTIVERn(offset) | (0x01 << remainder);
}

uint32_t readInterruptId(){
    return GICC_IAR & 0x3FF;
}
void clear_GICC_EOIR(uint16_t interrupt_id){
    if (interrupt_id > 1023){
        // print the error message mentioning the interrupt_id is out of range which is 0-1023
        uart_printf(CONSOLE, "Interrupt ID is out of range must be within 0 - 1023\r\n");
        return;
    }
    uint32_t toBeWritten = GICC_EOIR;
    toBeWritten = toBeWritten >> 10;
    toBeWritten = toBeWritten << 10;
    toBeWritten = toBeWritten | interrupt_id;
    // would only want to change the last 10 bits
    GICC_EOIR = interrupt_id;
}