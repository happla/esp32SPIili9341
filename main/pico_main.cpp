#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "pico/stdlib.h"

#include "pico_ili9341.h"
#include "pins_pico2w.h"

static void lvgl_task(void *) {
    lv_init();
    ili9341_init();

    static lv_color_t buffer[ILI9341_TFTWIDTH * 20];
    lv_display_t *display = lv_display_create(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT);
    lv_display_set_buffers(
        display,
        buffer,
        nullptr,
        sizeof(buffer),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );
    lv_display_set_flush_cb(display, ili9341_flush);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Pico W + ILI9341");
    lv_obj_center(label);

    while (true) {
        lv_tick_inc(1);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void application_task(void *) {
    gpio_init(ILI9341_PIN_LED);
    gpio_set_dir(ILI9341_PIN_LED, GPIO_OUT);

    while (true) {
        gpio_put(ILI9341_PIN_LED, !gpio_get(ILI9341_PIN_LED));
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main() {
    stdio_init_all();

    xTaskCreate(lvgl_task, "LVGL", 2048, nullptr, 2, nullptr);
    xTaskCreate(application_task, "Application", 1024, nullptr, 1, nullptr);
    vTaskStartScheduler();

    for (;;) {
    }
}