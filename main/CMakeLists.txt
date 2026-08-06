idf_component_register(
    SRCS "main.cpp" "ILI9341.cpp" "DisplayTask.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_driver_spi driver freertos
)
