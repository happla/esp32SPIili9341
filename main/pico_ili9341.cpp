#include "pico_ili9341.h"

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "pins_pico2w.h"

namespace {

constexpr uint8_t cmd_swreset = 0x01;
constexpr uint8_t cmd_slpout = 0x11;
constexpr uint8_t cmd_dispon = 0x29;
constexpr uint8_t cmd_madctl = 0x36;
constexpr uint8_t cmd_colmod = 0x3A;
constexpr uint8_t cmd_caset = 0x2A;
constexpr uint8_t cmd_paset = 0x2B;
constexpr uint8_t cmd_ramwr = 0x2C;

void command(uint8_t value) {
    gpio_put(ILI9341_PIN_DC, 0);
    gpio_put(ILI9341_PIN_CS, 0);
    spi_write_blocking(ILI9341_SPI, &value, 1);
    gpio_put(ILI9341_PIN_CS, 1);
}

void data(const uint8_t *values, size_t length) {
    gpio_put(ILI9341_PIN_DC, 1);
    gpio_put(ILI9341_PIN_CS, 0);
    spi_write_blocking(ILI9341_SPI, values, length);
    gpio_put(ILI9341_PIN_CS, 1);
}

void command_with_data(uint8_t value, const uint8_t *values, size_t length) {
    command(value);
    data(values, length);
}

void set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    const uint8_t columns[] = {
        static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1),
        static_cast<uint8_t>(x2 >> 8), static_cast<uint8_t>(x2)
    };
    const uint8_t rows[] = {
        static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1),
        static_cast<uint8_t>(y2 >> 8), static_cast<uint8_t>(y2)
    };

    command_with_data(cmd_caset, columns, sizeof(columns));
    command_with_data(cmd_paset, rows, sizeof(rows));
    command(cmd_ramwr);
}

}

void ili9341_init() {
    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RST);
    gpio_init(ILI9341_PIN_LED);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RST, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_LED, GPIO_OUT);
    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_LED, 1);

    spi_init(ILI9341_SPI, 24 * 1000 * 1000);
    gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);

    gpio_put(ILI9341_PIN_RST, 1);
    sleep_ms(5);
    gpio_put(ILI9341_PIN_RST, 0);
    sleep_ms(20);
    gpio_put(ILI9341_PIN_RST, 1);
    sleep_ms(150);

    command(cmd_swreset);
    sleep_ms(150);
    command(cmd_slpout);
    sleep_ms(150);

    const uint8_t pixel_format = 0x55;
    command_with_data(cmd_colmod, &pixel_format, 1);

    const uint8_t memory_access = 0x48;
    command_with_data(cmd_madctl, &memory_access, 1);
    command(cmd_dispon);
    sleep_ms(100);
}

void ili9341_flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    const uint16_t width = static_cast<uint16_t>(area->x2 - area->x1 + 1);
    const uint16_t height = static_cast<uint16_t>(area->y2 - area->y1 + 1);
    uint8_t line_buffer[ILI9341_TFTWIDTH * 2];

    if (width > ILI9341_TFTWIDTH) {
        lv_display_flush_ready(display);
        return;
    }

    set_window(area->x1, area->y1, area->x2, area->y2);

    for (uint16_t row = 0; row < height; ++row) {
        uint8_t *line = pixels + row * width * 2;
        for (uint16_t pixel = 0; pixel < width; ++pixel) {
            const uint16_t offset = pixel * 2;
            line_buffer[offset] = line[offset + 1];
            line_buffer[offset + 1] = line[offset];
        }
        data(line_buffer, width * 2);
    }

    lv_display_flush_ready(display);
}