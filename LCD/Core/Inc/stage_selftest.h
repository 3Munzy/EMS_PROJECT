#ifndef __STAGE_SELFTEST_H
#define __STAGE_SELFTEST_H

#include "display.h"
#include "main.h"

// Button definition — swap pin when real button is wired
#define BTN_SELFTEST_PORT   GPIOC
#define BTN_SELFTEST_PIN    GPIO_PIN_13

// Pass/Fail defines — Joshua uses these
#define SELFTEST_PASSED     1
#define SELFTEST_FAILED     0

// Functions Joshua calls
void Display_ShowSelfTestIdle(void);
void Display_ShowSelfTestBaseline(float vx, float vy, float vz);
void Display_ShowSelfTestST(float st_vx, float st_vy, float st_vz);
void Display_ShowSelfTestComparison(float vx,    float vy,    float vz,
                                    float st_vx, float st_vy, float st_vz);
void Display_ShowSelfTestDetermining(void);
void Display_ShowSelfTestResult(uint8_t passed);
void Display_ShowSelfTestProceed(uint8_t passed);

// Main entry point — call from main.c
// Returns SELFTEST_PASSED or SELFTEST_FAILED
uint8_t Stage_SelfTest_Run(void);

// Button helper
void SelfTest_WaitForButton(void);

#endif