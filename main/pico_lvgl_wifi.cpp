#include "pico_lvgl_wifi.h"

#include <algorithm>
#include <cstring>

#include "FreeRTOS.h"
#include "lvgl.h"
#include "pico_ili9341.h"
#include "pico_touch.h"
#include "pins_pico2w.h"
#include "task.h"

namespace {

constexpr uint32_t display_buffer_lines = 20;
static uint8_t display_buffer[ILI9341_TFTWIDTH * display_buffer_lines * 2];
static lv_display_t *display = nullptr;
static lv_indev_t *input = nullptr;
static lv_obj_t *ssid_field = nullptr;
static lv_obj_t *password_field = nullptr;
static bool completed = false;
static bool save_requested = false;
static PicoSettings *active_settings = nullptr;

void touch_read(lv_indev_t *, lv_indev_data_t *data) {
    const PicoTouchPoint point = pico_touch_read(
        active_settings->calibration_min_x,
        active_settings->calibration_max_x,
        active_settings->calibration_min_y,
        active_settings->calibration_max_y
    );
    data->state = point.pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = point.x;
    data->point.y = point.y;
}

void finish_event(lv_event_t *event) {
    const auto code = lv_event_get_code(event);
    if (code != LV_EVENT_CLICKED) return;

    save_requested = lv_event_get_user_data(event) != nullptr;
    completed = true;
}

lv_obj_t *make_action_button(lv_obj_t *parent, const char *text, int32_t x, int32_t y, void *user_data) {
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 88, 28);
    lv_obj_set_pos(button, x, y);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    lv_obj_add_event_cb(button, finish_event, LV_EVENT_CLICKED, user_data);
    return button;
}

void set_field_text(lv_obj_t *field, const char *text) {
    lv_textarea_set_text(field, text);
    lv_textarea_set_cursor_pos(field, LV_TEXTAREA_CURSOR_LAST);
}

void copy_text(char *destination, size_t capacity, const char *source) {
    std::strncpy(destination, source, capacity - 1);
    destination[capacity - 1] = '\0';
}

void create_screen() {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000F), 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Wi-Fi settings");
    lv_obj_set_pos(title, 10, 6);

    ssid_field = lv_textarea_create(screen);
    lv_obj_set_size(ssid_field, 190, 34);
    lv_obj_set_pos(ssid_field, 10, 38);
    lv_textarea_set_one_line(ssid_field, true);
    lv_textarea_set_placeholder_text(ssid_field, "SSID");

    password_field = lv_textarea_create(screen);
    lv_obj_set_size(password_field, 190, 34);
    lv_obj_set_pos(password_field, 10, 78);
    lv_textarea_set_one_line(password_field, true);
    lv_textarea_set_password_mode(password_field, true);
    lv_textarea_set_placeholder_text(password_field, "Password");

    make_action_button(screen, "Save", 220, 38, &save_requested);
    make_action_button(screen, "Back", 220, 78, nullptr);

    lv_obj_t *keyboard = lv_keyboard_create(screen);
    lv_obj_set_size(keyboard, 320, 118);
    lv_obj_set_pos(keyboard, 0, 120);
    lv_keyboard_set_textarea(keyboard, ssid_field);
    lv_obj_add_event_cb(ssid_field, [](lv_event_t *event) {
        lv_keyboard_set_textarea(
            static_cast<lv_obj_t *>(lv_event_get_user_data(event)),
            lv_event_get_target_obj(event)
        );
    }, LV_EVENT_FOCUSED, keyboard);
    lv_obj_add_event_cb(password_field, [](lv_event_t *event) {
        lv_keyboard_set_textarea(
            static_cast<lv_obj_t *>(lv_event_get_user_data(event)),
            lv_event_get_target_obj(event)
        );
    }, LV_EVENT_FOCUSED, keyboard);

    set_field_text(ssid_field, active_settings->ssid);
    set_field_text(password_field, active_settings->password);
}

}

void pico_lvgl_wifi_run(PicoSettings *settings) {
    active_settings = settings;
    completed = false;
    save_requested = false;
    pico_touch_reset();

    static bool initialized = false;
    static TickType_t last_tick = xTaskGetTickCount();
    if (!initialized) {
        lv_init();
        display = lv_display_create(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT);
        lv_display_set_buffers(
            display,
            display_buffer,
            nullptr,
            sizeof(display_buffer),
            LV_DISPLAY_RENDER_MODE_PARTIAL
        );
        lv_display_set_flush_cb(display, ili9341_flush);
        input = lv_indev_create();
        lv_indev_set_type(input, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(input, touch_read);
        initialized = true;
    }

    create_screen();
    while (!completed) {
        const TickType_t now = xTaskGetTickCount();
        const TickType_t elapsed = now - last_tick;
        last_tick = now;
        if (elapsed > 0) {
            lv_tick_inc(static_cast<uint32_t>(elapsed));
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    if (save_requested) {
        copy_text(settings->ssid, sizeof(settings->ssid), lv_textarea_get_text(ssid_field));
        copy_text(settings->password, sizeof(settings->password), lv_textarea_get_text(password_field));
        pico_settings_save(settings);
    }
    lv_obj_clean(lv_screen_active());
    active_settings = nullptr;
}
