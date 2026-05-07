#include "cli.h"
#include "tcp_server.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static bool edit_mode = false;
static FIL edit_file;

void cli_process_command(const char *cmd) {
    if (edit_mode) {
        if (strcmp(cmd, "SAVE") == 0) {
            f_close(&edit_file);
            edit_mode = false;
            tcp_printf("\r\nFile saved and closed. Returning to normal CLI mode.\r\n");
            return;
        }
        
        // Write the line to the file
        UINT bytes_written;
        f_write(&edit_file, cmd, strlen(cmd), &bytes_written);
        f_write(&edit_file, "\n", 1, &bytes_written);
        tcp_printf("  (written) %s\r\n", cmd);
        return;
    }

    if (strcmp(cmd, "help") == 0) {
        tcp_printf("Available commands:\r\n");
        tcp_printf("  help            - Show this help message\r\n");
        tcp_printf("  led on/off      - Turn Pico W LED on/off\r\n");
        tcp_printf("  uptime          - Show system uptime\r\n");
        tcp_printf("  ls              - List files on USB drive\r\n");
        tcp_printf("  read <file>     - Read contents of a file\r\n");
        tcp_printf("  write <text>    - Append text to nano_os.txt\r\n");
        tcp_printf("  edit <file>     - Open a file in line-editor mode\r\n");
    } 
    else if (strcmp(cmd, "led on") == 0) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        tcp_printf("LED turned ON.\r\n");
    } 
    else if (strcmp(cmd, "led off") == 0) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        tcp_printf("LED turned OFF.\r\n");
    } 
    else if (strcmp(cmd, "uptime") == 0) {
        uint32_t ms = time_us_32() / 1000;
        tcp_printf("Uptime: %lu milliseconds\r\n", ms);
    } 
    else if (strcmp(cmd, "ls") == 0) {
        DIR dir;
        FILINFO fno;
        if (f_opendir(&dir, "0:") == FR_OK) {
            tcp_printf("Directory listing for USB Drive:\r\n");
            while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
                if (fno.fattrib & AM_DIR) {
                    tcp_printf("  [DIR]  %s\r\n", fno.fname);
                } else {
                    tcp_printf("  [FILE] %-20s %lu bytes\r\n", fno.fname, fno.fsize);
                }
            }
            f_closedir(&dir);
        } else {
            tcp_printf("Error: Could not read USB drive. Is it inserted and formatted to FAT32?\r\n");
        }
    }
    else if (strncmp(cmd, "read ", 5) == 0) {
        const char *filename = cmd + 5;
        FIL file;
        if (f_open(&file, filename, FA_READ) == FR_OK) {
            tcp_printf("--- Contents of %s ---\r\n", filename);
            char buffer[64];
            UINT bytes_read;
            while (f_read(&file, buffer, sizeof(buffer)-1, &bytes_read) == FR_OK && bytes_read > 0) {
                buffer[bytes_read] = '\0';
                tcp_printf("%s", buffer);
            }
            tcp_printf("\r\n--- End of file ---\r\n");
            f_close(&file);
        } else {
            tcp_printf("Error: Could not open file '%s'.\r\n", filename);
        }
    }
    else if (strncmp(cmd, "edit ", 5) == 0) {
        const char *filename = cmd + 5;
        if (f_open(&edit_file, filename, FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS) == FR_OK) {
            edit_mode = true;
            tcp_printf("\r\n--- Line Editor: %s ---\r\n", filename);
            tcp_printf("Type your text. Each line is saved immediately.\r\n");
            tcp_printf("Type exactly 'SAVE' on a blank line to exit and save.\r\n");
        } else {
            tcp_printf("Error: Could not open file '%s' for editing.\r\n", filename);
        }
    }
    else if (strncmp(cmd, "write ", 6) == 0) {
        const char *text = cmd + 6;
        FIL file;
        if (f_open(&file, "nano_os.txt", FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS) == FR_OK) {
            UINT bytes_written;
            f_write(&file, text, strlen(text), &bytes_written);
            f_write(&file, "\n", 1, &bytes_written);
            f_close(&file);
            tcp_printf("Successfully wrote to nano_os.txt on USB drive!\r\n");
        } else {
            tcp_printf("Error: Could not write to USB drive.\r\n");
        }
    }
    else {
        tcp_printf("Unknown command: '%s'. Type 'help'.\r\n", cmd);
    }
}
