/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 * Other team members must NOT edit this file directly.
 *
 * FILE: stage_calibration.c
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Implements Stage 1 of the startup sequence: static accelerometer
 *   calibration.  The stage guides the user through holding the device
 *   in 6 orientations (+X, -X, +Y, -Y, +Z, -Z) and pressing the button
 *   at each position so the Static Calibration Engineer's ADC code can
 *   sample the raw ADXL335 output for that axis.
 *
 * STAGE 1 FLOW:
 *   1. Show idle screen ("Press Button To Calibrate") → wait for button
 *   2. Show intro screen ("6-Step Routine / Press Button To Begin")
 *      → wait up to 10 seconds.  If no press, restart from step 1.
 *   3. For each of the 6 axis directions:
 *        a. Show directional prompt (e.g. "+X UP")
 *        b. Wait for button press
 *        c. [HOOK] Static Calibration Engineer's ADC sample runs here
 *   4. Show "Calibration Complete!" in green for 2 seconds
 *   5. Show status "Ready" briefly, then hand control back to main.c
 *
 * HOW THE STATIC CALIBRATION ENGINEER INTEGRATES:
 *   - Create your own .c/.h files for your ADC / calibration logic.
 *   - Do NOT modify this file.
 *   - Inside your own .c file, implement the body of Stage_Calibration_Run()
 *     by calling Display_ShowCalibrateStep(direction) and then sampling
 *     the ADC.  See the HOOK comment at line ~141 for the exact location.
 *   - Include "stage_calibration.h" in your own .c file to access the
 *     display functions declared there.
 * ============================================================
 */

#include "stage_calibration.h"
#include "st7789.h"
#include "fonts.h"
#include <string.h>
#include <stdio.h>

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Draw_CentredString
 *
 * Internal helper. Draws a single text string centred horizontally on
 * the 240px-wide display at a given Y coordinate.
 *
 * Centering formula:
 *   x = (display_width - (char_count × font_char_width)) / 2
 *
 * Parameters:
 *   y     — vertical pixel row to start drawing at
 *   str   — null-terminated string to render
 *   font  — FontDef struct (e.g. Font_11x18, Font_16x26, Font_7x10)
 *   color — foreground colour in RGB565 format (background always BLACK)
 * Returns: void
 * ───────────────────────────────────────── */
static void Draw_CentredString(uint16_t y, const char *str,
                                FontDef font, uint16_t color)
{
    uint16_t x = (240 - (strlen(str) * font.width)) / 2;
    ST7789_WriteString(x, y, str, font, color, BLACK);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Calibration_WaitForButton
 *
 * Blocks until the user presses and then fully releases the calibration
 * button (GPIOC pin 13).  50 ms debounce delays are applied after both
 * the press and the release edges to filter contact bounce.
 *
 * Button convention on this board:
 *   GPIO_PIN_RESET = button not pressed (pulled low at rest)
 *   GPIO_PIN_SET   = button pressed     (pulled high when activated)
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Calibration_WaitForButton(void)
{
    /* Wait until the button is pressed (pin transitions RESET → SET) */
    while (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
           == GPIO_PIN_RESET);

    HAL_Delay(50);  /* Debounce — ignore bounce on the press edge */

    /* Wait until the button is fully released (pin transitions SET → RESET) */
    while (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
           == GPIO_PIN_SET);

    HAL_Delay(50);  /* Debounce — ignore bounce on the release edge */
}


/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowCalibrateIdle
 *
 * Clears the screen and shows the idle prompt:
 *   "Press Button"
 *   "To Calibrate"
 * in white.  Displayed at startup and after a 10-second intro timeout.
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Display_ShowCalibrateIdle(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(90,  "Press Button",  Font_11x18, WHITE);
    Draw_CentredString(115, "To Calibrate",  Font_11x18, WHITE);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowCalibrateIntro
 *
 * Clears the screen and shows the calibration intro screen:
 *   "6-Step Routine"
 *   "Press Button"
 *   "To Begin"
 * in white.  Displayed after the user presses the button on the idle screen.
 * The Stage_Calibration_Run() state machine gives the user 10 seconds to
 * press the button again before timing out and returning to idle.
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Display_ShowCalibrateIntro(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(70,  "6-Step Routine",     Font_11x18, WHITE);
    Draw_CentredString(100, "Press Button",        Font_11x18, WHITE);
    Draw_CentredString(125, "To Begin",            Font_11x18, WHITE);
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowCalibrateStep
 *
 * Clears the screen and shows a single calibration step directional prompt:
 *   <direction>     ← large yellow text (Font_16x26), e.g. "+X UP"
 *   "Press Button!" ← smaller white text (Font_11x18)
 *
 * Called once per axis step inside the loop in Stage_Calibration_Run().
 * The direction string is defined in the steps[] array (see below).
 *
 * Parameters:
 *   direction — null-terminated string describing the axis and orientation,
 *               e.g. "+X UP", "-X DOWN", "+Y RIGHT", "-Y LEFT",
 *                    "+Z FORWARD", "-Z BACKWARD"
 * Returns: void
 *
 * [HOOK - Static Calibration Engineer]: After calling this function,
 *   Stage_Calibration_Run() calls Calibration_WaitForButton() and then
 *   your ADC sampling code runs. See the HOOK comment in Stage_Calibration_Run().
 * ───────────────────────────────────────── */
void Display_ShowCalibrateStep(const char *direction)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(80,  direction,       Font_16x26, YELLOW);  /* Large yellow axis label */
    Draw_CentredString(130, "Press Button!", Font_11x18, WHITE);   /* User instruction */
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Display_ShowCalibrateComplete
 *
 * Clears the screen and shows the success screen:
 *   "Calibration"
 *   "Complete!"
 * in green, then blocks for 2 seconds so the user can read it before
 * Stage_Calibration_Run() continues to show "Ready" and return.
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Display_ShowCalibrateComplete(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(100, "Calibration", Font_11x18, GREEN);
    Draw_CentredString(125, "Complete!",   Font_11x18, GREEN);
    HAL_Delay(2000);  /* Hold screen for 2 seconds before moving on */
}

/* ─────────────────────────────────────────
 * [LCD - DO NOT MODIFY]: Stage_Calibration_Run
 *
 * Main entry point for Stage 1.  Called from main.c.  Blocks until the
 * full 6-axis calibration sequence is complete, then returns to main.c
 * so Stage 2 (Stage_SelfTest_Run) can begin.
 *
 * STATE MACHINE:
 *   1. Status bar → "Calibrating..." for 1.5s
 *   2. Idle screen → wait for button press
 *   3. Intro screen → wait up to 10s for button press
 *      - If 10s elapses with no press: call Stage_Calibration_Run()
 *        recursively to restart from the idle screen.
 *   4. Loop through the 6 axis steps (steps[] array):
 *      a. Display_ShowCalibrateStep(direction) — show prompt
 *      b. Calibration_WaitForButton()          — wait for press
 *      c. [HOOK] Static Calibration Engineer ADC sample
 *   5. Display_ShowCalibrateComplete() — green success screen 2s
 *   6. Status bar → "Ready" for 1.5s, then clear
 *
 * Parameters: none
 * Returns:    void
 * ───────────────────────────────────────── */
void Stage_Calibration_Run(void)
{
    /* --- Status bar --- */
    Display_SetStatus("Calibrating...");  /* Yellow status message at top of screen */
    HAL_Delay(1500);
    Display_ClearStatus();

    /* --- Idle screen: wait for the user to initiate calibration --- */
    Display_ShowCalibrateIdle();
    Calibration_WaitForButton();

    /* --- Intro screen: 10-second window to confirm start --- */
    Display_ShowCalibrateIntro();

    uint32_t intro_start = HAL_GetTick();  /* Capture timestamp to measure elapsed time */
    uint8_t  pressed = 0;

    /* Poll button for up to 10 seconds */
    while ((HAL_GetTick() - intro_start) < 10000)
    {
        if (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
        == GPIO_PIN_SET)

        {
            HAL_Delay(50);  /* Debounce */
            pressed = 1;
            break;
        }
    }

    if (!pressed)
    {
        /* 10-second timeout expired — restart the entire stage from idle.
         * Recursive call is safe: stack depth is bounded by user interaction. */
        Stage_Calibration_Run();
        return;
    }

    /* Wait for the button to be released before starting the step loop */
    while (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
           == GPIO_PIN_RESET);
    HAL_Delay(50);  /* Debounce release edge */

    /* --- 6 directional calibration steps ---
     * Each string is the label shown on the LCD during that step.
     * [HOOK - Static Calibration Engineer]: You may reorder these strings
     *   to match the order your ADC sampling algorithm expects, but do so
     *   by coordinating with the LCD Engineer — do NOT modify this file
     *   directly.  Create your own .c file with your ADC logic instead. */
    const char *steps[6] = {
        "+X UP",
        "-X DOWN",
        "+Y RIGHT",
        "-Y LEFT",
        "+Z FORWARD",
        "-Z BACKWARD"
    };

    for (uint8_t i = 0; i < 6; i++)
    {
        Display_ShowCalibrateStep(steps[i]);  /* Show directional prompt for this axis */
        Calibration_WaitForButton();          /* Block until user confirms alignment */

        /* [HOOK - Static Calibration Engineer]: Insert your ADC sampling call here.
         * At this point the device is being held in the orientation shown on screen.
         * Example of what your code should do:
         *
         *   MyCalibration_SampleAxis(i);
         *
         * Where i = 0 → +X, 1 → -X, 2 → +Y, 3 → -Y, 4 → +Z, 5 → -Z.
         * Do NOT write this code in this file. Create your own .c file,
         * include "stage_calibration.h", and implement your sampling function there.
         * Then call Stage_Calibration_Run() from your code (or have main.c do it). */
    }

    /* --- Completion screens --- */
    Display_ShowCalibrateComplete();     /* Green "Calibration Complete!" for 2s */
    Display_SetStatus("Ready");          /* Yellow status bar confirmation */
    HAL_Delay(1500);
    Display_ClearStatus();
}