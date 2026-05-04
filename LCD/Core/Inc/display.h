/*
 * display.h
 *
 *  Created on: May 4, 2026
 *      Author: Ibrahim Munir Ali
 */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "st7789.h"
#include <string.h>
#include <stdio.h>

void Display_Init(void);
void Display_UpdateSteps(uint32_t steps);
void Display_UpdatePace(uint8_t pace);
void Display_SetStatus(const char *msg);
void Display_ClearStatus(void);

// Pace states - share these with Marcus
#define PACE_STATIONARY 0
#define PACE_WALKING    1
#define PACE_RUNNING    2

// Pace thresholds — easy to adjust
#define SPM_STATIONARY_MAX 40.0f
#define SPM_WALKING_MAX 130.0f


#endif
