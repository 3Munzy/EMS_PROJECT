/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 *
 * FILE: stage_selftest.h
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Public interface for Stage 2: ADXL335 accelerometer self-test.
 *   This header is the READ-ONLY CONTRACT between the LCD Engineer
 *   and the Self-Test Engineer.
 *
 * HOW THE SELF-TEST ENGINEER USES THIS FILE:
 *   1. #include "stage_selftest.h" in your own .c file.
 *   2. Do NOT call Stage_SelfTest_Run() from your code — main.c calls it.
 *   3. Your job is to replace the six dummy float variables (vx, vy, vz,
 *      st_vx, st_vy, st_vz) and the delta pass/fail check in
 *      stage_selftest.c with your real ADC readings and logic.
 *      Coordinate with the LCD Engineer to insert your code at the
 *      marked PLACEHOLDER locations.
 *   4. Never modify this header or any other LCD file directly.
 *
 * EXPECTED ADXL335 SELF-TEST DELTAS:
 *   X axis: ST_Vx - Vx ≈ -325 mV  (±100 mV tolerance)
 *   Y axis: ST_Vy - Vy ≈ +325 mV  (±100 mV tolerance)
 *   Z axis: ST_Vz - Vz ≈ +550 mV  (±100 mV tolerance)
 *
 * EXAMPLE SKELETON (.c file the Self-Test Engineer writes):
 *
 *   // my_selftest.c
 *   #include "stage_selftest.h"  // include the LCD interface
 *   #include "my_adc.h"          // your own ADC code
 *
 *   // Called by Stage_SelfTest_Run() after button press
 *   void MySelfTest_GetReadings(float *vx, float *vy, float *vz,
 *                                float *st_vx, float *st_vy, float *st_vz) {
 *       // Assert self-test pin LOW, sample ADC, convert to mV
 *       *vx = ADC_ToMillivolts(ADC_Read(ADC_X));
 *       // ... etc.
 *       // Assert self-test pin HIGH, wait, sample again
 *       *st_vx = ADC_ToMillivolts(ADC_Read(ADC_X));
 *       // ... etc.
 *   }
 *   // Then coordinate with LCD Engineer to call this in stage_selftest.c
 * ============================================================
 */

#ifndef __STAGE_SELFTEST_H
#define __STAGE_SELFTEST_H

#include "display.h"
#include "main.h"

/* Button GPIO definition.
 * Same physical button as calibration stage: GPIOC pin 13 (onboard blue button). */
#define BTN_SELFTEST_PORT   GPIOC
#define BTN_SELFTEST_PIN    GPIO_PIN_13

/* Return value constants for Stage_SelfTest_Run().
 * [LCD - DO NOT MODIFY]: These values are used by main.c to check the result. */
#define SELFTEST_PASSED     1   /* All three axis deltas within tolerance */
#define SELFTEST_FAILED     0   /* One or more deltas out of tolerance (device will reset) */

/* [LCD - DO NOT MODIFY]: Display functions for the self-test stage.
 * These are called in sequence by Stage_SelfTest_Run().
 * The Self-Test Engineer does not call these directly — they are called
 * automatically in the correct order by the LCD stage logic. */

/* Shows "Begin Self-Test Routine! / Press Button To Start!" idle screen */
void Display_ShowSelfTestIdle(void);

/* Shows baseline axis voltage readings on screen for 2 seconds.
 *   vx, vy, vz — baseline voltages in millivolts (ADXL335 self-test pin LOW) */
void Display_ShowSelfTestBaseline(float vx, float vy, float vz);

/* Shows self-test mode axis voltage readings on screen for 2 seconds.
 *   st_vx, st_vy, st_vz — self-test voltages in millivolts (ADXL335 ST pin HIGH) */
void Display_ShowSelfTestST(float st_vx, float st_vy, float st_vz);

/* Shows a comparison table (baseline vs ST vs expected deltas) for 3 seconds.
 * All 6 voltage parameters are displayed side by side with expected delta values. */
void Display_ShowSelfTestComparison(float vx,    float vy,    float vz,
                                    float st_vx, float st_vy, float st_vz);

/* Shows "Delta Within Range? Determining..." suspense screen for 2.5 seconds */
void Display_ShowSelfTestDetermining(void);

/* Shows the final result: "PASSED!" in green or "FAILED!" in red, for 2 seconds.
 *   passed — non-zero = passed, 0 = failed */
void Display_ShowSelfTestResult(uint8_t passed);

/* Shows "Press Button To Start Step Tracker!" (pass) or "Press Button To Restart!" (fail).
 *   passed — non-zero = passed, 0 = failed */
void Display_ShowSelfTestProceed(uint8_t passed);

/* Main entry point — called by main.c.
 * Runs the full self-test sequence.  Returns SELFTEST_PASSED on success.
 * On failure calls NVIC_SystemReset() — does not return.
 * [PLACEHOLDER - Self-Test Engineer]: Replace dummy floats and pass/fail
 *   logic inside stage_selftest.c with real ADC readings. */
uint8_t Stage_SelfTest_Run(void);

/* Blocks until button is pressed then released.
 * Note: Wait logic is inverted compared to Calibration_WaitForButton. */
void SelfTest_WaitForButton(void);

#endif