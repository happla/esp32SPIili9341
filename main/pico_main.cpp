#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"

#include "pico_ili9341.h"
#include "pins_pico2w.h"

extern "C" void vApplicationStackOverflowHook(TaskHandle_t, char *) {
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

static void display_task(void *) {
    ili9341_init();

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
    ili9341_draw_text(170, 160, "TARGET: 85 C", 0xFFE0, black, 1);
    ili9341_draw_text(25, 190, "HEATER: OFF", 0xFFFF, dark_gray, 1);
    ili9341_draw_text(185, 190, "CALIBRATE", 0xFFFF, purple, 1);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
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