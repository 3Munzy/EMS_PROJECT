/*
 * display.c
 *
 *  Created on: May 4, 2026
 *      Author: Ibrahim Munir Ali
 */

#include "display.h"
#include <string.h>

// Layout constants - all Y positions fixed regardless of status bar
#define STATUS_Y        20
#define STATUS_HEIGHT   30
#define STEPS_LABEL_Y   80
#define STEPS_VALUE_Y   150
#define PACE_LABEL_Y    170
#define PACE_BADGE_Y    200
#define PACE_TEXT_Y     220

// Tracks last values so we only redraw what changed
static uint32_t last_steps = 9999999;
static uint8_t  last_pace  = 255;
static char     last_status[32] = "";

void Display_Init(void)
{
    ST7789_Fill_Color(BLACK);

    // Draw static labels
    ST7789_WriteString(92, STEPS_LABEL_Y, "STEPS", Font_11x18, GRAY, BLACK);
    ST7789_WriteString(98, PACE_LABEL_Y,  "PACE",  Font_11x18, GRAY, BLACK);

}

void Display_SetStatus(const char *msg)
{
    if (strcmp(msg, last_status) == 0) return;

    // Clear status bar area
    ST7789_Fill(0, STATUS_Y, 239, STATUS_Y + STATUS_HEIGHT, BLACK);

    if (strlen(msg) > 0)
    {
        uint8_t num_chars = strlen(msg);
        uint16_t x = (240 - (num_chars * Font_11x18.width)) / 2;
        ST7789_WriteString(x, STATUS_Y + 8, msg, Font_11x18, YELLOW, BLACK);
    }

    strncpy(last_status, msg, sizeof(last_status) - 1);
}

void Display_ClearStatus(void)
{
    Display_SetStatus("");
}

void Display_UpdateSteps(uint32_t steps)
{
    if (steps == last_steps) return;

    char buf[16];
    sprintf(buf, "%lu", steps);  // no padding, just the number

    // Calculate centred X position dynamically
    uint8_t num_chars = strlen(buf);
    uint16_t x = (240 - (num_chars * Font_16x26.width)) / 2;

    // Clear old value first
    ST7789_Fill(0, STEPS_VALUE_Y - 40, 239, STEPS_VALUE_Y + 5, BLACK);

    ST7789_WriteString(x, STEPS_VALUE_Y - 40, buf, Font_16x26, WHITE, BLACK);

    last_steps = steps;
}

void Display_UpdatePace(uint8_t pace)
{
    if (pace == last_pace) return;

    // Clear badge area
    ST7789_Fill(0, PACE_BADGE_Y, 239, PACE_BADGE_Y + 28, BLACK);

    const char *pace_str;
    uint16_t badge_color, text_color;

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
            return;
    }

    // Calculate badge width and centred X
    uint8_t num_chars = strlen(pace_str);
    uint16_t text_width = num_chars * Font_11x18.width;
    uint16_t badge_x    = (240 - text_width - 20) / 2;  // 20px padding inside badge
    uint16_t text_x     = (240 - text_width) / 2;

    // Draw badge then text
    ST7789_Fill(badge_x, PACE_BADGE_Y, badge_x + text_width + 20, PACE_BADGE_Y + 28, badge_color);
    ST7789_WriteString(text_x, PACE_BADGE_Y + 5, pace_str, Font_11x18, text_color, badge_color);

    last_pace = pace;
}


