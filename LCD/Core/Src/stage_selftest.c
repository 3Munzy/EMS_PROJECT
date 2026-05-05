/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 * Other team members must NOT edit this file directly.
 *
 * FILE: stage_selftest.c
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Implements Stage 2 of the startup sequence: ADXL335 accelerometer
 *   self-test validation.  This stage reads two sets of voltage readings
 *   from the ADXL335 — one with the self-test pin inactive (baseline)
 *   and one with the self-test pin active (ST mode) — then compares
 *   the deltas against ADXL335 datasheet-specified expected values:
 *
 *     Axis X: expected delta = -325 mV  (ST pulls X output down)
 *     Axis Y: expected delta = +325 mV  (ST pulls Y output up)
 *     Axis Z: expected delta = +550 mV  (ST pulls Z output up)
 *
 *   A ±100 mV tolerance is applied to each delta.
 *   On failure, the device performs a full hardware reset (NVIC_SystemReset)
 *   and restarts from Stage 1.
 *
 * STAGE 2 FLOW:
 *   1. Show idle screen → wait for button
 *   2. Read baseline voltages (Vx, Vy, Vz) — [PLACEHOLDER values]
 *   3. Show baseline readings on LCD
 *   4. Enable ADXL335 self-test pin, read ST voltages — [PLACEHOLDER values]
 *   5. Show ST readings on LCD
 *   6. Show comparison table (baseline vs. ST vs. expected deltas)
 *   7. Show "Determining..." suspense screen
 *   8. Compute deltas, check against tolerance — [PLACEHOLDER logic]
 *   9. Show PASSED or FAILED result
 *  10. Show "Press Button" prompt, wait for button
 *  11. On failure: NVIC_SystemReset() → restarts from main()
 *  12. On success: status "Ready", return to main.c
 *
 * HOW THE SELF-TEST ENGINEER INTEGRATES:
 *   - Create your own .c/.h files for ADC reads and the self-test pin GPIO.
 *   - Do NOT modify this file.
 *   - Replace the six dummy float variables (lines ~188-193) with your
 *     real ADC voltage readings.
 *   - Replace the delta pass/fail check (lines ~209-217) with your own
 *     validated pass/fail logic.
 *   - Include "stage_selftest.h" in your own .c file to access the
 *     Display_ShowSelfTest*() functions.
 * ============================================================
 */

#include "stage_selftest.h"
#include "st7789.h"
#include "fonts.h"
#include <string.h>
#include <stdio.h>

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Draw_CentredString
 *
 * Internal helper. Centres a string horizontally on the 240px display.
 * See stage_calibration.c for full documentation of this function.
 *
 * Parameters:
 *   y     — vertical pixel row to start drawing at
 *   str   — null-terminated string
 *   font  — FontDef struct (Font_11x18, Font_16x26, Font_7x10)
 *   color — RGB565 foreground colour (background always BLACK)
 * Returns: void
 * ───────────────────────────────────────── */
static void Draw_CentredString(uint16_t y, const char *str,
                                FontDef font, uint16_t color)
{
    uint16_t x = (240 - (strlen(str) * font.width)) / 2;
    ST7789_WriteString(x, y, str, font, color, BLACK);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: SelfTest_WaitForButton
 *
 * Blocks until the user presses and releases the self-test button
 * (same GPIOC pin 13 as the calibration button).  50 ms debounce
 * delays are applied after both edges.
 *
 * NOTE: The wait order here is the INVERSE of Calibration_WaitForButton:
 *   First loop exits when pin reads RESET (not SET) — waits while SET
 *   Second loop exits when pin reads SET (not RESET) — waits while RESET
 * This reflects the button being active-low in the self-test context
 * (or a wiring difference). Do NOT change this logic — it matches the
 * hardware setup for Stage 2.
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void SelfTest_WaitForButton(void)
{
    /* Wait while pin is SET (high) — exits when pin goes RESET (low) */
    while (HAL_GPIO_ReadPin(BTN_SELFTEST_PORT, BTN_SELFTEST_PIN)
           == GPIO_PIN_SET);
    HAL_Delay(50);  /* Debounce */
    /* Wait while pin is RESET (low) — exits when pin goes SET (high) */
    while (HAL_GPIO_ReadPin(BTN_SELFTEST_PORT, BTN_SELFTEST_PIN)
           == GPIO_PIN_RESET);
    HAL_Delay(50);  /* Debounce */
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestIdle
 *
 * Clears the screen and shows the self-test idle prompt:
 *   "Begin Self-Test"
 *   "Routine!"
 *   "Press Button"
 *   "To Start!"
 * in white.  Stage_SelfTest_Run() immediately waits for a button press
 * after calling this function.
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Display_ShowSelfTestIdle(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(70,  "Begin Self-Test", Font_11x18, WHITE);
    Draw_CentredString(95,  "Routine!",        Font_11x18, WHITE);
    Draw_CentredString(135, "Press Button",    Font_11x18, WHITE);
    Draw_CentredString(160, "To Start!",       Font_11x18, WHITE);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestBaseline
 *
 * Shows the ADXL335 baseline (self-test pin OFF) voltage readings:
 *   "Reading Baseline"
 *   "Axis Outputs..."
 *   "Vx: <vx>mV"    in yellow
 *   "Vy: <vy>mV"    in yellow
 *   "Vz: <vz>mV"    in yellow
 * Stays on screen for 2 seconds then returns.
 *
 * Parameters:
 *   vx — baseline X-axis output voltage in millivolts
 *   vy — baseline Y-axis output voltage in millivolts
 *   vz — baseline Z-axis output voltage in millivolts
 * Returns: void
 *
 * [HOOK - Self-Test Engineer]: Pass your real ADC-converted voltages here.
 *   Do NOT read hardcoded values. Sample the ADXL335 with self-test pin LOW,
 *   convert ADC counts to mV, then call:
 *     Display_ShowSelfTestBaseline(real_vx, real_vy, real_vz);
 * ───────────────────────────────────────── */
void Display_ShowSelfTestBaseline(float vx, float vy, float vz)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(30,  "Reading Baseline", Font_11x18, WHITE);
    Draw_CentredString(55,  "Axis Outputs...",  Font_11x18, WHITE);

    char buf[32];

    sprintf(buf, "Vx: %.0fmV", vx);
    Draw_CentredString(110, buf, Font_11x18, YELLOW);

    sprintf(buf, "Vy: %.0fmV", vy);
    Draw_CentredString(135, buf, Font_11x18, YELLOW);

    sprintf(buf, "Vz: %.0fmV", vz);
    Draw_CentredString(160, buf, Font_11x18, YELLOW);

    HAL_Delay(2000);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestST
 *
 * Shows the ADXL335 self-test mode (self-test pin ON) voltage readings:
 *   "Reading ST"
 *   "Axis Outputs..."
 *   "ST_Vx: <st_vx>mV"  in cyan
 *   "ST_Vy: <st_vy>mV"  in cyan
 *   "ST_Vz: <st_vz>mV"  in cyan
 * Stays on screen for 2 seconds then returns.
 *
 * Parameters:
 *   st_vx — self-test X-axis output voltage in millivolts
 *   st_vy — self-test Y-axis output voltage in millivolts
 *   st_vz — self-test Z-axis output voltage in millivolts
 * Returns: void
 *
 * [HOOK - Self-Test Engineer]: Pass your real ADC-converted voltages here.
 *   Assert the ADXL335 ST pin HIGH, wait for settling, sample ADC,
 *   convert to mV, then call:
 *     Display_ShowSelfTestST(real_st_vx, real_st_vy, real_st_vz);
 * ───────────────────────────────────────── */
void Display_ShowSelfTestST(float st_vx, float st_vy, float st_vz)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(30,  "Reading ST",      Font_11x18, WHITE);
    Draw_CentredString(55,  "Axis Outputs...", Font_11x18, WHITE);

    char buf[32];

    sprintf(buf, "ST_Vx: %.0fmV", st_vx);
    Draw_CentredString(110, buf, Font_11x18, CYAN);

    sprintf(buf, "ST_Vy: %.0fmV", st_vy);
    Draw_CentredString(135, buf, Font_11x18, CYAN);

    sprintf(buf, "ST_Vz: %.0fmV", st_vz);
    Draw_CentredString(160, buf, Font_11x18, CYAN);

    HAL_Delay(2000);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestComparison
 *
 * Shows a summary comparison table:
 *   "Comparing Against Delta Values:"
 *   Row 1 (yellow): baseline values   "Vx:NNN Vy:NNN Vz:NNN"
 *   Row 2 (cyan):   self-test values  "ST:NNN ST:NNN ST:NNN"
 *   Horizontal divider line
 *   "Expected Deltas:" in white
 *   "X:-325mV"  in green
 *   "Y:+325mV"  in green
 *   "Z:+550mV"  in green
 * Stays on screen for 3 seconds then returns.
 *
 * Parameters:
 *   vx    — baseline X voltage (mV)
 *   vy    — baseline Y voltage (mV)
 *   vz    — baseline Z voltage (mV)
 *   st_vx — self-test X voltage (mV)
 *   st_vy — self-test Y voltage (mV)
 *   st_vz — self-test Z voltage (mV)
 * Returns: void
 * ───────────────────────────────────────── */
void Display_ShowSelfTestComparison(float vx,    float vy,    float vz,
                                    float st_vx, float st_vy, float st_vz)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(10, "Comparing Against", Font_11x18, WHITE);
    Draw_CentredString(32, "Delta Values:",     Font_11x18, WHITE);

    char buf[32];

    // Baseline row
    sprintf(buf, "Vx:%.0f Vy:%.0f Vz:%.0f", vx, vy, vz);
    Draw_CentredString(70, buf, Font_7x10, YELLOW);

    // ST row
    sprintf(buf, "ST:%.0f ST:%.0f ST:%.0f", st_vx, st_vy, st_vz);
    Draw_CentredString(90, buf, Font_7x10, CYAN);

    // Divider
    ST7789_DrawLine(10, 108, 230, 108, GRAY);

    // Expected delta row
    Draw_CentredString(115, "Expected Deltas:", Font_7x10, WHITE);
    Draw_CentredString(130, "X:-325mV", Font_7x10, GREEN);
    Draw_CentredString(145, "Y:+325mV", Font_7x10, GREEN);
    Draw_CentredString(160, "Z:+550mV", Font_7x10, GREEN);

    HAL_Delay(3000);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestDetermining
 *
 * Shows a suspense/transition screen:
 *   "Delta Within"
 *   "Range?"        in white
 *   "Determining..."  in yellow (small Font_7x10)
 * Blocks for 2.5 seconds then returns.  Called just before the
 * pass/fail check so the user knows the result is being computed.
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Display_ShowSelfTestDetermining(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(90,  "Delta Within", Font_11x18, WHITE);
    Draw_CentredString(115, "Range?",       Font_11x18, WHITE);
    Draw_CentredString(155, "Determining...", Font_7x10, YELLOW);

    // Suspense pause
    HAL_Delay(2500);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestResult
 *
 * Shows the final pass/fail result screen:
 *   "Self-Test:"     in white
 *   "PASSED!"        in green (if passed != 0)
 *   "FAILED!"        in red   (if passed == 0)
 * Uses Font_16x26 for the result so it is clearly visible.
 * Blocks for 2 seconds then returns.
 *
 * Parameters:
 *   passed — non-zero = test passed, 0 = test failed
 * Returns: void
 * ───────────────────────────────────────── */
void Display_ShowSelfTestResult(uint8_t passed)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(80, "Self-Test:", Font_11x18, WHITE);

    if (passed)
        Draw_CentredString(115, "PASSED!", Font_16x26, GREEN);
    else
        Draw_CentredString(115, "FAILED!", Font_16x26, RED);

    HAL_Delay(2000);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowSelfTestProceed
 *
 * Shows the post-result prompt and waits for a button press:
 *   If passed:  "Press Button / To Start / Step Tracker!" in green
 *   If failed:  "Press Button / To Restart!"              in red
 *
 * After the user presses the button, Stage_SelfTest_Run() either
 * continues to the step tracker (on pass) or triggers NVIC_SystemReset
 * (on fail) to restart the device from the beginning.
 *
 * Parameters:
 *   passed — non-zero = test passed, 0 = test failed
 * Returns: void
 * ───────────────────────────────────────── */
void Display_ShowSelfTestProceed(uint8_t passed)
{
    ST7789_Fill_Color(BLACK);

    if (passed)
    {
        Draw_CentredString(80,  "Press Button",          Font_11x18, WHITE);
        Draw_CentredString(105, "To Start",              Font_11x18, WHITE);
        Draw_CentredString(130, "Step Tracker!",         Font_11x18, GREEN);
    }
    else
    {
        Draw_CentredString(80,  "Press Button",          Font_11x18, WHITE);
        Draw_CentredString(105, "To Restart!",           Font_11x18, RED);
    }
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Stage_SelfTest_Run
 *
 * Main entry point for Stage 2.  Called from main.c.
 * Runs the complete self-test sequence and returns SELFTEST_PASSED (1)
 * to main.c on success.  On failure, calls NVIC_SystemReset() which
 * reboots the entire MCU — execution never returns to the caller.
 *
 * RETURN VALUES (defined in stage_selftest.h):
 *   SELFTEST_PASSED = 1   — test passed, proceed to Stage 3
 *   SELFTEST_FAILED = 0   — (never actually returned; device resets instead)
 *
 * Parameters: none
 * Returns:    uint8_t — SELFTEST_PASSED on success; no return on failure
 * ───────────────────────────────────────── */
uint8_t Stage_SelfTest_Run(void)
{
    /* Note: status bar reads "Calibrating..." here; this is intentional —
     * it signals to the user that a startup check is in progress. */
    Display_SetStatus("Calibrating...");
    HAL_Delay(1500);
    Display_ClearStatus();

    /* --- Idle screen: prompt user to start the self-test --- */
    Display_ShowSelfTestIdle();
    SelfTest_WaitForButton();

    /* --- Acquire accelerometer readings ---
     * [PLACEHOLDER - Self-Test Engineer]: Replace ALL six variables below
     *   with real ADC readings from your ADXL335 interface.
     *   Do NOT hardcode these values in production firmware.
     *
     *   Baseline readings: sample with ADXL335 self-test pin LOW (inactive).
     *   ST readings:       assert self-test pin HIGH, wait for output to settle
     *                      (~1 ms per datasheet), then sample.
     *   Convert ADC counts to millivolts using your reference voltage and
     *   ADC resolution before assigning to these variables.
     *
     *   Create your own .c file and do the ADC work there.
     *   Include "stage_selftest.h" and call Stage_SelfTest_Run() from main.c
     *   (or pass the values in through your own wrapper). */
    float vx    = 1650.0f;   /* [PLACEHOLDER - Self-Test Engineer]: Real baseline Vx (mV) */
    float vy    = 1650.0f;   /* [PLACEHOLDER - Self-Test Engineer]: Real baseline Vy (mV) */
    float vz    = 1650.0f;   /* [PLACEHOLDER - Self-Test Engineer]: Real baseline Vz (mV) */
    float st_vx = 1325.0f;   /* [PLACEHOLDER - Self-Test Engineer]: Real self-test ST_Vx (mV). Expected ≈ vx - 325 */
    float st_vy = 1975.0f;   /* [PLACEHOLDER - Self-Test Engineer]: Real self-test ST_Vy (mV). Expected ≈ vy + 325 */
    float st_vz = 2200.0f;   /* [PLACEHOLDER - Self-Test Engineer]: Real self-test ST_Vz (mV). Expected ≈ vz + 550 */

    /* --- Show each reading set on the LCD --- */
    Display_ShowSelfTestBaseline(vx, vy, vz);              /* Yellow baseline values, 2s */
    Display_ShowSelfTestST(st_vx, st_vy, st_vz);          /* Cyan ST values, 2s */
    Display_ShowSelfTestComparison(vx, vy, vz, st_vx, st_vy, st_vz); /* Table + expected deltas, 3s */
    Display_ShowSelfTestDetermining();                      /* "Determining..." suspense, 2.5s */

    /* --- Compute axis deltas and determine pass/fail ---
     * delta = (ST reading) - (baseline reading).
     * Per ADXL335 datasheet, the expected deltas when self-test is asserted are:
     *   X: -325 mV  (the electrostatic actuator pulls the X proof mass)
     *   Y: +325 mV
     *   Z: +550 mV
     * A ±100 mV tolerance is applied to each axis.
     *
     * [PLACEHOLDER - Self-Test Engineer]: Replace this entire delta check
     *   block with your own validated pass/fail logic if your hardware
     *   produces different nominal delta values (e.g. due to supply voltage
     *   differences). Coordinate with the LCD Engineer before changing
     *   the tolerance constants. */
    float delta_x = st_vx - vx;
    float delta_y = st_vy - vy;
    float delta_z = st_vz - vz;

    /* Tolerance of ±100 mV around each expected delta value:
     *   X: must be in [-425, -225] mV  (target -325 mV)
     *   Y: must be in [+225, +425] mV  (target +325 mV)
     *   Z: must be in [+450, +650] mV  (target +550 mV) */
    uint8_t passed =
        (delta_x >= -425.0f && delta_x <= -225.0f) &&
        (delta_y >=  225.0f && delta_y <=  425.0f) &&
        (delta_z >=  450.0f && delta_z <=  650.0f);

    /* --- Show result and get user acknowledgement --- */
    Display_ShowSelfTestResult(passed);   /* "PASSED!" green or "FAILED!" red, 2s */
    Display_ShowSelfTestProceed(passed);  /* "Press Button To Start / Restart" prompt */
    SelfTest_WaitForButton();             /* Block until user presses the button */

    if (!passed)
    {
        /* Self-test failed — show "Resetting..." then perform a full hardware
         * reset.  NVIC_SystemReset() triggers a SYSRESETREQ which reboots the
         * MCU — execution restarts from the beginning of main() → Stage 1. */
        Display_SetStatus("Resetting...");
        HAL_Delay(1500);
        Display_ClearStatus();
        NVIC_SystemReset();  /* Full chip reset — does NOT return */
    }

    /* Self-test passed — show "Ready" briefly and return to main.c */
    Display_SetStatus("Ready");
    HAL_Delay(1500);
    Display_ClearStatus();

    return SELFTEST_PASSED;
}
