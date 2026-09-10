#include "pico_storage.h"

#include <stddef.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/flash.h"
#include "pico/stdlib.h"

namespace {

constexpr uint32_t settings_address = XIP_BASE + PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
constexpr uint32_t settings_offset = settings_address - XIP_BASE;
constexpr uint32_t settings_magic = 0x5341554EU;
constexpr uint16_t settings_version = 1;

struct StoredSettings {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    PicoSettings settings;
    uint32_t checksum;
};

uint32_t checksum(const StoredSettings &stored) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&stored.version);
    const size_t length = offsetof(StoredSettings, checksum) - offsetof(StoredSettings, version);
    uint32_t result = 2166136261u;
    for (size_t index = 0; index < length; ++index) {
        result ^= bytes[index];
        result *= 16777619u;
    }
    return result;
}

bool valid(const StoredSettings &stored) {
    return stored.magic == settings_magic &&
           stored.version == settings_version &&
           stored.size == sizeof(PicoSettings) &&
           stored.checksum == checksum(stored);
}

}

void pico_settings_defaults(PicoSettings *settings) {
    *settings = {};
    settings->calibration_min_x = 600;
    settings->calibration_max_x = 3800;
    settings->calibration_min_y = 600;
    settings->calibration_max_y = 3800;
    settings->target_temperature = 85;
    settings->locked = 1;
}

bool pico_settings_load(PicoSettings *settings) {
    const auto *stored = reinterpret_cast<const StoredSettings *>(settings_address);
    if (!valid(*stored)) {
        pico_settings_defaults(settings);
        return false;
    }
    *settings = stored->settings;
    return true;
}

bool pico_settings_save(const PicoSettings *settings) {
    StoredSettings stored = {
        settings_magic,
        settings_version,
        sizeof(PicoSettings),
        *settings,
        0
    };
    stored.checksum = checksum(stored);

    uint8_t page[FLASH_PAGE_SIZE] = {};
    memcpy(page, &stored, sizeof(stored));
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(settings_offset, FLASH_SECTOR_SIZE);
    flash_range_program(settings_offset, page, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);
    return memcmp(reinterpret_cast<const void *>(settings_address), &stored, sizeof(stored)) == 0;
}