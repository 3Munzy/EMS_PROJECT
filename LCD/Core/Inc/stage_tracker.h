/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 *
 * FILE: stage_tracker.h
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Public interface for Stage 3: the infinite step-tracking loop.
 *   This header is the READ-ONLY CONTRACT between the LCD Engineer
 *   and the Step Tracker Engineer.
 *
 * HOW THE STEP TRACKER ENGINEER USES THIS FILE:
 *   1. #include "stage_tracker.h" in your own .c file.
 *   2. Do NOT call Stage_Tracker_Run() from your code — main.c calls it.
 *   3. Your job is to replace the simulation phase block inside
 *      stage_tracker.c with real ADXL335 step-detection calls.
 *      Coordinate with the LCD Engineer to do this at the marked
 *      HOOK/PLACEHOLDER locations in stage_tracker.c.
 *   4. If you need to tune SPM thresholds, update the constants below
 *      AND the matching constants in display.h — they must stay in sync.
 *   5. Never modify this header or any other LCD file directly.
 *
 * EXAMPLE SKELETON (.c file the Step Tracker Engineer writes):
 *
 *   // my_step_detector.c
 *   #include "stage_tracker.h"  // access PACE_* constants and display API
 *   #include "my_accel.h"       // your ADXL335 driver
 *
 *   uint8_t MyStepDetector_StepDetected(void) {
 *       // Read ADXL335 acceleration, apply threshold / zero-crossing logic
 *       return step_flag;  // return 1 if a step event occurred, else 0
 *   }
 *   // Coordinate with LCD Engineer to call this inside stage_tracker.c
 *   // replacing the HAL_Delay + step_count++ simulation.
 * ============================================================
 */

#ifndef __STAGE_TRACKER_H
#define __STAGE_TRACKER_H

#include "display.h"
#include "main.h"

/* SPM pace thresholds — define the boundaries between the three pace states.
 * [LCD - DO NOT MODIFY]: If you change these values, also update display.h.
 *   Both files define the same thresholds and MUST stay in sync. */
#define SPM_STATIONARY_MAX   40.0f   /* Below this → PACE_STATIONARY */
#define SPM_WALKING_MAX     130.0f   /* At or below this → PACE_WALKING; above → PACE_RUNNING */

/* Fixed-count step window: retain only the last N step timestamps. */
#define STEP_WINDOW_N       8      /* number of recent step timestamps to keep */
#define IDLE_TIMEOUT_MS     1500   /* force PACE_STATIONARY if no step in this many ms */

/* Simulation phase durations (ms) — used ONLY while the simulation is in place.
 * [HOOK - Step Tracker Engineer]: These constants become irrelevant once you
 *   replace the phase simulation block with real step detection. */
#define PHASE_STATIONARY_MS   8000  /* How long the stationary simulation phase lasts */
#define PHASE_WALKING_MS      8000  /* How long the walking simulation phase lasts */
#define PHASE_RUNNING_MS     25000  /* How long the running simulation phase lasts */

/* Main entry point for Stage 3 — called by main.c.
 * Runs an infinite loop: detects steps, calculates SPM, updates LCD.
 * Never returns.
 * [HOOK - Step Tracker Engineer]: Replace simulation inside this function
 *   with your real accelerometer step detection. */
void Stage_Tracker_Run(void);

#endif
