#pragma once

#include <stddef.h>
#include <stdint.h>

#include "lvgl.h"

void ili9341_init();
void ili9341_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels);