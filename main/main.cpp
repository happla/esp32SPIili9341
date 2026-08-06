#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DisplayTask.h"

extern "C" void app_main(void) {
    // ESP-IDF automatically starts the FreeRTOS scheduler before calling app_main()
    xTaskCreate(
        display_task,
        "DisplayTask",
        4096,
        NULL,
        1,
        NULL
    );
}
