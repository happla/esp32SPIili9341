# ILI9341 Pico W / Pico 2 W

Pico SDK project for an ILI9341 display with LVGL and FreeRTOS.

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

## Build

Set `PICO_SDK_PATH` to the installed Pico SDK, then configure for Pico W:

```sh
cmake -S . -B build-pico -G Ninja -DPICO_BOARD=pico_w
cmake --build build-pico
```

For Pico 2 W, configure a separate build directory:

```sh
cmake -S . -B build-pico2w -G Ninja -DPICO_BOARD=pico2_w
cmake --build build-pico2w
```

LVGL and the Pico-compatible FreeRTOS kernel are fetched by CMake into the build
directory. They are not copied into the source tree.

The generated UF2 is `build-pico/ili9341_pico.uf2` or
`build-pico2w/ili9341_pico.uf2`.

## Flashing without BOOTSEL

With the board connected and running firmware, use:

```sh
picotool load -f build-pico/ili9341_pico.uf2
picotool reboot
```

If the firmware is not running, use SWD with a debug probe. The broken BOOTSEL
button does not prevent SWD programming.
