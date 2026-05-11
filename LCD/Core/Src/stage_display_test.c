#include "stage_display_test.h"

void Stage_DisplayTest_Run(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        Display_UpdatePace(PACE_STATIONARY);
        HAL_Delay(3000);

        Display_UpdatePace(PACE_WALKING);
        HAL_Delay(3000);

        Display_UpdatePace(PACE_RUNNING);
        HAL_Delay(3000);
    }
}
