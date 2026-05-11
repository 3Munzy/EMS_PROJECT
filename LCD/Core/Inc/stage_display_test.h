#ifndef __STAGE_DISPLAY_TEST_H
#define __STAGE_DISPLAY_TEST_H

#include "display.h"
#include "main.h"

/*
 * Debug stage: directly cycles Display_UpdatePace through all three pace
 * states (STATIONARY → WALKING → RUNNING) 3 times, holding each for 3 s.
 * No SPM logic, no step counting — pure display-layer validation.
 * Runs for ~27 s then returns so Stage_Tracker_Run() can start normally.
 *
 * If RUNNING badge does NOT appear here, the bug is in display.c.
 * If RUNNING badge appears here but not in the tracker, the bug is in
 * the SPM / phase logic in stage_tracker.c.
 */
void Stage_DisplayTest_Run(void);

#endif
