#include "stage_selftest.h"
#include "st7789.h"
#include "fonts.h"
#include <string.h>
#include <stdio.h>

// ─────────────────────────────────────────
// Internal helper — centres a string
// ─────────────────────────────────────────
static void Draw_CentredString(uint16_t y, const char *str,
                                FontDef font, uint16_t color)
{
    uint16_t x = (240 - (strlen(str) * font.width)) / 2;
    ST7789_WriteString(x, y, str, font, color, BLACK);
}

// ─────────────────────────────────────────
// Wait for button press then release
// ─────────────────────────────────────────
void SelfTest_WaitForButton(void)
{
    while (HAL_GPIO_ReadPin(BTN_SELFTEST_PORT, BTN_SELFTEST_PIN)
           == GPIO_PIN_SET);
    HAL_Delay(50);
    while (HAL_GPIO_ReadPin(BTN_SELFTEST_PORT, BTN_SELFTEST_PIN)
           == GPIO_PIN_RESET);
    HAL_Delay(50);
}

// ─────────────────────────────────────────
// Screen: "Begin Self-Test / Press Button To Start"
// ─────────────────────────────────────────
void Display_ShowSelfTestIdle(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(70,  "Begin Self-Test", Font_11x18, WHITE);
    Draw_CentredString(95,  "Routine!",        Font_11x18, WHITE);
    Draw_CentredString(135, "Press Button",    Font_11x18, WHITE);
    Draw_CentredString(160, "To Start!",       Font_11x18, WHITE);
}

// ─────────────────────────────────────────
// Screen: Baseline readings
// Joshua calls: Display_ShowSelfTestBaseline(vx, vy, vz)
// ─────────────────────────────────────────
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

// ─────────────────────────────────────────
// Screen: ST readings
// Joshua calls: Display_ShowSelfTestST(st_vx, st_vy, st_vz)
// ─────────────────────────────────────────
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

// ─────────────────────────────────────────
// Screen: Comparison table
// ─────────────────────────────────────────
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

// ─────────────────────────────────────────
// Screen: "Delta Within Range? Determining..."
// ─────────────────────────────────────────
void Display_ShowSelfTestDetermining(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(90,  "Delta Within", Font_11x18, WHITE);
    Draw_CentredString(115, "Range?",       Font_11x18, WHITE);
    Draw_CentredString(155, "Determining...", Font_7x10, YELLOW);

    // Suspense pause
    HAL_Delay(2500);
}

// ─────────────────────────────────────────
// Screen: PASSED or FAILED
// ─────────────────────────────────────────
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

// ─────────────────────────────────────────
// Screen: Proceed prompt
// ─────────────────────────────────────────
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

// ─────────────────────────────────────────
// Main Stage 2 entry point
// Returns SELFTEST_PASSED or SELFTEST_FAILED
// ─────────────────────────────────────────
uint8_t Stage_SelfTest_Run(void)
{
    Display_SetStatus("Calibrating...");
    HAL_Delay(1500);
    Display_ClearStatus();

    // --- Idle screen ---
    Display_ShowSelfTestIdle();
    SelfTest_WaitForButton();

    // --- Dummy values for testing ---
    // Joshua replaces these with real ADC readings
    float vx    = 1650.0f;
    float vy    = 1650.0f;
    float vz    = 1650.0f;
    float st_vx = 1325.0f;   // vx - 325
    float st_vy = 1975.0f;   // vy + 325
    float st_vz = 2200.0f;   // vz + 550

    // --- Baseline screen ---
    Display_ShowSelfTestBaseline(vx, vy, vz);

    // --- ST screen ---
    Display_ShowSelfTestST(st_vx, st_vy, st_vz);

    // --- Comparison screen ---
    Display_ShowSelfTestComparison(vx, vy, vz, st_vx, st_vy, st_vz);

    // --- Determining screen ---
    Display_ShowSelfTestDetermining();

    // --- Check deltas ---
    // Joshua replaces this logic with his real pass/fail check
    float delta_x = st_vx - vx;
    float delta_y = st_vy - vy;
    float delta_z = st_vz - vz;

    // Tolerance of ±100mV around expected values
    uint8_t passed =
        (delta_x >= -425.0f && delta_x <= -225.0f) &&
        (delta_y >=  225.0f && delta_y <=  425.0f) &&
        (delta_z >=  450.0f && delta_z <=  650.0f);

    // --- Result screen ---
    Display_ShowSelfTestResult(passed);

    // --- Proceed prompt ---
    Display_ShowSelfTestProceed(passed);
    SelfTest_WaitForButton();

    if (!passed)
    {
        // Failed — restart entire device from Stage 1
        Display_SetStatus("Resetting...");
        HAL_Delay(1500);
        Display_ClearStatus();
        NVIC_SystemReset();  // hardware reset — restarts from main()
    }

    // Passed
    Display_SetStatus("Ready");
    HAL_Delay(1500);
    Display_ClearStatus();

    return SELFTEST_PASSED;
}
