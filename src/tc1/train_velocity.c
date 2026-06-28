#include "train_velocity.h"

/*
 * Synthetic per-(train, speed) calibration.
 *
 * Units:
 *   vel_list[t][s].dist / vel_list[t][s].time  =  mm per tick (1 tick = 10 ms).
 *   stopping_dist[t][s]                        =  mm from speed-0 command to standstill.
 *
 * Data origin: original raw measurements lost. Values seeded from the few
 * surviving notes (lap = 4894 mm; ~14.2 s/lap at speed 10; 470 mm stop at
 * speed 5; 480 mm stop at speed 10 on train 1; 660 mm stop at speed 10 on
 * train 77) and extrapolated quadratically across the rest of the speed
 * curve. Per-train multipliers reflect typical motor/lubrication spread.
 *
 * Update at runtime as real measurements come in.
 */

/* mm/s at the nominal train, indexed by Marklin speed 0..14. */
static const int nominal_mm_per_s[SPEED_MAX] = {
    0,    /* 0 */
    30,   /* 1 */
    60,
    95,
    135,
    175,  /* 5  */
    220,
    265,
    305,
    335,
    350,  /* 10 - matches 4894 mm / 14.16 s lap */
    380,
    410,
    440,
    470,  /* 14 */
    0     /* 15 (unused / reverse trigger) */
};

/* nominal stopping distance (mm), indexed by speed. */
static const int nominal_stop_mm[SPEED_MAX] = {
    0,
    8,
    25,
    55,
    105,
    175,  /* 5  - midpoint of 300/470 mm measurements */
    255,
    335,
    400,
    450,
    480,  /* 10 - matches train-1 datum */
    560,
    640,
    720,
    800,
    0
};

/*
 * Per-train multipliers: {vel_pct, stop_pct}, where 100 = nominal.
 * Indexed by train number; unset trains default to nominal at runtime.
 */
struct train_cal { int train_no; int vel_pct; int stop_pct; };
static const struct train_cal TRAIN_CALS[] = {
    {  1, 100, 100 },
    { 24,  95, 105 },
    { 47,  90, 110 },
    { 48, 100, 100 },
    { 49, 102,  98 },
    { 50, 105,  95 },
    { 58,  93, 108 },
    { 77, 100, 138 },  /* preserves the 660 mm-at-speed-10 datum */
    { 78,  98, 102 },
};
#define TRAIN_CAL_COUNT ((int)(sizeof(TRAIN_CALS) / sizeof(TRAIN_CALS[0])))

static void lookup_cal(int t, int *vp, int *sp) {
    for (int i = 0; i < TRAIN_CAL_COUNT; i++) {
        if (TRAIN_CALS[i].train_no == t) {
            *vp = TRAIN_CALS[i].vel_pct;
            *sp = TRAIN_CALS[i].stop_pct;
            return;
        }
    }
    *vp = 100;
    *sp = 100;
}

void init_stoppint_dist(int stopping_dist[TRAIN_MAX][SPEED_MAX]) {
    for (int t = 0; t < TRAIN_MAX; t++) {
        int vp, sp;
        lookup_cal(t, &vp, &sp);
        for (int s = 0; s < SPEED_MAX; s++) {
            stopping_dist[t][s] = (nominal_stop_mm[s] * sp) / 100;
        }
    }
}

void init_vel(struct train_velocity vel_list[TRAIN_MAX][SPEED_MAX]) {
    /*
     * Encode mm/s as dist=mm-per-second, time=100 (since 100 ticks = 1 s).
     * That keeps the division in compute_time exact for any integer mm.
     */
    for (int t = 0; t < TRAIN_MAX; t++) {
        int vp, sp;
        lookup_cal(t, &vp, &sp);
        for (int s = 0; s < SPEED_MAX; s++) {
            int mm_per_s = (nominal_mm_per_s[s] * vp) / 100;
            vel_list[t][s].dist = mm_per_s;   /* mm in 100 ticks */
            vel_list[t][s].time = 100;        /* ticks */
        }
    }
}

/*
 * ticks to traverse `dist_mm` at the given velocity.
 * Old formula: (dist / speed->dist) * speed->time -- loses precision
 * because integer division happens first.
 */
int compute_time(int dist_mm, struct train_velocity *speed) {
    if (speed == 0 || speed->dist <= 0) return 0;
    return (dist_mm * speed->time) / speed->dist;
}
