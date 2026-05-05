/*
 * display.c
 *
 *  Created on: May 4, 2026
 *      Author: Ibrahim Munir Ali
 */

/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 * Other team members must NOT edit this file directly.
 *
 * FILE: display.c
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   High-level display management layer sitting between the application
 *   stages (calibration, selftest, tracker) and the raw ST7789 driver.
 *   Responsible for rendering three on-screen regions:
 *
 *     ┌─────────────────────────────┐  ← y=20  STATUS BAR
 *     │  [STATUS MESSAGE in yellow] │           (transient, e.g. "Calibrating...")
 *     ├─────────────────────────────┤  ← y=80  "STEPS" label (static, gray)
 *     │         1234                │  ← y=110 Step count (large white number)
 *     ├─────────────────────────────┤  ← y=170 "PACE" label (static, gray)
 *     │  [  WALKING  ]              │  ← y=200 Pace badge (coloured rectangle + text)
 *     └─────────────────────────────┘
 *
 * DIRTY-FLAG PATTERN:
 *   Each public function tracks the last value it rendered (last_steps,
 *   last_pace, last_status).  If the new value equals the last, the
 *   function returns immediately without touching the LCD, avoiding
 *   unnecessary SPI traffic and flicker.
 *
 * HOW OTHER ENGINEERS USE THIS FILE:
 *   - Include "display.h" in your own .c file.
 *   - Call Display_Init() once after ST7789_Init().
 *   - Call Display_UpdateSteps() and Display_UpdatePace() whenever your
 *     step count or pace changes.
 *   - Call Display_SetStatus() to show a brief status message.
 *   - Do NOT modify this file.
 * ============================================================
 */

#include "display.h"
#include <string.h>

/* Layout constants — all Y coordinates are fixed pixel rows on the 240×240 display.
 * Adjust these if the UI layout needs to change (LCD Engineer only). */
#define STATUS_Y        20   /* Top of the status bar region */
#define STATUS_HEIGHT   30   /* Height in pixels of the status bar clear area */
#define STEPS_LABEL_Y   80   /* Y position of the static "STEPS" label */
#define STEPS_VALUE_Y   150  /* Y position used as baseline for the large step number */
#define PACE_LABEL_Y    170  /* Y position of the static "PACE" label */
#define PACE_BADGE_Y    200  /* Y position of the coloured pace badge rectangle */
#define PACE_TEXT_Y     220  /* Y position of pace text (unused separately — text drawn inside badge) */

/* Dirty-flag state — sentinel values chosen to guarantee a redraw on first call:
 *   last_steps = 9999999  → no real step count will match this on startup
 *   last_pace  = 255      → not a valid PACE_* constant (valid range 0-2)
 *   last_status = ""      → empty string, matches Display_ClearStatus() state */
static uint32_t last_steps = 9999999;
static uint8_t  last_pace  = 255;
static char     last_status[32] = "";

/*
 * [LCD - DO NOT MODIFY]: Display_Init
 *
 * Clears the entire screen to black and draws the two static labels
 * ("STEPS" and "PACE") that remain on screen for the lifetime of the
 * step-tracking stage.  Call this exactly once, after ST7789_Init(),
 * before calling any other Display_* function.
 *
 * Parameters: none
 * Returns:    void
 */
void Display_Init(void)
{
    ST7789_Fill_Color(BLACK);  /* Wipe entire display to black */

    /* Draw static labels — these never change, so they are painted once here */
    ST7789_WriteString(92, STEPS_LABEL_Y, "STEPS", Font_11x18, GRAY, BLACK);
    ST7789_WriteString(98, PACE_LABEL_Y,  "PACE",  Font_11x18, GRAY, BLACK);

}

/*
 * [LCD - DO NOT MODIFY]: Display_SetStatus
 *
 * Shows a short status message centred horizontally in the status bar
 * region at the top of the screen (y = STATUS_Y).  Text is rendered in
 * yellow using Font_11x18.  If msg equals the last message shown the
 * function returns immediately (dirty-flag guard).  Pass "" or call
 * Display_ClearStatus() to erase the bar.
 *
 * Parameters:
 *   msg — null-terminated string to display (max ~21 chars for 240px wide)
 * Returns: void
 *
 * Example calls from stage files:
 *   Display_SetStatus("Calibrating...");
 *   Display_SetStatus("Ready");
 *   Display_ClearStatus();   // equivalent to Display_SetStatus("")
 */
void Display_SetStatus(const char *msg)
{
    if (strcmp(msg, last_status) == 0) return;  /* Skip redundant redraw */

    /* Clear status bar area before drawing new text */
    ST7789_Fill(0, STATUS_Y, 239, STATUS_Y + STATUS_HEIGHT, BLACK);

    if (strlen(msg) > 0)
    {
        /* Centre the string: x = (display_width - text_pixel_width) / 2 */
        uint8_t num_chars = strlen(msg);
        uint16_t x = (240 - (num_chars * Font_11x18.width)) / 2;
        ST7789_WriteString(x, STATUS_Y + 8, msg, Font_11x18, YELLOW, BLACK);
    }

    strncpy(last_status, msg, sizeof(last_status) - 1);  /* Update dirty-flag */
}

/*
 * [LCD - DO NOT MODIFY]: Display_ClearStatus
 *
 * Erases the status bar region and resets the dirty-flag to "".
 * Thin wrapper around Display_SetStatus("").
 *
 * Parameters: none
 * Returns:    void
 */
void Display_ClearStatus(void)
{
    Display_SetStatus("");
}

/*
 * [LCD - DO NOT MODIFY]: Display_UpdateSteps
 *
 * Renders the current step count as a large white number centred on screen.
 * Uses Font_16x26 (each character is 16px wide × 26px tall).  Skips the
 * redraw if steps has not changed since the last call (dirty-flag).
 * The previous number is erased with a black fill before drawing the new one
 * so that wider numbers (e.g. "10000") don't leave artefacts from shorter ones.
 *
 * Parameters:
 *   steps — total accumulated step count to display (uint32_t)
 * Returns: void
 *
 * Call this from your step-counting code whenever the step count changes:
 *   Display_UpdateSteps(step_count);
 */
void Display_UpdateSteps(uint32_t steps)
{
    if (steps == last_steps) return;  /* Skip if unchanged */

    char buf[16];
    sprintf(buf, "%lu", steps);  /* Convert step count to string — no padding, just the number */

    /* Centre the number: x = (display_width - text_pixel_width) / 2 */
    uint8_t num_chars = strlen(buf);
    uint16_t x = (240 - (num_chars * Font_16x26.width)) / 2;

    /* Clear the entire number area before drawing to avoid digit artefacts */
    ST7789_Fill(0, STEPS_VALUE_Y - 40, 239, STEPS_VALUE_Y + 5, BLACK);

    ST7789_WriteString(x, STEPS_VALUE_Y - 40, buf, Font_16x26, WHITE, BLACK);

    last_steps = steps;  /* Update dirty-flag */
}

/*
 * [LCD - DO NOT MODIFY]: Display_UpdatePace
 *
 * Renders a colour-coded pace badge at the bottom of the screen.
 * Badge colour and text change depending on the pace state:
 *
 *   PACE_STATIONARY (0) → dark blue badge,  cyan text,  "STATIONARY"
 *   PACE_WALKING    (1) → green badge,       black text, "WALKING"
 *   PACE_RUNNING    (2) → red badge,         white text, "RUNNING"
 *
 * The badge width is sized to fit the label text plus 10px padding on
 * each side, then centred horizontally on the 240px wide display.
 * Skips the redraw if pace has not changed (dirty-flag).
 *
 * Parameters:
 *   pace — one of PACE_STATIONARY, PACE_WALKING, PACE_RUNNING (see display.h)
 * Returns: void
 *
 * Call this from your step-counting code whenever the pace classification changes:
 *   Display_UpdatePace(PACE_WALKING);
 */
void Display_UpdatePace(uint8_t pace)
{
    if (pace == last_pace) return;  /* Skip if unchanged */

    /* Erase previous badge area before drawing the new one */
    ST7789_Fill(0, PACE_BADGE_Y, 239, PACE_BADGE_Y + 28, BLACK);

    const char *pace_str;
    uint16_t badge_color, text_color;

    /* Select label string and colour scheme for the current pace state */
    switch (pace)
    {
        case PACE_STATIONARY:
            pace_str    = "STATIONARY";
            badge_color = DARKBLUE;
            text_color  = CYAN;
            break;
        case PACE_WALKING:
            pace_str    = "WALKING";
            badge_color = GREEN;
            text_color  = BLACK;
            break;
        case PACE_RUNNING:
            pace_str    = "RUNNING";
            badge_color = RED;
            text_color  = WHITE;
            break;
        default:
            return;  /* Unknown pace value — do nothing */
    }

    /* Calculate badge geometry:
     *   text_width = character count × font width (Font_11x18 → 11px per char)
     *   badge_x    = centre the badge with 10px padding on each side
     *   text_x     = centre the text within the full display width */
    uint8_t num_chars = strlen(pace_str);
    uint16_t text_width = num_chars * Font_11x18.width;
    uint16_t badge_x    = (240 - text_width - 20) / 2;  /* 20px total horizontal padding */
    uint16_t text_x     = (240 - text_width) / 2;

    /* Draw filled badge rectangle, then overlay the text */
    ST7789_Fill(badge_x, PACE_BADGE_Y, badge_x + text_width + 20, PACE_BADGE_Y + 28, badge_color);
    ST7789_WriteString(text_x, PACE_BADGE_Y + 5, pace_str, Font_11x18, text_color, badge_color);

    last_pace = pace;  /* Update dirty-flag */
}


