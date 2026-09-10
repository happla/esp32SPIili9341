#pragma once

#include <stddef.h>
#include <stdint.h>

#include "lvgl.h"

void ili9341_init();
void ili9341_fill(uint16_t color);
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void ili9341_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels);