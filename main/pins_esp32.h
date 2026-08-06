#ifndef PINS_ESP32_H
#define PINS_ESP32_H

#include "driver/gpio.h"
#include "driver/spi_master.h"

// Hardware SPI Host Selection (VSPI / SPI3_HOST)
#define ILI9341_SPI_HOST SPI3_HOST

// Display SPI Pin Mappings (VSPI Defaults)
#define ILI9341_PIN_MISO GPIO_NUM_19
#define ILI9341_PIN_MOSI GPIO_NUM_23
#define ILI9341_PIN_SCK  GPIO_NUM_18

// Display Control Pin Mappings
#define ILI9341_PIN_CS   GPIO_NUM_5
#define ILI9341_PIN_DC   GPIO_NUM_2
#define ILI9341_PIN_RST  GPIO_NUM_4
#define ILI9341_PIN_LED  GPIO_NUM_22

// Touchscreen Pin Mappings
#define ILI9341_PIN_TCS  GPIO_NUM_15
#define ILI9341_PIN_TRQ  GPIO_NUM_21
#define ILI9341_PIN_TDO  ILI9341_PIN_MISO // Shared MISO (GPIO 19)
#define ILI9341_PIN_TDIN ILI9341_PIN_MOSI // Shared MOSI (GPIO 23)
#define ILI9341_PIN_TCLK ILI9341_PIN_SCK  // Shared SCK  (GPIO 18)

#endif // PINS_ESP32_H
