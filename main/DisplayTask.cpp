#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ILI9341.h"
#include "pins_esp32.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <stdio.h>

#define TOUCH_CMD_X 0x90 
#define TOUCH_CMD_Y 0xD0 

static ILI9341 lcd(ILI9341_PIN_CS, ILI9341_PIN_DC, ILI9341_PIN_RST);
static spi_device_handle_t touch_spiDev = NULL;

// Global calibration variables initialized with safe defaults
static uint16_t cal_minX = 600, cal_maxX = 3800;
static uint16_t cal_minY = 600, cal_maxY = 3800;

// Sauna State Variables
static int currentTemp = 78;   // Simulated or sensor reading
static int targetTemp = 85;    // Target temperature
static bool heaterOn = false;

static void touch_init() {
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << ILI9341_PIN_TCS);
    gpio_config(&io_conf);
    gpio_set_level(ILI9341_PIN_TCS, 1);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pin_bit_mask = (1ULL << ILI9341_PIN_TRQ);
    gpio_config(&io_conf);

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 2000000;
    devcfg.mode = 0;
    devcfg.spics_io_num = -1;
    devcfg.queue_size = 1;

    spi_bus_add_device(ILI9341_SPI_HOST, &devcfg, &touch_spiDev);
}

static uint16_t touch_read_adc(uint8_t cmd) {
    if (touch_spiDev == NULL) return 0;
    uint8_t tx[3] = { cmd, 0x00, 0x00 };
    uint8_t rx[3] = { 0 };

    gpio_set_level(ILI9341_PIN_CS, 1);
    gpio_set_level(ILI9341_PIN_TCS, 0);

    spi_transaction_t t = {};
    t.length = 24;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    spi_device_polling_transmit(touch_spiDev, &t);

    gpio_set_level(ILI9341_PIN_TCS, 1);
    return ((rx[1] << 8) | rx[2]) >> 3;
}

static bool touch_is_pressed() {
    return (gpio_get_level(ILI9341_PIN_TRQ) == 0);
}

struct TouchPoint {
    uint16_t x;
    uint16_t y;
    bool pressed;
};

static TouchPoint waitForTouch() {
    TouchPoint p = {0, 0, false};
    while (!touch_is_pressed()) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // Debounce settle

    (void)touch_read_adc(TOUCH_CMD_X);
    uint16_t rx = touch_read_adc(TOUCH_CMD_X);
    uint16_t ry = touch_read_adc(TOUCH_CMD_Y);

    p.x = rx;
    p.y = ry;
    p.pressed = true;

    while (touch_is_pressed()) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelay(pdMS_TO_TICKS(50));
    return p;
}

// Robust mapping function that safely handles reversed min/max bounds
static int32_t mapRange(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    if (in_min < in_max) {
        if (x < in_min) x = in_min;
        if (x > in_max) x = in_max;
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    } else {
        // Handle inverted ranges where min > max
        if (x > in_min) x = in_min;
        if (x < in_max) x = in_max;
        return (in_min - x) * (out_max - out_min) / (in_min - in_max) + out_min;
    }
}

static void runCalibration() {
    lcd.fillScreen(ILI9341_BLACK);
    lcd.drawString(20, 40, "TOUCH CALIBRATION", ILI9341_WHITE, ILI9341_BLACK, 2);

    //  Top-Left Target
    lcd.fillRect(10, 10, 20, 20, ILI9341_RED);
    lcd.drawString(40, 15, "TOUCH-TOP-LEFT-MARKER", ILI9341_GREEN, ILI9341_BLACK, 1);
    TouchPoint p1 = waitForTouch();
    
    // Bottom-Right Target
    lcd.fillRect(290, 210, 20, 20, ILI9341_RED);
    lcd.drawString(40, 110, "TOUCH BOTTOM-RIGHT MARKER", ILI9341_GREEN, ILI9341_BLACK, 1);
    TouchPoint p2 = waitForTouch();

    // Store measured bounds accurately
    cal_minX = p1.x; // Top-Left X
    cal_maxX = p2.x; // Bottom-Right X
    cal_minY = p1.y; // Top-Left Y
    cal_maxY = p2.y; // Bottom-Right Y

    printf("CALIBRATION COMPLETE! X_min:%d, X_max:%d, Y_min:%d, Y_max:%d\n", cal_minX, cal_maxX, cal_minY, cal_maxY);
}

static TouchPoint touch_get_point() {
    TouchPoint p = { 0, 0, false };
    if (!touch_is_pressed()) return p;

    (void)touch_read_adc(TOUCH_CMD_X);
    uint16_t rawX = touch_read_adc(TOUCH_CMD_X);
    uint16_t rawY = touch_read_adc(TOUCH_CMD_Y);

    if (rawX > 100 && rawX < 4000 && rawY > 100 && rawY < 4000) {
        p.pressed = true;
        
        // Correct mapping matching landscape orientation bounds
        int mappedX = mapRange(rawX, cal_minX, cal_maxX, 0, 320);
        int mappedY = mapRange(rawY, cal_minY, cal_maxY, 0, 240);

        p.x = mappedX;
        p.y = mappedY;
    }
    return p;
}

// Draw the static layout elements of the UI
static void drawUI() {
    lcd.fillScreen(ILI9341_BLACK);

    // Top Header Bar
    lcd.fillRect(0, 0, 320, 35, ILI9341_NAVY);
    lcd.drawString(10, 10, "SAUNA CONTROLLER", ILI9341_WHITE, ILI9341_NAVY, 2);

    // Current Temp Box (Left) 
    lcd.fillRect(15, 45, 140, 110, ILI9341_DARKGREY);
    lcd.fillRect(18, 48, 134, 104, ILI9341_BLACK); // Inner fill to create a border frame look
    lcd.drawString(25, 55, "CURRENT TEMP", ILI9341_LIGHTGREY, ILI9341_BLACK, 1);

    // Target Adjuster Buttons [+] and [-] (Right)
    lcd.fillRect(170, 45, 135, 50, ILI9341_DARKGREEN);
    lcd.drawString(230, 62, "+", ILI9341_WHITE, ILI9341_DARKGREEN, 3);

    lcd.fillRect(170, 105, 135, 50, ILI9341_MAROON);
    lcd.drawString(230, 122, "-", ILI9341_WHITE, ILI9341_MAROON, 3);

    // Bottom Action Buttons
    lcd.fillRect(15, 175, 145, 45, ILI9341_DARKGREY); // Heater Toggle Button
    lcd.fillRect(170, 175, 135, 45, ILI9341_PURPLE);   // Calibrate Button
    lcd.drawString(185, 190, "CALIBRATE", ILI9341_WHITE, ILI9341_PURPLE, 1);
}

// Refresh dynamic values (Temperatures & Status states) without redrawing full screen
static void updateUIValues() {
    char buf[16];

    // Current Temp Text
    snprintf(buf, sizeof(buf), "%d C  ", currentTemp);
    lcd.drawString(25, 85, buf, ILI9341_WHITE, ILI9341_BLACK, 3);

    // Target Temp Text
    snprintf(buf, sizeof(buf), "TARGET: %d C ", targetTemp);
    lcd.drawString(170, 160, buf, ILI9341_YELLOW, ILI9341_BLACK, 1);

    // Status / Heater Button State text
    if (heaterOn) {
        lcd.fillRect(15, 175, 145, 45, ILI9341_RED);
        lcd.drawString(30, 190, "HEATER: ON", ILI9341_WHITE, ILI9341_RED, 1);
        lcd.drawString(220, 10, "HEATING", ILI9341_RED, ILI9341_NAVY, 1);
    } else {
        lcd.fillRect(15, 175, 145, 45, ILI9341_DARKGREY);
        lcd.drawString(30, 190, "HEATER: OFF", ILI9341_WHITE, ILI9341_DARKGREY, 1);
        lcd.drawString(220, 10, "IDLE   ", ILI9341_LIGHTGREY, ILI9341_NAVY, 1);
    }
}
extern "C" void display_task(void *pvParameters) {
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << ILI9341_PIN_LED);
    gpio_config(&io_conf);
    gpio_set_level(ILI9341_PIN_LED, 1);

    lcd.begin(24000000);
    touch_init();
    lcd.setRotation(1); 

    runCalibration();

    drawUI();
    updateUIValues();

    char debugBuf[32];

    while (true) {
        TouchPoint p = touch_get_point();
        if (p.pressed) {
            snprintf(debugBuf, sizeof(debugBuf), "TOUCH X:%03d Y:%03d   ", p.x, p.y);
            lcd.drawString(15, 222, debugBuf, ILI9341_CYAN, ILI9341_BLACK, 1);

            // [+] Button Hitbox (Expanded slightly for easy tapping)
            if (p.x >= 160 && p.x <= 315 && p.y >= 40 && p.y <= 100) {
                if (targetTemp < 110) {
                    targetTemp++;
                    updateUIValues();
                }
                vTaskDelay(pdMS_TO_TICKS(150)); // Debounce
            } 
            // [-] Button Hitbox
            else if (p.x >= 160 && p.x <= 315 && p.y >= 101 && p.y <= 160) {
                if (targetTemp > 0) {
                    targetTemp--;
                    updateUIValues();
                }
                vTaskDelay(pdMS_TO_TICKS(150)); // Debounce
            } 
            // Heater Toggle Hitbox
            else if (p.x >= 15 && p.x <= 160 && p.y >= 175 && p.y <= 220) {
                heaterOn = !heaterOn;
                updateUIValues();
                vTaskDelay(pdMS_TO_TICKS(300));
            } 
            // Calibrate Button Hitbox
            else if (p.x >= 170 && p.x <= 305 && p.y >= 175 && p.y <= 220) {
                runCalibration();
                drawUI();
                updateUIValues();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
