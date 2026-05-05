#include "stage_calibration.h"
#include "st7789.h"
#include "fonts.h"
#include <string.h>
#include <stdio.h>

// ─────────────────────────────────────────
// Internal helper — centres a string on screen
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
void Calibration_WaitForButton(void)
{
    // Wait for press (pin goes HIGH on this board)
    while (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
           == GPIO_PIN_RESET);

    HAL_Delay(50);  // debounce

    // Wait for release
    while (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
           == GPIO_PIN_SET);

    HAL_Delay(50);  // debounce
}


// ─────────────────────────────────────────
// Screen: "Press Button To Calibrate"
// ─────────────────────────────────────────
void Display_ShowCalibrateIdle(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(90,  "Press Button",  Font_11x18, WHITE);
    Draw_CentredString(115, "To Calibrate",  Font_11x18, WHITE);
}

// ─────────────────────────────────────────
// Screen: "6-Step Routine / Press Button To Begin"
// ─────────────────────────────────────────
void Display_ShowCalibrateIntro(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(70,  "6-Step Routine",     Font_11x18, WHITE);
    Draw_CentredString(100, "Press Button",        Font_11x18, WHITE);
    Draw_CentredString(125, "To Begin",            Font_11x18, WHITE);
}

// ─────────────────────────────────────────
// Screen: directional prompt
// Marcus calls: Display_ShowCalibrateStep("+X UP");
// ─────────────────────────────────────────
void Display_ShowCalibrateStep(const char *direction)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(80,  direction,       Font_16x26, YELLOW);
    Draw_CentredString(130, "Press Button!", Font_11x18, WHITE);
}

// ─────────────────────────────────────────
// Screen: "Calibration Complete!"
// ─────────────────────────────────────────
void Display_ShowCalibrateComplete(void)
{
    ST7789_Fill_Color(BLACK);
    Draw_CentredString(100, "Calibration", Font_11x18, GREEN);
    Draw_CentredString(125, "Complete!",   Font_11x18, GREEN);
    HAL_Delay(2000);  // show for 2 seconds before moving on
}

// ─────────────────────────────────────────
// Main Stage 1 entry point
// Called from main.c — blocks until done
// ─────────────────────────────────────────
void Stage_Calibration_Run(void)
{
    // Show status bar
    Display_SetStatus("Calibrating...");
    HAL_Delay(1500);
    Display_ClearStatus();

    // --- Idle screen ---
    Display_ShowCalibrateIdle();
    Calibration_WaitForButton();

    // --- Intro screen ---
    // 10 second idle timeout — if no press, return to idle
    Display_ShowCalibrateIntro();

    uint32_t intro_start = HAL_GetTick();
    uint8_t  pressed = 0;

    while ((HAL_GetTick() - intro_start) < 10000)
    {
        if (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
        == GPIO_PIN_SET)

        {
            HAL_Delay(50);  // debounce
            pressed = 1;
            break;
        }
    }

    if (!pressed)
    {
        // Timed out — go back to idle and restart
        Stage_Calibration_Run();
        return;
    }

    // Wait for release
    while (HAL_GPIO_ReadPin(BTN_CALIBRATE_PORT, BTN_CALIBRATE_PIN)
           == GPIO_PIN_RESET);
    HAL_Delay(50);

    // --- 6 directional steps ---
    // Marcus will replace these strings with his own order if needed
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
        Display_ShowCalibrateStep(steps[i]);
        Calibration_WaitForButton();

        // This is where Marcus hooks in his ADC sampling per step
        // e.g. Marcus_SampleAxis(i);
    }

    // --- Complete ---
    Display_ShowCalibrateComplete();
    Display_SetStatus("Ready");
    HAL_Delay(1500);
    Display_ClearStatus();
}