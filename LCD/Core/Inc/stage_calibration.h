/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 *
 * FILE: stage_calibration.h
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Public interface for Stage 1: static accelerometer calibration.
 *   This header is the READ-ONLY CONTRACT between the LCD Engineer
 *   and the Static Calibration Engineer.
 *
 * HOW THE STATIC CALIBRATION ENGINEER USES THIS FILE:
 *   1. #include "stage_calibration.h" in your own .c file.
 *   2. Do NOT call Stage_Calibration_Run() from your code — main.c
 *      already calls it.  Your job is to implement the ADC sampling
 *      that runs inside the hook point in stage_calibration.c.
 *   3. To hook in your ADC sampling, you need to coordinate with the
 *      LCD Engineer to insert your sampling call at the marked HOOK
 *      location in stage_calibration.c (after each button press).
 *   4. Never modify this header or any other LCD file directly.
 *
 * EXAMPLE SKELETON (.c file the Static Calibration Engineer writes):
 *
 *   // my_calibration.c
 *   #include "stage_calibration.h"  // include the LCD interface
 *   #include "my_adc_driver.h"      // your own ADC code
 *
 *   void MyCalibration_SampleAxis(uint8_t axis_index) {
 *       // axis_index: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z
 *       uint16_t raw = ADC_Read();
 *       calibration_offsets[axis_index] = raw;
 *   }
 *   // Then coordinate with LCD Engineer to call this inside stage_calibration.c
 * ============================================================
 */

#ifndef __STAGE_CALIBRATION_H
#define __STAGE_CALIBRATION_H

#include "display.h"
#include "main.h"

/* Button GPIO definition.
 * BTN_CALIBRATE_PORT / BTN_CALIBRATE_PIN map to GPIOC pin 13 — the onboard
 * blue push-button on the NUCLEO board.  Active HIGH on this board. */
#define BTN_CALIBRATE_PORT  GPIOC
#define BTN_CALIBRATE_PIN   GPIO_PIN_13

/* [LCD - DO NOT MODIFY]: Display functions for the calibration stage.
 * These are called internally by Stage_Calibration_Run().
 * The Static Calibration Engineer does not call these directly — the
 * LCD Engineer's stage logic calls them in the correct sequence. */

/* Shows "Press Button / To Calibrate" idle screen */
void Display_ShowCalibrateIdle(void);

/* Shows "6-Step Routine / Press Button / To Begin" intro screen */
void Display_ShowCalibrateIntro(void);

/* Shows directional prompt for one calibration step.
 *   direction — axis label string, e.g. "+X UP", "-Y LEFT", "+Z FORWARD" */
void Display_ShowCalibrateStep(const char *direction);

/* Shows "Calibration Complete!" success screen for 2 seconds */
void Display_ShowCalibrateComplete(void);

/* Main entry point — called by main.c.
 * Runs the full 6-step calibration sequence. Blocks until complete.
 * [HOOK - Static Calibration Engineer]: Your ADC sampling runs inside
 *   this function after each button press. See stage_calibration.c. */
void Stage_Calibration_Run(void);

/* Blocks until button is pressed then released (with 50ms debounce).
 * Used internally — not typically called by other engineers directly. */
void Calibration_WaitForButton(void);

#endif