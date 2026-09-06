*** Settings ***
Documentation     Pico W firmware smoke tests and hardware bring-up checks.
Library           OperatingSystem
Library           Process

*** Variables ***
${PROJECT_ROOT}    ${CURDIR}${/}..
${BUILD_DIR}       ${PROJECT_ROOT}${/}build-robot
${BOARD}          pico_w
${UF2_FILE}       ${BUILD_DIR}${/}ili9341_pico.uf2

*** Test Cases ***
Pico Source Exposes LVGL Display Path
    [Tags]    source
    ${main}=    Get File    ${PROJECT_ROOT}${/}main${/}pico_main.cpp
    ${driver}=    Get File    ${PROJECT_ROOT}${/}main${/}pico_ili9341.cpp
    Should Contain    ${main}    lv_init()
    Should Contain    ${main}    lv_display_set_flush_cb
    Should Contain    ${main}    ili9341_init()
    Should Contain    ${driver}    spi_write_blocking
    Should Contain    ${driver}    lv_display_flush_ready

Pico Pin Map Matches Display Wiring
    [Tags]    source
    ${pins}=    Get File    ${PROJECT_ROOT}${/}main${/}pins_pico2w.h
    Should Contain    ${pins}    #define ILI9341_PIN_MISO 16
    Should Contain    ${pins}    #define ILI9341_PIN_CS   17
    Should Contain    ${pins}    #define ILI9341_PIN_SCK  18
    Should Contain    ${pins}    #define ILI9341_PIN_MOSI 19
    Should Contain    ${pins}    #define ILI9341_PIN_DC   20
    Should Contain    ${pins}    #define ILI9341_PIN_RST  21

Configure Pico W Firmware
    [Tags]    build
    Configure Pico W Build

Build Pico W Firmware
    [Tags]    build
    Configure Pico W Build
    ${result}=    Run Process
    ...    cmake
    ...    --build
    ...    ${BUILD_DIR}
    ...    -j
    ...    4
    ...    stdout=PIPE
    ...    stderr=STDOUT
    Log    ${result.stdout}
    Should Be Equal As Integers    ${result.rc}    0

UF2 Firmware Is Generated
    [Tags]    build
    File Should Exist    ${UF2_FILE}
    ${size}=    Get File Size    ${UF2_FILE}
    Should Be True    ${size} > 0

Pico Board Is Visible To Picotool
    [Tags]    hardware
    ${result}=    Run Process    picotool    info    stdout=PIPE    stderr=STDOUT
    Log    ${result.stdout}
    Should Be Equal As Integers    ${result.rc}    0

Flash Pico W Firmware
    [Tags]    hardware    flash
    File Should Exist    ${UF2_FILE}
    ${load}=    Run Process    picotool    load    -f    ${UF2_FILE}    stdout=PIPE    stderr=STDOUT
    Log    ${load.stdout}
    Should Be Equal As Integers    ${load.rc}    0
    ${reboot}=    Run Process    picotool    reboot    stdout=PIPE    stderr=STDOUT
    Log    ${reboot.stdout}
    Should Be Equal As Integers    ${reboot.rc}    0

*** Keywords ***
Configure Pico W Build
    ${result}=    Run Process
    ...    cmake
    ...    -S
    ...    ${PROJECT_ROOT}
    ...    -B
    ...    ${BUILD_DIR}
    ...    -G
    ...    Ninja
    ...    -DPICO_BOARD=${BOARD}
    ...    stdout=PIPE
    ...    stderr=STDOUT
    Log    ${result.stdout}
    Should Be Equal As Integers    ${result.rc}    0