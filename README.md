# ILI9341 Pico W / Pico 2 W

Pico SDK project for an ILI9341 display with LVGL and FreeRTOS. The current
target is Pico W; Pico 2 W is supported as a selectable CMake board target.

The current firmware initializes LVGL, drives the ILI9341 over SPI, and renders
a test label. Touch input and the application UI are not connected yet.

## Hardware

The current pin map is shared by Pico W and Pico 2 W:

| Signal | GPIO |
| --- | ---: |
| MISO / T_DO | 16 |
| Display CS | 17 |
| SCK / T_CLK | 18 |
| MOSI / T_DIN | 19 |
| DC | 20 |
| RESET | 21 |
| Backlight | 22 |
| Touch CS | 15 |
| Touch IRQ | 14 |

## Prerequisites

Install or provide:

- Pico SDK 2.x
- CMake
- Ninja
- Arm GNU Toolchain
- Internet access for the first configure, so CMake can fetch LVGL and FreeRTOS

Set `PICO_SDK_PATH` to the Pico SDK location. For example:

```sh
export PICO_SDK_PATH="$HOME/pico/pico-sdk"
```

LVGL and the Pico-compatible FreeRTOS kernel are downloaded automatically into
`build-pico/_deps/`. They are not copied into the source tree.

## Build

Run these commands from the project directory:

```sh
cmake -S . -B build-pico -G Ninja -DPICO_BOARD=pico_w
cmake --build build-pico
```

For Pico 2 W, configure a separate build directory:

```sh
cmake -S . -B build-pico2w -G Ninja -DPICO_BOARD=pico2_w
cmake --build build-pico2w
```

Use a separate build directory when changing board targets. The Pico 2 W build
has not yet been hardware-tested.

The generated UF2 is `build-pico/ili9341_pico.uf2` or
`build-pico2w/ili9341_pico.uf2`.

## Robot Framework Tests

The firmware smoke suite is in `tests/firmware.robot`. It checks the LVGL
display path, pin definitions, Pico W configuration, firmware compilation, and
UF2 generation without requiring a connected board.

Run the source and build checks with RobotCode:

```sh
robotcode robot -i sourceORbuild tests/firmware.robot
```

With a Pico connected and already running firmware, check board detection:

```sh
robotcode robot -i hardware tests/firmware.robot
```

To intentionally load and reboot the board, include the `flash` tag:

```sh
robotcode robot -i flash tests/firmware.robot
```

The suite writes its build output to `build-robot/`. Hardware tests are kept
separate because `picotool load` changes the firmware on the connected board.

## Flashing

### Running firmware

With the board connected and running firmware, use:

```sh
picotool load -f build-pico/ili9341_pico.uf2
picotool reboot
```

This works without pressing BOOTSEL. It requires the firmware to be running and
the USB connection to be available.

### SWD recovery

If the board is not running firmware, use an SWD debug probe with OpenOCD or
Picoprobe. A broken BOOTSEL button does not prevent SWD programming. SWD is the
recovery path if `picotool` cannot find the board.

## Project files

- `main/pico_ili9341.cpp`: Pico SPI and ILI9341 initialization/flush driver
- `main/pico_main.cpp`: FreeRTOS tasks and LVGL setup
- `main/pins_pico2w.h`: shared Pico W/Pico 2 W pin map
- `main/lv_conf.h`: LVGL configuration
- `main/FreeRTOSConfig.h`: FreeRTOS configuration
