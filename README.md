# TODO:
- use lvgl
  
```
esp32_display_project/
├── CMakeLists.txt              # Root CMake configuration
└── main/
    ├── CMakeLists.txt          # Component CMake configuration
    ├── DisplayTask.h           # Task header prototype
    ├── DisplayTask.cpp         # Task implementation (display + touch loop)
    ├── ILI9341.h               # ESP32 ILI9341 driver header
    ├── ILI9341.cpp             # ESP32 ILI9341 driver implementation
    ├── pins_esp32.h            # Pin definitions for ESP32-WROOM (VSPI)
    └── main.cpp                # app_main entry point
```

| Display |  ESP32-wroom pins |
| --- | --- |
| VCC | 3v3 |
| GND  |  GND |
| CS | GPIO5 |
| DC | GPIO2  |
| RST |  GPIO4 |
| MOSI |  GPIO23 |
| SCK | GPIO18 |
| MISO |  GPIO19 |
| LED | 3v3 |
| T_CS | GPIO15 |
| T_DIN |  GPIO23 |
| T_DO | GPIO 19 |
| T_IRQ | GPIO21 |

<div>
    <img src="https://github.com/happla/esp32SPIili9341/blob/main/IMG_1814.JPG" width="400">
</div>


