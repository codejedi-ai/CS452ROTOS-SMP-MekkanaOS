#ifndef _shell_h_
#define _shell_h_ 1

/* Layout constants for the positional shell UI. The logo lives at
   (1,1)-(6,~70); the status panel under it; the prompt at SHELLROW. */
#define SHELLROW    20
#define SHELLCOL    1
#define TICKSCOL    60      /* right-side column for the "Ticks:" readout   */
#define LOGO_WIDTH  70      /* width of the logo block; for prompt offset   */
#define NAMEOFFSET  10      /* "DARCY[%u]> " glyphs before the cursor       */

/* Below servers (0); idle stays at SCHED_LOWEST_PRIORITY (255). */
#define TERMINAL_SHELL_PRIORITY 20

void command_shell();

#endif /* shell.h */
