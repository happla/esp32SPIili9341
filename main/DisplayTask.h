/*
 *      Author: Hannu Pham
 *      Ported for ESP32-WROOM32 - 2.4" TFT LCD (ILI9341 + XPT2046 Touch)
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FreeRTOS task entry point for display rendering and touch handling.
 * @param pvParameters Standard FreeRTOS task parameter pointer (unused).
 */
void display_task(void *pvParameters);

#ifdef __cplusplus
}
#endif
