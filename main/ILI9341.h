#pragma once

#include <stdint.h>
#include <stddef.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "pins_esp32.h"

#define ILI9341_TFTWIDTH 240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP 0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID 0x04
#define ILI9341_RDDST 0x09

#define ILI9341_SLPIN 0x10
#define ILI9341_SLPOUT 0x11
#define ILI9341_PTLON 0x12
#define ILI9341_NORON 0x13

#define ILI9341_INVOFF 0x20
#define ILI9341_INVON 0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON 0x29

#define ILI9341_CASET 0x2A
#define ILI9341_PASET 0x2B
#define ILI9341_RAMWR 0x2C
#define ILI9341_RAMRD 0x2E

#define ILI9341_VSCRDEF 0x33
#define ILI9341_MADCTL 0x36
#define ILI9341_VSCRSADD 0x37
#define ILI9341_PIXFMT 0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1 0xC0
#define ILI9341_PWCTR2 0xC1
#define ILI9341_VMCTR1 0xC5
#define ILI9341_VMCTR2 0xC7

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

// MADCTL Flags
#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

// Colors
#define ILI9341_BLACK 0x0000
#define ILI9341_NAVY 0x000F
#define ILI9341_DARKGREEN 0x03E0
#define ILI9341_DARKCYAN 0x03EF
#define ILI9341_MAROON 0x7800
#define ILI9341_PURPLE 0x780F
#define ILI9341_OLIVE 0x7BE0
#define ILI9341_LIGHTGREY 0xC618
#define ILI9341_DARKGREY 0x7BEF
#define ILI9341_BLUE 0x001F
#define ILI9341_GREEN 0x07E0
#define ILI9341_CYAN 0x07FF
#define ILI9341_RED 0xF800
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW 0xFFE0
#define ILI9341_WHITE 0xFFFF
#define ILI9341_ORANGE 0xFD20
#define ILI9341_GREENYELLOW 0xAFE5
#define ILI9341_PINK 0xFC18

class ILI9341 {
public:
    ILI9341(gpio_num_t csPin = ILI9341_PIN_CS, gpio_num_t dcPin = ILI9341_PIN_DC, gpio_num_t rstPin = ILI9341_PIN_RST);
    ~ILI9341();

    bool init();
    void begin(uint32_t freq = 24000000);

    void setRotation(uint8_t rotation);
    void fillScreen(uint16_t color);
    void invertDisplay(bool i);

    void scrollTo(uint16_t y);
    void setScrollMargins(uint16_t top, uint16_t bottom);

    void drawPixel(uint16_t x, uint16_t y, uint16_t color);
    void drawHLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
    void drawVLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
    void drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size = 1);
    void drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size = 1);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

    uint16_t width() const;
    uint16_t height() const;

private:
    void reset();
    void writeCommand(uint8_t cmd);
    void writeCommand(uint8_t cmd, const uint8_t* data, size_t len);
    void writeData(const uint8_t* data, size_t len);
    void setAddrWindowInternal(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

    inline void select()   { gpio_set_level(m_cs, 0); }
    inline void deselect() { gpio_set_level(m_cs, 1); }
    inline void dcCmd()    { gpio_set_level(m_dc, 0); }
    inline void dcData()   { gpio_set_level(m_dc, 1); }

private:
    spi_device_handle_t m_spiDev;
    gpio_num_t m_cs;
    gpio_num_t m_dc;
    gpio_num_t m_rst;

    uint16_t m_width;
    uint16_t m_height;
    uint32_t m_spiFrequency;
    SemaphoreHandle_t m_spiMutex;
};
