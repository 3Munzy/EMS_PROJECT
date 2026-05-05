/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 * Other team members must NOT edit this file directly.
 *
 * FILE: stage_tracker.c
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Implements Stage 3 of the startup sequence: the main step-tracking
 *   infinite loop.  This stage runs forever after Stage 2 completes.
 *
 *   It currently uses a SIMULATION to generate fake step events so
 *   the LCD and pace classification logic can be tested without real
 *   hardware.  The Step Tracker Engineer must replace this simulation
 *   with real ADXL335 step-detection calls.
 *
 * ALGORITHM OVERVIEW:
 *
 *   1. Rolling Window (100 timestamps, 10-second window):
 *      Each time a step is detected, its HAL_GetTick() timestamp is
 *      stored in step_times[].  Entries older than STEP_WINDOW_MS (10s)
 *      are discarded.  The window holds at most 100 recent steps.
 *
 *   2. Steps-Per-Minute (SPM) Calculation:
 *      SPM = (n - 1) × 60000 / span_ms
 *      where n = number of steps in the window,
 *            span_ms = newest_timestamp - oldest_timestamp
 *      This counts INTERVALS between steps, which is always n-1 for n
 *      timestamps.  Requires at least 2 steps in the window.
 *
 *   3. Pace Classification:
 *      SPM < SPM_STATIONARY_MAX (40)  → PACE_STATIONARY
 *      SPM ≤ SPM_WALKING_MAX    (130) → PACE_WALKING
 *      SPM > SPM_WALKING_MAX    (130) → PACE_RUNNING
 *
 *   4. Display Update:
 *      Display_UpdateSteps() and Display_UpdatePace() are called every
 *      loop iteration.  Both functions have dirty-flag guards so the
 *      LCD is only redrawn when the value changes.
 *
 * HOW THE STEP TRACKER ENGINEER INTEGRATES:
 *   - Create your own .c/.h files for ADXL335 step detection.
 *   - Do NOT modify this file.
 *   - Replace the phase simulation block (see HOOK comment below) with
 *     a call to your step detection function.
 *   - Include "stage_tracker.h" to access PACE_* constants and
 *     Display_UpdateSteps() / Display_UpdatePace() via display.h.
 *   - If you change SPM thresholds, update the constants in stage_tracker.h
 *     and display.h — both files define the same values and must stay in sync.
 * ============================================================
 */

#include "stage_tracker.h"
#include "st7789.h"
#include "display.h"

/*
 * [LCD - DO NOT MODIFY]: Stage_Tracker_Run
 *
 * Main entry point for Stage 3.  Never returns — runs an infinite loop.
 * Handles all step counting, SPM calculation, pace classification, and
 * display updates.
 *
 * Parameters: none
 * Returns:    void (infinite loop — never returns to caller)
 */
void Stage_Tracker_Run(void)
{
    /* Initialise display to zero steps, stationary pace */
    Display_UpdateSteps(0);
    Display_UpdatePace(PACE_STATIONARY);

    /* --- Step tracking state variables ---
     * step_count       : total accumulated steps since power-on (monotonically increasing)
     * phase_start      : timestamp of the last phase transition (simulation only)
     * phase            : current simulation phase (0=stationary, 1=walking, 2=running)
     * step_interval_ms : simulated time between steps in ms (simulation only)
     * step_times[]     : ring buffer of the last 100 step timestamps (HAL_GetTick values)
     * step_count_window: number of valid entries currently in step_times[] */
    uint32_t step_count       = 0;
    uint32_t phase_start      = HAL_GetTick();
    uint8_t  phase            = 0;
    uint32_t step_interval_ms = 1000;  /* Start in stationary simulation (1 step per second) */

    static uint32_t step_times[100];   /* Timestamps of the last 100 detected steps */
    static uint8_t  step_count_window = 0;  /* How many entries in step_times[] are currently valid */

    while (1)
    {
        uint32_t now           = HAL_GetTick();
        uint32_t phase_elapsed = now - phase_start;

        /* ================================================================
         * [HOOK - Step Tracker Engineer]: Replace the ENTIRE simulation
         * block below (phase transitions + HAL_Delay + step_count++) with
         * your real accelerometer-based step detection.
         *
         * What you need to do:
         *   1. Poll or interrupt-drive the ADXL335 for step events.
         *   2. When a step is detected, increment step_count by 1.
         *   3. Remove the HAL_Delay — your detection loop must be
         *      non-blocking or interrupt-driven so SPM timing is accurate.
         *   4. Remove the phase transition block entirely once real detection
         *      is in place.
         *
         * Example of what your code should look like in your own .c file:
         *   if (MyStepDetector_StepDetected()) {
         *       step_count++;
         *   }
         *
         * Do NOT write your step detection code in this file.
         * Include "stage_tracker.h" in your own .c file and call
         * Stage_Tracker_Run() from main.c as it is currently.
         * ================================================================ */

        /* --- Simulation: Phase transitions (REMOVE when real sensor integrated) ---
         * Cycles through three activity phases to demonstrate pace display:
         *   Phase 0 (PHASE_STATIONARY_MS): 1 step every 1000ms → ~60 SPM (stationary)
         *   Phase 1 (PHASE_WALKING_MS):    1 step every 400ms  → ~150 SPM (walking)
         *   Phase 2 (PHASE_RUNNING_MS):    1 step every 200ms  → ~300 SPM (running)
         * Each phase lasts PHASE_*_MS milliseconds (10 seconds by default). */
        if (phase == 0 && phase_elapsed >= PHASE_STATIONARY_MS)
        {
            phase             = 1;
            step_interval_ms  = 400;   /* Simulate walking pace (~150 SPM) */
            phase_start       = HAL_GetTick();
        }
        else if (phase == 1 && phase_elapsed >= PHASE_WALKING_MS)
        {
            phase             = 2;
            step_interval_ms  = 200;   /* Simulate running pace (~300 SPM) */
            phase_start       = HAL_GetTick();
        }
        else if (phase == 2 && phase_elapsed >= PHASE_RUNNING_MS)
        {
            phase             = 0;
            step_interval_ms  = 1000;  /* Back to stationary (~60 SPM) */
            phase_start       = HAL_GetTick();
        }

        /* [PLACEHOLDER - Step Tracker Engineer]: Remove this HAL_Delay.
         * It is only here to simulate the time between steps.
         * Real step detection must NOT block with a fixed delay. */
        HAL_Delay(step_interval_ms);

        /* [PLACEHOLDER - Step Tracker Engineer]: Replace this unconditional
         * increment with a conditional check from your step detection algorithm:
         *   e.g.  if (MyStepDetector_StepDetected()) step_count++;
         * The current code always increments, which is only valid in simulation. */
        step_count++;
        now = HAL_GetTick();  /* Refresh timestamp immediately after step event */

        /* --- Rolling window: store this step's timestamp ---
         * If the buffer has space, append to the end.
         * If the buffer is full (100 entries), shift everything left by one
         * and put the new timestamp at the end (oldest entry is discarded). */
        if (step_count_window < 100)
        {
            step_times[step_count_window++] = now;
        }
        else
        {
            for (uint8_t j = 0; j < 99; j++)
                step_times[j] = step_times[j + 1];
            step_times[99] = now;
        }

        /* Discard any entries older than STEP_WINDOW_MS (10 seconds) from the
         * front of the array.  This keeps the window bounded to recent activity
         * so SPM reflects current pace rather than steps taken minutes ago.
         * Linear scan from index 0; each removed entry shifts the rest down. */
        uint8_t i = 0;
        while (i < step_count_window)
        {
            if ((now - step_times[i]) > STEP_WINDOW_MS)
            {
                /* This entry is too old — remove it by shifting everything after it left */
                for (uint8_t j = i; j < step_count_window - 1; j++)
                    step_times[j] = step_times[j + 1];
                step_count_window--;
                /* Do not increment i — recheck the same index after the shift */
            }
            else i++;
        }

        /* --- SPM (Steps Per Minute) calculation ---
         *
         * Formula:  SPM = (n - 1) × 60000 / span_ms
         *
         * Explanation:
         *   n         = number of step timestamps in the window
         *   (n - 1)   = number of INTERVALS between those n steps
         *   span_ms   = newest_timestamp - oldest_timestamp (duration of all intervals)
         *   60000     = milliseconds per minute
         *
         * Dividing (n-1) intervals by their total duration gives intervals/ms,
         * multiplied by 60000 converts to steps/minute.
         *
         * Requires at least 2 steps (1 interval) to compute a meaningful SPM.
         * With only 1 step in the window, SPM stays at 0.0 (PACE_STATIONARY). */
        float spm = 0.0f;
        if (step_count_window >= 2)
        {
            uint32_t oldest  = step_times[0];                        /* Oldest valid step time */
            uint32_t newest  = step_times[step_count_window - 1];   /* Most recent step time */
            uint32_t span_ms = newest - oldest;                      /* Total window duration */

            if (span_ms > 0)  /* Guard against division by zero (two steps at same tick) */
                spm = (step_count_window - 1) * 60000.0f / span_ms;
        }

        /* --- Pace classification ---
         * Three bands based on SPM thresholds defined in stage_tracker.h:
         *   SPM < 40  → STATIONARY  (fewer than 40 steps per minute)
         *   SPM ≤ 130 → WALKING     (40–130 steps per minute)
         *   SPM > 130 → RUNNING     (more than 130 steps per minute)
         *
         * [HOOK - Step Tracker Engineer]: If these thresholds need tuning
         *   for your hardware, update SPM_STATIONARY_MAX and SPM_WALKING_MAX
         *   in BOTH stage_tracker.h AND display.h — they must stay in sync. */
        uint8_t pace;
        if (spm < SPM_STATIONARY_MAX)
            pace = PACE_STATIONARY;
        else if (spm <= SPM_WALKING_MAX)
            pace = PACE_WALKING;
        else
            pace = PACE_RUNNING;

        /* --- Update display ---
         * Both calls use dirty-flag guards internally, so if neither value
         * has changed since the last iteration, no SPI traffic is generated. */
        Display_UpdateSteps(step_count);  /* Large white number in the centre of the screen */
        Display_UpdatePace(pace);         /* Coloured badge at the bottom of the screen */
    }
}
