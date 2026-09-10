#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "pico_ili9341.h"
#include "pico_storage.h"
#include "pico_touch.h"
#include "pins_pico2w.h"

extern "C" void vApplicationStackOverflowHook(TaskHandle_t, char *) {
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

static void render_screen(const PicoSettings &settings, bool heater_on) {
    constexpr uint16_t black = 0x0000;
    constexpr uint16_t navy = 0x000F;
    constexpr uint16_t dark_gray = 0x7BEF;
    constexpr uint16_t dark_green = 0x0320;
    constexpr uint16_t maroon = 0x7800;
    constexpr uint16_t purple = 0x780F;

    ili9341_fill(black);
    ili9341_fill_rect(0, 0, 320, 35, navy);
    ili9341_fill_rect(15, 45, 140, 110, dark_gray);
    ili9341_fill_rect(18, 48, 134, 104, black);
    ili9341_fill_rect(170, 45, 135, 50, dark_green);
    ili9341_fill_rect(170, 105, 135, 50, maroon);
    ili9341_fill_rect(15, 175, 145, 45, dark_gray);
    ili9341_fill_rect(170, 175, 135, 45, purple);
    ili9341_draw_text(10, 10, "SAUNA CONTROLLER", 0xFFFF, navy, 2);
    ili9341_draw_text(25, 55, "CURRENT TEMP", 0xC618, black, 1);
    ili9341_draw_text(35, 88, "78 C", 0xFFFF, black, 2);
    ili9341_draw_text(205, 62, "+", 0xFFFF, dark_green, 3);
    ili9341_draw_text(205, 122, "-", 0xFFFF, maroon, 3);
    char target_text[20];
    snprintf(target_text, sizeof(target_text), "TARGET: %d C", settings.target_temperature);
    ili9341_draw_text(170, 160, target_text, 0xFFE0, black, 1);
    ili9341_draw_text(25, 190, heater_on ? "HEATER: ON" : "HEATER: OFF", 0xFFFF, heater_on ? 0xF800 : dark_gray, 1);
    ili9341_draw_text(185, 190, "WIFI SETUP", 0xFFFF, purple, 1);
    ili9341_fill_rect(240, 0, 80, 35, settings.locked ? 0x7800 : 0x0320);
    ili9341_draw_text(242, 10, settings.locked ? "DOOR CLOSED" : "DOOR OPEN", 0xFFFF,
                      settings.locked ? 0x7800 : 0x0320, 1);
    ili9341_draw_text(20, 225, settings.ssid[0] == '\0' ? "WIFI: NOT SET" : "WIFI: CONFIGURED", 0x07FF, black, 1);
}

static void render_wifi_fields(const PicoSettings &settings, uint8_t field) {
    constexpr uint16_t black = 0x0000;
    constexpr uint16_t dark_gray = 0x7BEF;
    constexpr uint16_t cyan = 0x07FF;
    ili9341_fill_rect(10, 32, 300, 26, field == 1 ? cyan : dark_gray);
    ili9341_fill_rect(13, 35, 294, 20, black);
    ili9341_fill_rect(10, 64, 300, 26, field == 2 ? cyan : dark_gray);
    ili9341_fill_rect(13, 67, 294, 20, black);
    ili9341_draw_text(18, 40, "SSID:", 0xC618, black, 1);
    ili9341_draw_text(18, 72, "PASSWORD:", 0xC618, black, 1);
    ili9341_draw_text(58, 40, settings.ssid[0] == '\0' ? "NOT SET" : settings.ssid, 0xFFFF, black, 1);

    char masked_password[65] = {};
    const size_t password_length = strlen(settings.password);
    for (size_t index = 0; index < password_length && index < sizeof(masked_password) - 1; ++index) {
        masked_password[index] = '*';
    }
    ili9341_draw_text(78, 72, password_length == 0 ? "NOT SET" : masked_password, 0xFFFF, black, 1);
}

static void render_wifi(const PicoSettings &settings, uint8_t field, bool lowercase) {
    constexpr uint16_t black = 0x0000;
    constexpr uint16_t navy = 0x000F;
    constexpr uint16_t dark_gray = 0x7BEF;
    constexpr uint16_t cyan = 0x07FF;
    constexpr uint16_t green = 0x07E0;
    constexpr uint16_t maroon = 0x7800;
    constexpr uint16_t purple = 0x780F;

    ili9341_fill(black);
    ili9341_fill_rect(0, 0, 320, 35, navy);
    ili9341_draw_text(10, 10, "WIFI SETTINGS", 0xFFFF, navy, 2);
    render_wifi_fields(settings, field);

    static constexpr const char *rows[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
    static constexpr uint8_t row_lengths[] = {10, 9, 7};
    static constexpr uint16_t row_offsets[] = {0, 16, 48};
    for (uint8_t row = 0; row < 3; ++row) {
        for (uint8_t column = 0; column < row_lengths[row]; ++column) {
            const uint16_t x = row_offsets[row] + column * 32;
                    ili9341_fill_rect(x + 1, 105 + row * 30, 30, 27, dark_gray);
            char key[2] = {lowercase ? static_cast<char>(rows[row][column] + ('a' - 'A')) : rows[row][column], '\0'};
            ili9341_draw_text(x + 11, 113 + row * 30, key, 0xFFFF, dark_gray, 1);
        }
    }
    ili9341_fill_rect(1, 208, 78, 30, maroon);
    ili9341_fill_rect(81, 208, 78, 30, dark_gray);
    ili9341_fill_rect(161, 208, 78, 30, green);
    ili9341_fill_rect(241, 208, 78, 30, purple);
    ili9341_draw_text(20, 218, "DEL", 0xFFFF, maroon, 1);
    ili9341_draw_text(101, 218, lowercase ? "LOWER" : "UPPER", 0xFFFF, dark_gray, 1);
    ili9341_draw_text(178, 218, "SAVE", 0x0000, green, 1);
    ili9341_draw_text(258, 218, "BACK", 0xFFFF, purple, 1);
}

static void append_character(char *buffer, size_t capacity, char value) {
    const size_t length = strlen(buffer);
    if (length + 1 < capacity) {
        buffer[length] = value;
        buffer[length + 1] = '\0';
    }
}

static void delete_character(char *buffer) {
    const size_t length = strlen(buffer);
    if (length > 0) buffer[length - 1] = '\0';
}

static void render_target(const PicoSettings &settings) {
    char target_text[20];
    ili9341_fill_rect(165, 155, 150, 20, 0x0000);
    snprintf(target_text, sizeof(target_text), "TARGET: %d C", settings.target_temperature);
    ili9341_draw_text(170, 160, target_text, 0xFFE0, 0x0000, 1);
}

static void render_lock(const PicoSettings &settings) {
    const uint16_t background = settings.locked ? 0x7800 : 0x0320;
    ili9341_fill_rect(240, 0, 80, 35, background);
    ili9341_draw_text(242, 10, settings.locked ? "DOOR CLOSED" : "DOOR OPEN", 0xFFFF, background, 1);
}

static void render_heater(bool heater_on) {
    const uint16_t background = heater_on ? 0xF800 : 0x7BEF;
    ili9341_fill_rect(15, 175, 145, 45, background);
    ili9341_draw_text(25, 190, heater_on ? "HEATER: ON" : "HEATER: OFF", 0xFFFF, background, 1);
}

static void display_task(void *) {
    ili9341_init();
    pico_touch_init();
    PicoSettings settings;
    if (!pico_settings_load(&settings)) {
        pico_settings_save(&settings);
    }
    bool heater_on = false;
    bool wifi_screen = false;
    uint8_t wifi_field = 1;
    bool keyboard_lowercase = false;
    render_screen(settings, heater_on);

    while (true) {
        const PicoTouchPoint point = pico_touch_read(
            settings.calibration_min_x,
            settings.calibration_max_x,
            settings.calibration_min_y,
            settings.calibration_max_y
        );
        if (point.pressed) {
            if (!wifi_screen) {
                char touch_text[24];
                snprintf(touch_text, sizeof(touch_text), "X:%03d Y:%03d", point.x, point.y);
                ili9341_draw_text(190, 225, touch_text, 0xFFFF, 0x0000, 1);
            }
            if (wifi_screen) {
                if (point.y >= 32 && point.y < 58) {
                    wifi_field = 1;
                    render_wifi_fields(settings, wifi_field);
                } else if (point.y >= 64 && point.y < 90) {
                    wifi_field = 2;
                    render_wifi_fields(settings, wifi_field);
                } else if (point.y >= 105 && point.y < 208) {
                    static constexpr const char *rows[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
                    static constexpr uint8_t row_lengths[] = {10, 9, 7};
                    static constexpr uint16_t row_offsets[] = {0, 16, 48};
                    const uint8_t row = static_cast<uint8_t>((point.y - 105) / 30);
                    const uint16_t offset = row_offsets[row];
                    const uint8_t column = static_cast<uint8_t>((point.x - offset) / 32);
                    if (row < 3 && point.x >= offset && column < row_lengths[row]) {
                        append_character(wifi_field == 1 ? settings.ssid : settings.password,
                                         wifi_field == 1 ? sizeof(settings.ssid) : sizeof(settings.password),
                                         keyboard_lowercase ? static_cast<char>(rows[row][column] + ('a' - 'A')) : rows[row][column]);
                        render_wifi_fields(settings, wifi_field);
                    }
                } else if (point.y >= 208) {
                    if (point.x < 80) {
                        delete_character(wifi_field == 1 ? settings.ssid : settings.password);
                    } else if (point.x < 160) {
                        keyboard_lowercase = !keyboard_lowercase;
                        render_wifi(settings, wifi_field, keyboard_lowercase);
                    } else if (point.x < 240) {
                        pico_settings_save(&settings);
                        wifi_screen = false;
                        render_screen(settings, heater_on);
                    } else {
                        wifi_screen = false;
                        render_screen(settings, heater_on);
                    }
                    if (wifi_screen && point.x < 80) render_wifi_fields(settings, wifi_field);
                }
            } else if (point.y < 35 && point.x >= 235) {
                settings.locked = settings.locked == 0;
                pico_settings_save(&settings);
                render_lock(settings);
            } else if (!settings.locked && point.x >= 160 && point.x <= 315 && point.y >= 40 && point.y <= 100) {
                if (settings.target_temperature < 110) ++settings.target_temperature;
                pico_settings_save(&settings);
                render_target(settings);
            } else if (!settings.locked && point.x >= 160 && point.x <= 315 && point.y >= 101 && point.y <= 160) {
                if (settings.target_temperature > 0) --settings.target_temperature;
                pico_settings_save(&settings);
                render_target(settings);
            } else if (!settings.locked && point.x >= 15 && point.x <= 160 && point.y >= 175 && point.y <= 220) {
                heater_on = !heater_on;
                render_heater(heater_on);
            } else if (!settings.locked && point.x >= 160 && point.x <= 315 && point.y >= 175 && point.y <= 220) {
                wifi_screen = true;
                wifi_field = 1;
                keyboard_lowercase = false;
                render_wifi(settings, wifi_field, keyboard_lowercase);
            }
            while (pico_touch_read(settings.calibration_min_x, settings.calibration_max_x,
                                   settings.calibration_min_y, settings.calibration_max_y).pressed) {
                vTaskDelay(pdMS_TO_TICKS(20));
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void application_task(void *) {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main() {
    stdio_init_all();

    xTaskCreate(display_task, "Display", 2048, nullptr, 2, nullptr);
    xTaskCreate(application_task, "Application", 1024, nullptr, 1, nullptr);
    vTaskStartScheduler();

    for (;;) {
    }
}