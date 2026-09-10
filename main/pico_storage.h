#pragma once

#include <stdint.h>

struct PicoSettings {
    uint16_t calibration_min_x;
    uint16_t calibration_max_x;
    uint16_t calibration_min_y;
    uint16_t calibration_max_y;
    int16_t target_temperature;
    uint8_t locked;
    char ssid[33];
    char password[65];
};

void pico_settings_defaults(PicoSettings *settings);
bool pico_settings_load(PicoSettings *settings);
bool pico_settings_save(const PicoSettings *settings);