#include "rpi.h"
#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "clockserver.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "UART2_MARKLIN_server.h"
#include "io_notifier.h"
#include "gameserver.h"

#include "gic.h"
#include "systimer.h"
#include "config.h"
#if !MARKLIN_HW_UART3
#include "auxuart.h"
#endif

#include "marklin_worker.h"
#include "track_server.h"
#include "shell.h"
#include "display_server.h"
#include "boot_tests.h"
#include "idle.h"

void command_shell();   /* declared in ui/shell.c */
void* STACK_EL0_START; // Maybe delete this later
#define CLOCKINTID 99
#define CLOCKSERVERON 1
#define UARTSERVERON 1
/*
 * Recent-window idle %.
 * Each iteration: read cumulative idle/kernel counters, take the delta against
 * the previous sample, push into a ring buffer, sum the window, print.
 * Window size is set at compile time via IDLE_WINDOW (default 100 samples).
 */
#ifndef IDLE_WINDOW
#define IDLE_WINDOW 100
#endif

void idle(){
	uint32_t r_buf[IDLE_WINDOW];
	uint32_t k_buf[IDLE_WINDOW];
	for (int i = 0; i < IDLE_WINDOW; i++) { r_buf[i] = 0; k_buf[i] = 0; }
	int     idx     = 0;
	int     filled  = 0;
	uint32_t last_r = GetRuntime();
	uint32_t last_k = GetKernelRuntime();

	while(1){
		uint32_t r = GetRuntime();
		uint32_t k = GetKernelRuntime();
		r_buf[idx] = r - last_r;
		k_buf[idx] = k - last_k;
		last_r = r;
		last_k = k;
		idx = (idx + 1) % IDLE_WINDOW;
		if (filled < IDLE_WINDOW) filled++;

		uint64_t rs = 0, ks = 0;
		for (int i = 0; i < filled; i++) { rs += r_buf[i]; ks += k_buf[i]; }

		uint32_t cum_pct    = k ? (uint32_t)((100ULL * r) / k) : 0;
		uint32_t recent_pct = ks ? (uint32_t)((100ULL * rs) / ks) : 0;

		uart_printf(CONSOLE, "\033[2;1H\033[K");
		uart_printf(CONSOLE, "idle: recent=%u%% (w=%u/%u)  cumulative=%u%%",
		            recent_pct, filled, IDLE_WINDOW, cum_pct);
		asm("WFI");
	}
	Exit();
}

int kmain(void *reg) {  

  STACK_EL0_START = reg; // Immediately calls this to store stack_end point as x0
  InitSys(reg);

  uart_init();
  uart_config_and_enable(CONSOLE, 115200);
#if MARKLIN_HW_UART3
  /* Real Pi 4 + Marklin hat: PL011 UART3 @ 0xFE201600, 2400 baud, 8N2. */
  uart_config_and_enable_marklin();
#else
  /* QEMU build: Marklin link runs over the AUX mini-UART (serial1) instead.
     UART3 isn't modelled. Configure the AUX block lazily on first use; the
     marklin_worker is responsible for routing bytes through aux_uart_*. */
  aux_uart_init();
#endif

  gic_init();

  /* Name server first among priority-0 tasks so RegisterAs/WhoIs succeed
   * before io_notifier and UART servers start. */
  int tid = KernelCreate(0, nameserver, 0);
  /* display_server is launched right after nameserver so every other task
     can WhoIs("display") on first use. Priority 0 -- it must be responsive. */
  tid = KernelCreate(0, display_server, 0);
  uart_printf(CONSOLE, "display_server tid: %d\r\n", tid);
  tid = KernelCreate(IDLE_PRIORITY, idle, 0);

  /* Layered boot self-tests. Priority 1 so it runs early and gets a chance
     to spawn its higher-priority children (priority 0 / 2 inside the runner)
     before init_solonoids ties up marklin_worker. */
  tid = KernelCreate(1, boot_test_runner, 0);
  uart_printf(CONSOLE, "boot_test_runner tid: %d\r\n", tid);
  #if CLOCKSERVERON == 1
  set_timerC3(get_timerLO() + 10000);
  route_interrupt(CLOCKINTID, 0);
  enable_interrupt(CLOCKINTID);
  /* clock_notifier polls the system-timer counter (the IRQ doesn't route
     through GIC under QEMU raspi4b). Priority 5 (below the shell at 4 and
     below the boot-test runner at 1) so the notifier only spins when
     everyone else is blocked -- avoids starving the shell. Shell + tests
     give it CPU naturally by Delay()-ing through clock_server. */
  tid = KernelCreate(5, clock_notifier, 0);
  tid = KernelCreate(0, clock_server, 0);
  #endif
  #if UARTSERVERON == 1
  // enable and route the interupt
  route_interrupt(UARTINTER, 0);
  enable_interrupt(UARTINTER);
  enable_RX_and_TX();


  // INIT THE IO-SERVERS AND NOTIFIERS
  tid = KernelCreate(0, io_notifier, 0);
  uart_printf(CONSOLE, "io_notifier tid: %d\r\n", tid);
  tid = KernelCreate(0, UART2_MARKLIN_server, 0);
  uart_printf(CONSOLE, "UART2_MARKLIN_server tid: %d\r\n", tid);
  tid = KernelCreate(0, UART1_CONSOLE_server, 0);
  uart_printf(CONSOLE, "UART1_CONSOLE_server tid: %d\r\n", tid);
  #endif


  // tid = KernelCreate(0, switch_worker, 0);

  /* init_solonoids used to bulk-Send 22 switch commands at boot. Under the
     polling-clock kernel that blocks clock_notifier (prio 5) from running
     during the K3 self-test. The shell can recreate switch state on demand
     via `sw <id> <S|C>`; we skip the auto-init here. */
  // tid = KernelCreate(2, init_solonoids, 0);
  /* command_shell is spawned by boot_test_runner once K1..K4 self-tests pass. */
    // sensor servers


  /* marklin_worker_read_notifier loops Send→Send→Send to grab s88 sensor
     bytes; at priority 1 it perpetually owns the prio-1 slot and starves
     clock_notifier (prio 5), so K3 + the shell hang. Demote it to prio 6 so
     it only runs when nothing higher is ready. */
  tid = KernelCreate(6, marklin_worker_read_notifier, 0);
  uart_printf(CONSOLE, "marklin_worker_read_notifier tid: %d\r\n", tid);
  tid = KernelCreate(1, marklin_worker, 0);
  uart_printf(CONSOLE, "marklin_worker tid: %d\r\n", tid);
  //uart_printf(CONSOLE, "marklin_worker tid: %d\r\n", tid);
  // clear the screen
  
  tid = KernelCreate(1, track_server, 0);
  uart_printf(CONSOLE, "track_server tid: %d\r\n", tid);

  uart_printf(CONSOLE, "\033[2J");
  Schedule();
  // U-Boot displays the return value from main - might be handy for debugging

  return 0;
}
