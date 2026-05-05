#ifndef __STAGE_CALIBRATION_H
#define __STAGE_CALIBRATION_H

#include "display.h"
#include "main.h"

// Button definition — swap pin when real button is wired
#define BTN_CALIBRATE_PORT  GPIOC
#define BTN_CALIBRATE_PIN   GPIO_PIN_13

// Function Marcus calls to pass a direction string
// e.g. Display_ShowCalibrateStep("+X UP");
void Display_ShowCalibrateIdle(void);
void Display_ShowCalibrateIntro(void);
void Display_ShowCalibrateStep(const char *direction);
void Display_ShowCalibrateComplete(void);

// Main entry point — call this from main.c
// Blocks until calibration is complete
void Stage_Calibration_Run(void);

// Button helper — waits until button is pressed then released
void Calibration_WaitForButton(void);

#endif