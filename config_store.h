#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_MAGIC 0x4E414E4F // "NANO"

typedef struct {
    uint32_t magic;
    char wifi_ssid[32];
    char wifi_password[64];
} sys_config_t;

extern sys_config_t sys_config;

void config_load(void);
void config_save(void);

#endif
