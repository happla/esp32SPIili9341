/*
 *      Author: Hannu Pham
 *      Written for 2.4" TFT LCD with touchscreen breakout w/microsd socket - ILI9341
 *      Some parts are copied from @file Adafruit_ILI9341.cpp to fit raspberry pi pico 2 w
 *https://raw.githubusercontent.com/adafruit/Adafruit_ILI9341/refs/heads/master/Adafruit_ILI9341.cpp
 */



#ifndef PINS_PICO2W_H
#define PINS_PICO2W_H

// Define hardware SPI block
#define ILI9341_SPI      spi0
#define ILI9341_TFTWIDTH  320
#define ILI9341_TFTHEIGHT 240

// Display pin mappings 
#define ILI9341_PIN_MISO 16 //RX
#define ILI9341_PIN_CS   17 // Display chip select
#define ILI9341_PIN_SCK  18 // Clock
#define ILI9341_PIN_MOSI 19 // TX
#define ILI9341_PIN_DC   20 // Data/command
#define ILI9341_PIN_RST  21 // Reset
#define ILI9341_PIN_LED  22 // Backlight (GPIO)

// Touchscreen Pin mappings 
#define ILI9341_PIN_TRQ 14 // INT
#define ILI9341_PIN_TDO ILI9341_PIN_MISO // shared miso gp16
#define ILI9341_PIN_TDIN ILI9341_PIN_MOSI //shared mosi gp19
#define ILI9341_PIN_TCS 15 // touch cs
#define ILI9341_PIN_TCLK ILI9341_PIN_SCK // shared sck gp18

#endif // PINS_PICO2W_H
