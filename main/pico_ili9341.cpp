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

void write_color(uint16_t color, size_t count) {
    const uint8_t pixel[] = {
        static_cast<uint8_t>(color >> 8), static_cast<uint8_t>(color)
    };
    gpio_put(ILI9341_PIN_DC, 1);
    gpio_put(ILI9341_PIN_CS, 0);
    for (size_t index = 0; index < count; ++index) {
        spi_write_blocking(ILI9341_SPI, pixel, sizeof(pixel));
    }
    gpio_put(ILI9341_PIN_CS, 1);
}

const uint8_t *glyph(char value) {
    static const uint8_t space[] = {0, 0, 0, 0, 0};
    static const uint8_t digits[][5] = {
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
        {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
        {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
        {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}
    };
    static const uint8_t letters[][5] = {
        {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36},
        {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x22, 0x1C},
        {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
        {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F},
        {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x20, 0x40, 0x41, 0x3F, 0x01},
        {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
        {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F},
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06},
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
        {0x26, 0x49, 0x49, 0x49, 0x32}, {0x01, 0x01, 0x7F, 0x01, 0x01},
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F},
        {0x3F, 0x40, 0x38, 0x40, 0x3F}, {0x63, 0x14, 0x08, 0x14, 0x63},
        {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43}
    };
    static const uint8_t plus[] = {0x08, 0x08, 0x3E, 0x08, 0x08};
    static const uint8_t minus[] = {0x08, 0x08, 0x08, 0x08, 0x08};
    static const uint8_t colon[] = {0x00, 0x36, 0x36, 0x00, 0x00};
    static const uint8_t star[] = {0x14, 0x08, 0x3E, 0x08, 0x14};
    static const uint8_t lowercase[][5] = {
        {0x20, 0x54, 0x54, 0x54, 0x78}, {0x7F, 0x48, 0x44, 0x38, 0x00},
        {0x38, 0x44, 0x44, 0x44, 0x00}, {0x38, 0x44, 0x48, 0x7F, 0x00},
        {0x38, 0x54, 0x54, 0x18, 0x00}, {0x08, 0x7E, 0x09, 0x01, 0x00},
        {0x0C, 0x52, 0x52, 0x3E, 0x00}, {0x7F, 0x08, 0x04, 0x78, 0x00},
        {0x00, 0x44, 0x7D, 0x40, 0x00}, {0x20, 0x40, 0x44, 0x3D, 0x00},
        {0x7F, 0x10, 0x28, 0x44, 0x00}, {0x00, 0x41, 0x7F, 0x40, 0x00},
        {0x7C, 0x04, 0x18, 0x04, 0x78}, {0x7C, 0x08, 0x04, 0x78, 0x00},
        {0x38, 0x44, 0x44, 0x38, 0x00}, {0x7C, 0x14, 0x14, 0x08, 0x00},
        {0x08, 0x14, 0x14, 0x7C, 0x00}, {0x7C, 0x08, 0x04, 0x00, 0x00},
        {0x48, 0x54, 0x54, 0x24, 0x00}, {0x04, 0x3F, 0x44, 0x00, 0x00},
        {0x3C, 0x40, 0x20, 0x7C, 0x00}, {0x1C, 0x20, 0x40, 0x20, 0x1C},
        {0x3C, 0x40, 0x30, 0x40, 0x3C}, {0x44, 0x28, 0x10, 0x28, 0x44},
        {0x0C, 0x50, 0x50, 0x3C, 0x00}, {0x44, 0x64, 0x54, 0x4C, 0x44}
    };
    if (value >= 'a' && value <= 'z') return lowercase[value - 'a'];
    if (value >= '0' && value <= '9') return digits[value - '0'];
    if (value >= 'A' && value <= 'Z') return letters[value - 'A'];
    if (value == '+') return plus;
    if (value == '-') return minus;
    if (value == ':') return colon;
    if (value == '*') return star;
    return space;
}

void draw_character(uint16_t x, uint16_t y, char value, uint16_t foreground, uint16_t background, uint8_t scale) {
    if (scale == 0) scale = 1;
    const uint16_t width = 6 * scale;
    const uint16_t height = 8 * scale;
    uint8_t pixels[18 * 24 * 2];
    for (uint16_t pixel = 0; pixel < width * height; ++pixel) {
        pixels[pixel * 2] = static_cast<uint8_t>(background >> 8);
        pixels[pixel * 2 + 1] = static_cast<uint8_t>(background);
    }
    const uint8_t *bitmap = glyph(value);
    for (uint16_t row = 0; row < 7; ++row) {
        for (uint16_t column = 0; column < 5; ++column) {
            const bool set = bitmap[column] & (1u << row);
            for (uint8_t dy = 0; dy < scale; ++dy) {
                for (uint8_t dx = 0; dx < scale; ++dx) {
                    const uint16_t px = column * scale + dx;
                    const uint16_t py = row * scale + dy;
                    const uint16_t color = set ? foreground : background;
                    const size_t offset = (py * width + px) * 2;
                    pixels[offset] = static_cast<uint8_t>(color >> 8);
                    pixels[offset + 1] = static_cast<uint8_t>(color);
                }
            }
        }
    }
    set_window(x, y, x + width - 1, y + height - 1);
    data(pixels, static_cast<size_t>(width) * height * 2);
}

}

void ili9341_init() {
    gpio_init(ILI9341_PIN_CS);
    gpio_init(ILI9341_PIN_DC);
    gpio_init(ILI9341_PIN_RST);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
    gpio_set_dir(ILI9341_PIN_RST, GPIO_OUT);
    gpio_put(ILI9341_PIN_CS, 1);

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

    const uint8_t memory_access = 0x28;
    command_with_data(cmd_madctl, &memory_access, 1);
    command(cmd_dispon);
    sleep_ms(100);
}

void ili9341_fill(uint16_t color) {
    set_window(0, 0, ILI9341_TFTWIDTH - 1, ILI9341_TFTHEIGHT - 1);
    write_color(color, static_cast<size_t>(ILI9341_TFTWIDTH) * ILI9341_TFTHEIGHT);
}

void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (x >= ILI9341_TFTWIDTH || y >= ILI9341_TFTHEIGHT || width == 0 || height == 0) {
        return;
    }
    if (x + width > ILI9341_TFTWIDTH) width = ILI9341_TFTWIDTH - x;
    if (y + height > ILI9341_TFTHEIGHT) height = ILI9341_TFTHEIGHT - y;
    set_window(x, y, x + width - 1, y + height - 1);
    write_color(color, static_cast<size_t>(width) * height);
}

void ili9341_draw_text(uint16_t x, uint16_t y, const char *text, uint16_t foreground, uint16_t background, uint8_t scale) {
    const uint16_t advance = 6 * (scale == 0 ? 1 : scale);
    while (*text != '\0' && x + advance <= ILI9341_TFTWIDTH) {
        draw_character(x, y, *text, foreground, background, scale);
        x += advance;
        ++text;
    }
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