/*
 * display.h
 *
 *  Created on: May 4, 2026
 *      Author: Ibrahim Munir Ali
 */

/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 *
 * FILE: display.h
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Public interface for the high-level display management layer.
 *   This is the PRIMARY INTEGRATION POINT for every other engineer.
 *
 * HOW TO USE THIS FILE:
 *   1. #include "display.h" in your own .c file.
 *   2. Call Display_Init() once during startup (after ST7789_Init()).
 *   3. Call Display_UpdateSteps() and Display_UpdatePace() from your
 *      step-counting loop whenever values change.
 *   4. Call Display_SetStatus() to show transient messages (e.g. "Ready").
 *   5. Do NOT modify this file.  Do NOT call ST7789_* functions directly.
 *
 * READ-ONLY CONTRACT:
 *   This header is the interface contract between the LCD Engineer and
 *   all other engineers.  It defines what the LCD subsystem can do.
 *   Include it, call the functions, never edit it.
 * ============================================================
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "st7789.h"
#include <string.h>
#include <stdio.h>

/* Initialise the display: clear to black, draw static "STEPS" and "PACE" labels.
 * Call once after ST7789_Init() and before any other Display_* function. */
void Display_Init(void);

/* Update the step count shown on screen (large white number, centred).
 * Has a dirty-flag guard — only redraws if steps != last value.
 *   steps — total accumulated step count (uint32_t) */
void Display_UpdateSteps(uint32_t steps);

/* Update the pace badge at the bottom of the screen.
 * Has a dirty-flag guard — only redraws if pace != last value.
 *   pace — one of PACE_STATIONARY, PACE_WALKING, PACE_RUNNING */
void Display_UpdatePace(uint8_t pace);

/* Show a short status message centred in yellow at the top of the screen.
 * Has a dirty-flag guard — no redraw if msg equals last message.
 *   msg — null-terminated string (max ~21 chars).  Pass "" to clear. */
void Display_SetStatus(const char *msg);

/* Erase the status bar.  Equivalent to Display_SetStatus(""). */
void Display_ClearStatus(void);

/* Pace state constants — pass to Display_UpdatePace().
 * [LCD - DO NOT MODIFY]: These values map to specific badge colours in display.c. */
#define PACE_STATIONARY 0   /* < 40 SPM  — dark blue badge, cyan text */
#define PACE_WALKING    1   /* 40-130 SPM — green badge, black text */
#define PACE_RUNNING    2   /* > 130 SPM  — red badge, white text */

/* SPM threshold constants — define the boundaries between pace states.
 * [LCD - DO NOT MODIFY]: If you change these, also update stage_tracker.h.
 * Both files define the same constants and must stay in sync. */
#define SPM_STATIONARY_MAX 40.0f    /* Below this SPM → STATIONARY */
#define SPM_WALKING_MAX 130.0f      /* At or below this SPM → WALKING; above → RUNNING */


#endif
