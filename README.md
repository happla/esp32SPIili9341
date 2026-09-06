# ILI9341 Display Controller

ESP-IDF firmware for an ESP32-WROOM driving a 2.4-inch ILI9341 TFT display
with an XPT2046-compatible resistive touchscreen.

> [!NOTE]
> Pico W / Pico 2 W development is on the
> [`feature/aitest-pico-lvgl-port`](../../tree/feature/aitest-pico-lvgl-port)

## Status

The ESP32 display UI and touchscreen are tested and working.

The Pico W port currently has a verified LVGL/FreeRTOS build and generated UF2,
but its hardware display test and touch/UI port are still incomplete.

## ESP32 Firmware

The ESP32 firmware includes:

- ILI9341 display initialization over VSPI
- XPT2046 touchscreen input and calibration
- Sauna controller demo interface
- FreeRTOS display task

## Hardware

### Display

| Display signal | ESP32-WROOM GPIO |
| --- | ---: |
| VCC | 3.3 V |
| GND | GND |
| CS | 5 |
| DC | 2 |
| RESET | 4 |
| MOSI | 23 |
| SCK | 18 |
| MISO | 19 |
| LED | 3.3 V |

### Touchscreen

The touchscreen shares the SPI bus with the display:

| Touch signal | ESP32-WROOM GPIO |
| --- | ---: |
| T_CS | 15 |
| T_DIN | 23 |
| T_DO | 19 |
| T_IRQ | 21 |

## Project Layout

```text
.
├── CMakeLists.txt
└── main/
    ├── DisplayTask.cpp    # Display, touch, calibration, and demo UI
    ├── DisplayTask.h
    ├── ILI9341.cpp        # ESP32 ILI9341 driver
    ├── ILI9341.h
    ├── main.cpp           # ESP-IDF app_main entry point
    └── pins_esp32.h       # ESP32-WROOM VSPI pin map
```

## Build

Prerequisites:

- ESP-IDF installed and exported in `IDF_PATH`
- ESP32 toolchain configured through ESP-IDF
- ESP32-WROOM connected over USB/UART

From the project directory:

```sh
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

Press `Ctrl+]` to exit the serial monitor.

## Pico Development

Switch to the Pico branch before working on the Pico port:

```sh
git switch feature/aitest-pico-lvgl-port
```

See that branch's README for Pico SDK setup, LVGL and FreeRTOS dependencies,
Pico W / Pico 2 W pin mappings, UF2 flashing, and SWD recovery instructions.

## Next Steps

- Port the tested ESP32 UI behavior to the Pico branch.
- Add Pico XPT2046 touch input.
- Persist touch calibration so the user does not need to calibrate on every boot.
- Enhance the LVGL UI after the functional port is complete.

<div>
    <img src="https://github.com/happla/esp32SPIili9341/blob/main/IMG_1814.JPG" width="400">
</div>
