#include "ILI9341.h"
#include <string.h>

ILI9341::ILI9341(gpio_num_t csPin, gpio_num_t dcPin, gpio_num_t rstPin)
    : m_spiDev(NULL), m_cs(csPin), m_dc(dcPin), m_rst(rstPin),
      m_width(ILI9341_TFTWIDTH), m_height(ILI9341_TFTHEIGHT),
      m_spiFrequency(24000000), m_spiMutex(NULL) {}

ILI9341::~ILI9341() {
    if (m_spiMutex != NULL) {
        vSemaphoreDelete(m_spiMutex);
    }
    if (m_spiDev != NULL) {
        spi_bus_remove_device(m_spiDev);
    }
}

bool ILI9341::init() {
    if (m_spiMutex == NULL) {
        m_spiMutex = xSemaphoreCreateMutex();
        if (m_spiMutex == NULL) return false;
    }

    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        // Initialize GPIO pins
        gpio_config_t io_conf = {};
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << m_cs) | (1ULL << m_dc) | (1ULL << m_rst);
        gpio_config(&io_conf);
        
        deselect();
        gpio_set_level(m_rst, 1);

        // Initialize ESP32 SPI Bus
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = ILI9341_PIN_MOSI;
        buscfg.miso_io_num = ILI9341_PIN_MISO;
        buscfg.sclk_io_num = ILI9341_PIN_SCK;
        buscfg.quadwp_io_num = -1;
        buscfg.quadhd_io_num = -1;
        buscfg.max_transfer_sz = 4096;

        spi_bus_initialize(ILI9341_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

        // Attach Display to SPI Bus
        spi_device_interface_config_t devcfg = {};
        devcfg.clock_speed_hz = m_spiFrequency;
        devcfg.mode = 0;
        devcfg.spics_io_num = -1; // Handled manually
        devcfg.queue_size = 7;

        spi_bus_add_device(ILI9341_SPI_HOST, &devcfg, &m_spiDev);

        reset();

        // Initialization Commands
        static const uint8_t initcmd[] = {
          0xEF, 3, 0x03, 0x80, 0x02,
          0xCF, 3, 0x00, 0xC1, 0x30,
          0xED, 4, 0x64, 0x03, 0x12, 0x81,
          0xE8, 3, 0x85, 0x00, 0x78,
          0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
          0xF7, 1, 0x20,
          0xEA, 2, 0x00, 0x00,
          ILI9341_PWCTR1  , 1, 0x23,
          ILI9341_PWCTR2  , 1, 0x10,
          ILI9341_VMCTR1  , 2, 0x3e, 0x28,
          ILI9341_VMCTR2  , 1, 0x86,
          ILI9341_MADCTL  , 1, 0x48,
          ILI9341_VSCRSADD, 1, 0x00,
          ILI9341_PIXFMT  , 1, 0x55,
          ILI9341_FRMCTR1 , 2, 0x00, 0x18,
          ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27,
          0xF2, 1, 0x00,
          ILI9341_GAMMASET , 1, 0x01,
          ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
          ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
          ILI9341_SLPOUT  , 0x80,
          ILI9341_DISPON  , 0x80,
          0x00
        };

        const uint8_t *addr = initcmd;
        while (*addr) {
            uint8_t cmd = *addr++;
            uint8_t numArgs = *addr++;
            uint8_t ms = numArgs & 0x80;
            numArgs &= 0x7F;

            dcCmd(); select();
            spi_transaction_t t = {};
            t.length = 8; t.tx_buffer = &cmd;
            spi_device_polling_transmit(m_spiDev, &t);
            deselect();

            if (numArgs > 0) {
                dcData(); select();
                t.length = numArgs * 8; t.tx_buffer = addr;
                spi_device_polling_transmit(m_spiDev, &t);
                deselect();
                addr += numArgs;
            }

            if (ms) {
                xSemaphoreGive(m_spiMutex);
                vTaskDelay(pdMS_TO_TICKS(150));
                (void)xSemaphoreTake(m_spiMutex, portMAX_DELAY);
            }
        }
        xSemaphoreGive(m_spiMutex);
    }
    return true;
}

void ILI9341::begin(uint32_t freq) {
    m_spiFrequency = freq;
    init();
}

void ILI9341::reset() {
    gpio_set_level(m_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(m_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(m_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(150));
}

void ILI9341::writeCommand(uint8_t cmd) {
    dcCmd(); select();
    spi_transaction_t t = {};
    t.length = 8; t.tx_buffer = &cmd;
    spi_device_polling_transmit(m_spiDev, &t);
    deselect();
}

void ILI9341::writeCommand(uint8_t cmd, const uint8_t* data, size_t len) {
    writeCommand(cmd);
    if (len > 0 && data != nullptr) writeData(data, len);
}

void ILI9341::writeData(const uint8_t *data, size_t len) {
    if (len == 0) return;
    dcData(); select();
    spi_transaction_t t = {};
    t.length = len * 8; t.tx_buffer = data;
    spi_device_polling_transmit(m_spiDev, &t);
    deselect();
}

void ILI9341::setAddrWindowInternal(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint8_t xa[] = { (uint8_t)(x >> 8), (uint8_t)(x & 0xFF), (uint8_t)((x+w-1) >> 8), (uint8_t)((x+w-1) & 0xFF) };
    uint8_t ya[] = { (uint8_t)(y >> 8), (uint8_t)(y & 0xFF), (uint8_t)((y+h-1) >> 8), (uint8_t)((y+h-1) & 0xFF) };

    writeCommand(ILI9341_CASET, xa, 4);
    writeCommand(ILI9341_PASET, ya, 4);
    writeCommand(ILI9341_RAMWR);
}

void ILI9341::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        setAddrWindowInternal(x, y, w, h);
        xSemaphoreGive(m_spiMutex);
    }
}

void ILI9341::drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= m_width || y >= m_height) return;

    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        setAddrWindowInternal(x, y, 1, 1);
        uint8_t buffer[] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
        writeData(buffer, 2);
        xSemaphoreGive(m_spiMutex);
    }
}

static const uint8_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14, 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, 0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x50, 0x30, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x60, 0x60, 0x00, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00,
    0x42, 0x61, 0x51, 0x49, 0x46, 0x21, 0x41, 0x45, 0x4B, 0x31, 0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x01, 0x71, 0x09, 0x05, 0x03,
    0x36, 0x49, 0x49, 0x49, 0x36, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x36, 0x36, 0x00, 0x00,
    0x00, 0x56, 0x36, 0x00, 0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08, 0x02, 0x01, 0x51, 0x09, 0x06, 0x32, 0x49, 0x79, 0x41, 0x3E,
    0x7E, 0x11, 0x11, 0x11, 0x7E, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x22, 0x1C, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x49, 0x49, 0x7A, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x63, 0x14, 0x08, 0x14, 0x63,
    0x07, 0x08, 0x70, 0x08, 0x07, 0x61, 0x51, 0x49, 0x45, 0x43
};

void ILI9341::fillScreen(uint16_t color) {
    fillRect(0, 0, m_width, m_height, color);
}

void ILI9341::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= m_width) || (y >= m_height)) return;
    if ((x + w - 1) >= m_width)  w = m_width  - x;
    if ((y + h - 1) >= m_height) h = m_height - y;

    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        setAddrWindowInternal(x, y, w, h);

        dcData(); select();
        
        uint8_t highByte = color >> 8;
        uint8_t lowByte  = color & 0xFF;

        #define PIXEL_BUF_SIZE 64
        uint8_t buffer[PIXEL_BUF_SIZE * 2];
        for (size_t i = 0; i < PIXEL_BUF_SIZE; i++) {
            buffer[i * 2]     = highByte;
            buffer[i * 2 + 1] = lowByte;
        }

        size_t totalPixels = w * h;
        while (totalPixels > 0) {
            size_t pixelsToWrite = (totalPixels < PIXEL_BUF_SIZE) ? totalPixels : PIXEL_BUF_SIZE;
            spi_transaction_t t = {};
            t.length = pixelsToWrite * 2 * 8;
            t.tx_buffer = buffer;
            spi_device_polling_transmit(m_spiDev, &t);
            totalPixels -= pixelsToWrite;
        }
        deselect();
        xSemaphoreGive(m_spiMutex);
    }
}

void ILI9341::drawHLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
    fillRect(x, y, length, 1, color);
}

void ILI9341::drawVLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
    fillRect(x, y, 1, length, color);
}

void ILI9341::drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (c < ' ' || c > 'Z') c = '?';
    uint16_t idx = (c - ' ') * 5;

    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = font5x7[idx + i];
        for (int8_t j = 0; j < 8; j++) {
            if (line & 0x01) {
                if (size == 1) drawPixel(x + i, y + j, color);
                else fillRect(x + i * size, y + j * size, size, size, color);
            } else if (bg != color) {
                if (size == 1) drawPixel(x + i, y + j, bg);
                else fillRect(x + i * size, y + j * size, size, size, bg);
            }
            line >>= 1;
        }
    }
}

void ILI9341::drawString(uint16_t x, uint16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        drawChar(x, y, *str, color, bg, size);
        x += 6 * size;
        str++;
    }
}

void ILI9341::setRotation(uint8_t rotation) {
    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        uint8_t madctl = 0;
        rotation %= 4;
        switch (rotation) {
            case 0:
                madctl = MADCTL_MX | MADCTL_BGR;
                m_width  = ILI9341_TFTWIDTH;
                m_height = ILI9341_TFTHEIGHT;
                break;
            case 1:
                madctl = MADCTL_MV | MADCTL_BGR;
                m_width  = ILI9341_TFTHEIGHT;
                m_height = ILI9341_TFTWIDTH;
                break;
            case 2:
                madctl = MADCTL_MY | MADCTL_BGR;
                m_width  = ILI9341_TFTWIDTH;
                m_height = ILI9341_TFTHEIGHT;
                break;
            case 3:
                madctl = MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR;
                m_width  = ILI9341_TFTHEIGHT;
                m_height = ILI9341_TFTWIDTH;
                break;
        }
        writeCommand(ILI9341_MADCTL, &madctl, 1);
        xSemaphoreGive(m_spiMutex);
    }
}

void ILI9341::invertDisplay(bool i) {
    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        writeCommand(i ? ILI9341_INVON : ILI9341_INVOFF);
        xSemaphoreGive(m_spiMutex);
    }
}

void ILI9341::scrollTo(uint16_t y) {
    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        uint8_t data[] = { (uint8_t)(y >> 8), (uint8_t)(y & 0xFF) };
        writeCommand(ILI9341_VSCRSADD, data, 2);
        xSemaphoreGive(m_spiMutex);
    }
}

void ILI9341::setScrollMargins(uint16_t top, uint16_t bottom) {
    if (xSemaphoreTake(m_spiMutex, portMAX_DELAY) == pdTRUE) {
        uint16_t middle = ILI9341_TFTHEIGHT - top - bottom;
        uint8_t data[] = {
            (uint8_t)(top >> 8),    (uint8_t)(top & 0xFF),
            (uint8_t)(middle >> 8), (uint8_t)(middle & 0xFF),
            (uint8_t)(bottom >> 8), (uint8_t)(bottom & 0xFF)
        };
        writeCommand(ILI9341_VSCRDEF, data, 6);
        xSemaphoreGive(m_spiMutex);
    }
}

uint16_t ILI9341::width() const  { return m_width; }
uint16_t ILI9341::height() const { return m_height; }
