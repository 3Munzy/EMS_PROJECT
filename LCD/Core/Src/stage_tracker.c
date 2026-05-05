#include "stage_tracker.h"
#include "st7789.h"
#include "display.h"

void Stage_Tracker_Run(void)
{
    // Initialise display
    Display_UpdateSteps(0);
    Display_UpdatePace(PACE_STATIONARY);

    // Step tracking variables
    uint32_t step_count       = 0;
    uint32_t phase_start      = HAL_GetTick();
    uint8_t  phase            = 0;
    uint32_t step_interval_ms = 1000;  // start stationary

    static uint32_t step_times[100];
    static uint8_t  step_count_window = 0;

    while (1)
    {
        uint32_t now           = HAL_GetTick();
        uint32_t phase_elapsed = now - phase_start;

        // --- Phase transitions (simulation) ---
        // Replace this entire block with real accelerometer input
        // when integrating with Joshua's step detection
        if (phase == 0 && phase_elapsed >= PHASE_STATIONARY_MS)
        {
            phase             = 1;
            step_interval_ms  = 400;   // ~150 spm = Walking
            phase_start       = HAL_GetTick();
        }
        else if (phase == 1 && phase_elapsed >= PHASE_WALKING_MS)
        {
            phase             = 2;
            step_interval_ms  = 200;   // ~300 spm = Running
            phase_start       = HAL_GetTick();
        }
        else if (phase == 2 && phase_elapsed >= PHASE_RUNNING_MS)
        {
            phase             = 0;
            step_interval_ms  = 1000;  // ~60 spm = Stationary
            phase_start       = HAL_GetTick();
        }

        HAL_Delay(step_interval_ms);

        // --- Step detected ---
        // Replace HAL_Delay + step_count++ with Joshua's
        // step detection call e.g. if (Joshua_StepDetected())
        step_count++;
        now = HAL_GetTick();

        // --- Rolling window ---
        if (step_count_window < 100)
        {
            step_times[step_count_window++] = now;
        }
        else
        {
            for (uint8_t j = 0; j < 99; j++)
                step_times[j] = step_times[j + 1];
            step_times[99] = now;
        }

        // Discard steps outside window
        uint8_t i = 0;
        while (i < step_count_window)
        {
            if ((now - step_times[i]) > STEP_WINDOW_MS)
            {
                for (uint8_t j = i; j < step_count_window - 1; j++)
                    step_times[j] = step_times[j + 1];
                step_count_window--;
            }
            else i++;
        }

        // --- SPM calculation ---
        float spm = 0.0f;
        if (step_count_window >= 2)
        {
            uint32_t oldest  = step_times[0];
            uint32_t newest  = step_times[step_count_window - 1];
            uint32_t span_ms = newest - oldest;

            if (span_ms > 0)
                spm = (step_count_window - 1) * 60000.0f / span_ms;
        }

        // --- Pace classification ---
        // Thresholds from Marcus — update SPM_STATIONARY_MAX
        // and SPM_WALKING_MAX in stage_tracker.h if they change
        uint8_t pace;
        if (spm < SPM_STATIONARY_MAX)
            pace = PACE_STATIONARY;
        else if (spm <= SPM_WALKING_MAX)
            pace = PACE_WALKING;
        else
            pace = PACE_RUNNING;

        // --- Update display ---
        Display_UpdateSteps(step_count);
        Display_UpdatePace(pace);
    }
}
