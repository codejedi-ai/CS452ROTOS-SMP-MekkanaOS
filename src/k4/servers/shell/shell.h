#define SHELLROW 20
#define SHELLCOL 1
#define TICKSCOL 200
#define LOGOOFFSET 10
#define LOGO_WIDTH 70
#define NAMEOFFSET 10

/* Below servers (0); idle stays at SCHED_LOWEST_PRIORITY (255). */
#define TERMINAL_SHELL_PRIORITY 20

/* Base interactive shell (bash equivalent). */
void commands_shell(void);

void train_shell(void);

/* Train-control app: track A/B, solenoids, sensors, tc1 commands. */
void train_shell(void);
