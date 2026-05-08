#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_MAGIC 0x4E414E4F // "NANO"

typedef struct {
    uint32_t magic;
    uint32_t firmware_marker;
    char wifi_ssid[32];
    char wifi_password[64];
} sys_config_t;

extern sys_config_t sys_config;

uint32_t get_firmware_marker(void);

void config_load(void);
void config_save(void);

#endif
