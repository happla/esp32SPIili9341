#include "pico_touch.h"

#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "pins_pico2w.h"

namespace {

constexpr uint8_t touch_cmd_x = 0x90;
constexpr uint8_t touch_cmd_y = 0xD0;

uint16_t read_axis(uint8_t command) {
    uint8_t tx[3] = {command, 0, 0};
    uint8_t rx[3] = {};
    gpio_put(ILI9341_PIN_CS, 1);
    gpio_put(ILI9341_PIN_TCS, 0);
    spi_write_read_blocking(ILI9341_SPI, tx, rx, sizeof(tx));
    gpio_put(ILI9341_PIN_TCS, 1);
    return static_cast<uint16_t>(((rx[1] << 8) | rx[2]) >> 3);
}

uint16_t read_filtered_axis(uint8_t command) {
    uint32_t total = 0;
    constexpr uint8_t sample_count = 5;
    for (uint8_t sample = 0; sample < sample_count; ++sample) {
        total += read_axis(command);
        sleep_us(100);
    }
    return static_cast<uint16_t>(total / sample_count);
}

uint16_t map_axis(uint16_t value, uint16_t input_min, uint16_t input_max, uint16_t output_max) {
    if (input_min == input_max) return 0;
    if (input_min < input_max) {
        if (value < input_min) value = input_min;
        if (value > input_max) value = input_max;
        return static_cast<uint16_t>((static_cast<uint32_t>(value - input_min) * output_max) /
                                     (input_max - input_min));
    }
    if (value > input_min) value = input_min;
    if (value < input_max) value = input_max;
    return static_cast<uint16_t>((static_cast<uint32_t>(input_min - value) * output_max) /
                                 (input_min - input_max));
}

}

void pico_touch_init() {
    gpio_init(ILI9341_PIN_TCS);
    gpio_set_dir(ILI9341_PIN_TCS, GPIO_OUT);
    gpio_put(ILI9341_PIN_TCS, 1);
    gpio_init(ILI9341_PIN_TRQ);
    gpio_set_dir(ILI9341_PIN_TRQ, GPIO_IN);
    gpio_pull_up(ILI9341_PIN_TRQ);
}

PicoTouchPoint pico_touch_read(uint16_t min_x, uint16_t max_x, uint16_t min_y, uint16_t max_y) {
    PicoTouchPoint point = {};
    spi_set_baudrate(ILI9341_SPI, 2 * 1000 * 1000);
    const uint16_t raw_x = read_filtered_axis(touch_cmd_x);
    const uint16_t raw_y = read_filtered_axis(touch_cmd_y);
    spi_set_baudrate(ILI9341_SPI, 24 * 1000 * 1000);
    point.raw_x = raw_x;
    point.raw_y = raw_y;
    if (raw_x < 100 || raw_x > 4000 || raw_y < 100 || raw_y > 4000) return point;

    point.pressed = true;
    point.x = map_axis(raw_x, max_x, min_x, ILI9341_TFTWIDTH - 1);
    point.y = map_axis(raw_y, min_y, max_y, ILI9341_TFTHEIGHT - 1);
    return point;
}