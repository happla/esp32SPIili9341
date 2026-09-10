#pragma once

#include <stdint.h>

struct PicoTouchPoint {
    uint16_t x;
    uint16_t y;
    uint16_t raw_x;
    uint16_t raw_y;
    bool pressed;
};

void pico_touch_init();
PicoTouchPoint pico_touch_read(uint16_t min_x, uint16_t max_x, uint16_t min_y, uint16_t max_y);