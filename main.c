#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "kernel.h"
#include "tcp_server.h"
#include "usb_fs.h"
#include "config_store.h"
#include "tusb.h"
#include "ff.h"

// Simple heartbeat task to show the OS is running
static uint32_t print_last_wake = 0;
void print_task(void) {
    uint32_t now = time_us_32() / 1000;
    if (now - print_last_wake >= 5000) {
        print_last_wake = now;
        printf("NanoOS Heartbeat... Current Time: %lu ms\n", now);
    }
}

// USB Host processing task
void usb_host_task(void) {
    tuh_task(); // Process TinyUSB Host events
}

void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) str[len-1] = '\0';
    if (len > 1 && (str[len-2] == '\n' || str[len-2] == '\r')) str[len-2] = '\0';
}

static char* f_gets_custom(char* buff, int len, FIL* fp) {
    int i = 0;
    UINT br;
    char c;
    while (i < len - 1) {
        if (f_read(fp, &c, 1, &br) != FR_OK || br == 0) {
            break;
        }
        buff[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    buff[i] = '\0';
    return (i > 0) ? buff : NULL;
}

void check_usb_for_config(void) {
    printf("Waiting 3 seconds to check for USB drive...\n");
    for (int i = 0; i < 300; i++) {
        tuh_task();
        sleep_ms(10);
    }

    if (usb_is_mounted) {
        printf("USB Drive detected! Checking for wifi.txt...\n");
        FIL file;
        if (f_open(&file, "wifi.txt", FA_READ) == FR_OK) {
            char line[64];
            
            // Read SSID (line 1)
            if (f_gets_custom(line, sizeof(line), &file)) {
                trim_newline(line);
                strncpy(sys_config.wifi_ssid, line, sizeof(sys_config.wifi_ssid) - 1);
            }
            
            // Read Password (line 2)
            if (f_gets_custom(line, sizeof(line), &file)) {
                trim_newline(line);
                strncpy(sys_config.wifi_password, line, sizeof(sys_config.wifi_password) - 1);
            }
            
            f_close(&file);
            
            printf("Loaded credentials from wifi.txt! Saving to internal flash...\n");
            config_save();
        } else {
            printf("No wifi.txt found on USB.\n");
        }
    } else {
        printf("No USB drive detected during boot.\n");
    }
}

void load_default_wifi_config(void) {
    uint32_t current_marker = get_firmware_marker();
    if (sys_config.magic == CONFIG_MAGIC && sys_config.firmware_marker == current_marker) {
        printf("Valid Wi-Fi config exists for this firmware, skipping defaults.\n");
        return;
    }

    printf("Fresh flash or new firmware detected, loading embedded wifi.txt defaults...\n");

    // Include embedded wifi.txt content
    extern const char _binary_wifi_txt_start[];
    extern const char _binary_wifi_txt_end[];
    extern const size_t _binary_wifi_txt_size;

    const char *content = _binary_wifi_txt_start;
    size_t content_size = (size_t)(_binary_wifi_txt_end - _binary_wifi_txt_start);

    // Parse the embedded content (similar to USB parsing)
    char line[64];
    int line_idx = 0;
    int char_idx = 0;

    for (size_t i = 0; i < content_size && i < sizeof(line) - 1; i++) {
        if (content[i] == '\n' || content[i] == '\r' || content[i] == 0) {
            line[char_idx] = '\0';

            if (line_idx == 0 && strlen(line) > 0) {
                // SSID (line 1)
                strncpy(sys_config.wifi_ssid, line, sizeof(sys_config.wifi_ssid) - 1);
                printf("Default SSID: %s\n", sys_config.wifi_ssid);
            } else if (line_idx == 1 && strlen(line) > 0) {
                // Password (line 2)
                strncpy(sys_config.wifi_password, line, sizeof(sys_config.wifi_password) - 1);
                printf("Default Password: [HIDDEN]\n");
            }

            line_idx++;
            char_idx = 0;

            // Skip any additional CR/LF characters
            while (i + 1 < content_size && (content[i + 1] == '\n' || content[i + 1] == '\r')) {
                i++;
            }
        } else {
            line[char_idx++] = content[i];
        }
    }

    // Handle last line if no trailing newline
    if (char_idx > 0 && line_idx < 2) {
        line[char_idx] = '\0';
        if (line_idx == 0) {
            strncpy(sys_config.wifi_ssid, line, sizeof(sys_config.wifi_ssid) - 1);
            printf("Default SSID: %s\n", sys_config.wifi_ssid);
        } else if (line_idx == 1) {
            strncpy(sys_config.wifi_password, line, sizeof(sys_config.wifi_password) - 1);
            printf("Default Password: [HIDDEN]\n");
        }
    }

    if (strlen(sys_config.wifi_ssid) > 0) {
        printf("Default Wi-Fi credentials loaded successfully!\n");
        config_save();
    } else {
        printf("Failed to load default Wi-Fi credentials.\n");
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("\n\n--- Starting NanoOS (Pico 2 W Wi-Fi + Internal Flash Edition) ---\n");

    // Initialize Config Store
    config_load();

    // Load default Wi-Fi config if needed
    load_default_wifi_config();

    // Initialize USB Host
    usb_fs_init();

    // Check USB for overriding config
    check_usb_for_config();

    if (sys_config.magic != CONFIG_MAGIC || strlen(sys_config.wifi_ssid) == 0) {
        printf("\nERROR: No Wi-Fi credentials found!\n");
        printf("Please insert a USB drive with a 'wifi.txt' file (SSID on line 1, password on line 2) and reboot.\n");
        while (1) {
            tuh_task(); // Keep USB alive just in case
            sleep_ms(10);
        }
    }

    // Initialize the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Failed to initialize cyw43_arch\n");
        return -1;
    }
    
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi SSID: '%s'...\n", sys_config.wifi_ssid);
    if (cyw43_arch_wifi_connect_timeout_ms(sys_config.wifi_ssid, sys_config.wifi_password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to Wi-Fi. Check your SSID and password.\n");
        return -1;
    }

    printf("Connected! IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    printf("To connect, use PuTTY 'Raw' mode to this IP on port 1234.\n");

    // Start the TCP Command Line Server
    tcp_server_init();

    // Initialize OS and start tasks
    os_init();
    os_add_task(print_task);
    os_add_task(usb_host_task);

    // Enter scheduler loop
    os_run();

    cyw43_arch_deinit(); // Never reached
    return 0;
}
