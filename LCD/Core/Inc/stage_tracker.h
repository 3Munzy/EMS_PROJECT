#ifndef __STAGE_TRACKER_H
#define __STAGE_TRACKER_H

#include "display.h"
#include "main.h"

// Pace thresholds — update if Marcus changes his values
#define SPM_STATIONARY_MAX   40.0f
#define SPM_WALKING_MAX     130.0f

// Rolling window size in ms
#define STEP_WINDOW_MS      10000

// Phase durations for simulation testing
#define PHASE_STATIONARY_MS  10000
#define PHASE_WALKING_MS     10000
#define PHASE_RUNNING_MS     10000

// Main entry point — call from main.c
// Runs forever (infinite loop inside)
void Stage_Tracker_Run(void);

#endif
