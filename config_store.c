#include "config_store.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>

// Assume standard 2MB if not defined, but PICO_FLASH_SIZE_BYTES should be defined by the board
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
#endif

// We use the very last sector of flash for configuration
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

sys_config_t sys_config;

void config_load(void) {
    // Read from memory-mapped flash
    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    
    sys_config_t *saved_config = (sys_config_t *)flash_target_contents;
    
    if (saved_config->magic == CONFIG_MAGIC) {
        memcpy(&sys_config, saved_config, sizeof(sys_config_t));
    } else {
        // No valid config found
        sys_config.magic = 0;
        memset(sys_config.wifi_ssid, 0, sizeof(sys_config.wifi_ssid));
        memset(sys_config.wifi_password, 0, sizeof(sys_config.wifi_password));
    }
}

void config_save(void) {
    sys_config.magic = CONFIG_MAGIC;

    // Buffer to hold a full page (required by flash programming)
    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0, FLASH_PAGE_SIZE);
    memcpy(buffer, &sys_config, sizeof(sys_config_t));

    // Disable interrupts before erasing/writing flash
    uint32_t interrupts = save_and_disable_interrupts();
    
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    
    restore_interrupts(interrupts);
}
