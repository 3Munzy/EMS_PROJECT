# STM32 Wearable Step Tracker

A wearable step counter running on STM32G071 with an ADXL335 accelerometer and a 1.3" ST7789 240×240 LCD. On power-on it runs three stages back to back — calibration, self-test, then the step tracker loop forever.

---

## Hardware

| Component | Part |
|-----------|------|
| MCU | STM32G071RB (NUCLEO-G071RB) |
| Accelerometer | ADXL335 (analog 3-axis, ±3g) |
| Display | ST7789 1.3" TFT, 240×240, RGB565, SPI |
| Button | Onboard blue push-button (PC13, active HIGH) |

**Pin assignments:**

| Signal | Pin | Description |
|--------|-----|-------------|
| BTN_CALIBRATE | PC13 | User button — calibration & self-test |
| ST7789_CS | PB6 | LCD chip select |
| ST7789_DC | PA9 | LCD data/command |
| ST7789_RST | PC7 | LCD reset |
| USART2_TX/RX | PA2/PA3 | Serial debug |
| LED_GREEN | PA5 | Onboard LED (unused by firmware) |

---

## Project Structure

```
Core/Inc/
  main.h                 GPIO pin definitions (CubeMX auto-generated)
  display.h              Display API — the main interface for everyone
  stage_calibration.h    Calibration stage interface
  stage_selftest.h       Self-test stage interface
  stage_tracker.h        Step tracker constants
  st7789.h               Low-level LCD driver (don't touch)

Core/Src/
  main.c                 Entry point — calls all three stages in order
  display.c              Step count, pace badge, status bar
  stage_calibration.c    Stage 1: 6-axis calibration screens + button logic
  stage_selftest.c        Stage 2: ADXL335 self-test screens + pass/fail
  stage_tracker.c        Stage 3: step count, SPM calc, pace classification
  st7789.c               ST7789 SPI driver (don't touch)
```

---

## Ownership

Everything in `Core/Inc/` and `Core/Src/` is owned by the LCD Engineer. **Don't edit any of these files.** The `.h` files are your interface — include them, call the functions, that's it.

Write your own `.c` and `.h` files for your subsystem logic and call into the LCD headers from there.

| File | Owner | Editable? |
|------|-------|-----------|
| All files in `Core/Inc/` | LCD Engineer | No |
| All files in `Core/Src/` | LCD Engineer | No |
| Your own files | You | Yes |

---

## System Flow

```
main()
  → Stage_Calibration_Run()   — 6-axis calibration, guided by LCD + button
  → Stage_SelfTest_Run()      — ADXL335 self-test, resets device if failed
  → Stage_Tracker_Run()       — infinite loop, counts steps, updates display
```

Status bar messages shown at each transition: `"Calibrating..."`, `"Ready"`, `"Resetting..."`.

---

## Integration

Each engineer needs to write their own `.c` file that hooks into the relevant stage. The hook locations are marked in the source with `[HOOK]` and `[PLACEHOLDER]` comments — just search for your role name.

**Static Calibration Engineer** — your ADC sampling runs inside `stage_calibration.c` after each button press (once per axis, 6 total). Hook location is clearly marked in the for-loop.

**Self-Test Engineer** — two spots in `stage_selftest.c`: the six dummy float variables (replace with real ADC reads) and the delta pass/fail check. Expected deltas: X = -325 mV, Y = +325 mV, Z = +550 mV, ±100 mV tolerance.

**Step Tracker Engineer** — the simulation phase block in `stage_tracker.c` (the `if (phase == 0...)` section + `HAL_Delay` + `step_count++`) gets replaced with your real step detection call. The SPM math and display updates already happen right after, so you just need to feed in the step event.

---

## Display API

Quick reference for the functions you'll actually call:

**`display.h`**
- `Display_SetStatus("text")` / `Display_ClearStatus()` — status bar at top
- `Display_UpdateSteps(n)` — big step count number in the middle
- `Display_UpdatePace(PACE_STATIONARY / PACE_WALKING / PACE_RUNNING)` — coloured badge at bottom

**`stage_calibration.h`**
- `Stage_Calibration_Run()` — runs the whole calibration sequence (main.c calls this)
- `Display_ShowCalibrateStep("direction")` — shows one axis prompt, e.g. `"+X UP"`

**`stage_selftest.h`**
- `Stage_SelfTest_Run()` — runs the whole self-test (main.c calls this), resets on fail
- `Display_ShowSelfTestBaseline(vx, vy, vz)` — show baseline readings
- `Display_ShowSelfTestST(st_vx, st_vy, st_vz)` — show self-test readings
- `Display_ShowSelfTestComparison(vx, vy, vz, st_vx, st_vy, st_vz)` — comparison table
- `Display_ShowSelfTestResult(passed)` — PASSED/FAILED screen

**`stage_tracker.h`**
- `Stage_Tracker_Run()` — infinite loop, never returns (main.c calls this)
- Thresholds: `SPM_STATIONARY_MAX = 40`, `SPM_WALKING_MAX = 130`

---

## Building

Add your `.c` files to the `target_sources` block in `CMakeLists.txt`:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/main.c
    ...
    Core/Src/your_file.c   # add here
)
```
