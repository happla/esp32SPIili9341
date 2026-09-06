# ILI9341 Display Controller

ESP-IDF firmware for an ESP32-WROOM driving a 2.4-inch ILI9341 TFT display
with an XPT2046-compatible resistive touchscreen.
> [!NOTE]
> Pico W / Pico 2 W development has moved to the
> [`feature/aitest-pico-lvgl-port`](../../tree/feature/aitest-pico-lvgl-port)
## Current Firmware

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
    └── pins_esp32.h      # ESP32-WROOM VSPI pin map
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
- Use LVGL for a richer UI on the Pico branch.
- Persist touch calibration so the user does not need to calibrate on every boot.

<div>
    <img src="https://github.com/happla/esp32SPIili9341/blob/main/IMG_1814.JPG" width="400">
</div>
<<<<<<< HEAD
# next steps:
- use lvgl for better ui
- fix calibration through nvr so user doesnt have to calibrate everytime display turns on
  
```
esp32_display_project/
├── CMakeLists.txt              # Root CMake configuration
=======
# ILI9341 Display Controller

ESP-IDF firmware for an ESP32-WROOM driving a 2.4-inch ILI9341 TFT display
with an XPT2046-compatible resistive touchscreen.

> [!NOTE]
> Pico W / Pico 2 W development has moved to the
> [`feature/aitest-pico-lvgl-port`](../../tree/feature/aitest-pico-lvgl-port)
> branch. That branch contains the Pico SDK, FreeRTOS, LVGL, and Pico-native
> ILI9341 driver work.

## Current Firmware

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
>>>>>>> 62b40b9 (update readme)
└── main/
    ├── DisplayTask.cpp    # Display, touch, calibration, and demo UI
    ├── DisplayTask.h
    ├── ILI9341.cpp        # ESP32 ILI9341 driver
    ├── ILI9341.h
    ├── main.cpp           # ESP-IDF app_main entry point
    └── pins_esp32.h      # ESP32-WROOM VSPI pin map
```

<<<<<<< HEAD
| Display |  ESP32-wroom pins |
| --- | --- |
| VCC | 3v3 |
| GND  |  GND |
| CS | GPIO5 |
| DC | GPIO2  |
| RST |  GPIO4 |
| MOSI |  GPIO23 |
| SCK | GPIO18 |
| MISO |  GPIO19 |
| LED | 3v3 |
| T_CS | GPIO15 |
| T_DIN |  GPIO23 |
| T_DO | GPIO 19 |
| T_IRQ | GPIO21 |

<div>
    <img src="https://github.com/happla/esp32SPIili9341/blob/main/IMG_1814.JPG" width="400">
</div>


=======
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
>>>>>>> 62b40b9 (update readme)
