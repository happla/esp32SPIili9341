cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD "pico_w" CACHE STRING "Pico board target")

if(NOT DEFINED ENV{PICO_SDK_PATH})
	message(FATAL_ERROR "PICO_SDK_PATH must point to the Pico SDK")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(ili9341_pico C CXX ASM)

pico_sdk_init()

include(FetchContent)

FetchContent_Declare(
	freertos_kernel
	GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
	GIT_TAG V11.1.0
	GIT_SHALLOW TRUE
)
FetchContent_GetProperties(freertos_kernel)
if(NOT freertos_kernel_POPULATED)
	FetchContent_Populate(freertos_kernel)
endif()

FetchContent_Declare(
	lvgl
	GIT_REPOSITORY https://github.com/lvgl/lvgl.git
	GIT_TAG v9.2.2
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(lvgl)
target_include_directories(lvgl PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/main
	${freertos_kernel_SOURCE_DIR}/include
	${freertos_kernel_SOURCE_DIR}/portable/ThirdParty/GCC/RP2040/include
)
target_link_libraries(lvgl PUBLIC pico_stdlib)

set(FREERTOS_KERNEL_DIR ${freertos_kernel_SOURCE_DIR})
set(FREERTOS_PORT_DIR ${FREERTOS_KERNEL_DIR}/portable/ThirdParty/GCC/RP2040)
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/main)
set(FREERTOS_KERNEL_PATH ${FREERTOS_KERNEL_DIR} CACHE PATH "FreeRTOS kernel path")
include(${FREERTOS_PORT_DIR}/FreeRTOS_Kernel_import.cmake)

add_executable(ili9341_pico
	main/pico_main.cpp
	main/pico_ili9341.cpp
)

target_include_directories(ili9341_pico PRIVATE main)
target_compile_definitions(ili9341_pico PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_link_libraries(ili9341_pico PRIVATE
	pico_stdlib
	hardware_spi
	lvgl
	FreeRTOS-Kernel
	FreeRTOS-Kernel-Heap4
)

pico_enable_stdio_usb(ili9341_pico 1)
pico_enable_stdio_uart(ili9341_pico 0)
pico_add_extra_outputs(ili9341_pico)
