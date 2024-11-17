/*
 * MIT License
 *
 * Copyright (c) 2024 SplinterGU
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credits:
 * - Developed by SplinterGU
 * - GitHub: https://github.com/SplinterGU/ESPeccyFlashTool
 * - This project is a tool for flashing firmware to ESP32 devices.
 *
 * Usage:
 * especcy_flash_tool [options]
 * Options:
 *   -h            This help message.
 *   -nopsram      Use no PSRAM firmware.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef _WIN32
#include <sys/wait.h>
#endif

#include "download_file.h"
#include "esp32_detect.h"

#ifdef _WIN32
    #define ESPUTIL             "esputil.exe"
#else
    #define ESPUTIL             "esputil_linux"
#endif

// Function to show the help message
void show_help() {
    printf("Usage: especcy_flash_tool [options]\n");
    printf("Options:\n");
    printf("  -h            This help\n");
    printf("  -nopsram      Use no PSRAM firmware\n");
    printf("\n");
    printf("GitHub: https://github.com/SplinterGU/ESPeccyFlashTool\n");
}

// Function to flash the firmware
int flash_firmware(const char *firmware_name, const char *port_name) {
    const char *command = NULL;

    // Concatenate the firmware name to the command
    char full_command[512];  // Assuming the command doesn't exceed 512 characters
#ifdef _WIN32
    snprintf(full_command, sizeof(full_command), "esputil.exe -fspi 6,17,8,11,16 -p %s -b 115200 flash 0x0 %s", port_name, firmware_name);
#else
    snprintf(full_command, sizeof(full_command), "./esputil_linux -fspi 6,17,8,11,16 -p %s -b 115200 flash 0x0 %s", port_name, firmware_name);
#endif

    int ret_code = system(full_command);

#ifdef _WIN32
    if (ret_code == 0) {
        // On Windows, a return of 0 means success
        return 0;
    } else {
        fprintf(stderr, "Command failed with code %d\n", ret_code);
        return -1;
    }
#else
    if (ret_code == -1) {
        perror("Error executing command");
        return -1;
    } else if (WIFEXITED(ret_code) && WEXITSTATUS(ret_code) == 0) {
        return 0;
    } else {
        fprintf(stderr, "Command failed with code %d\n", WEXITSTATUS(ret_code));
        return -1;
    }
#endif
}

int main(int argc, char *argv[]) {
    printf("ESPeccy Flash Tool\n");
    printf("Copyright (c) 2024 SplinterGU\n\n");

    // Check if the -h option was passed
    if (argc > 1 && strcmp(argv[1], "-h") == 0) {
        show_help();
        return 0; // Show help and exit
    }

    const char *firmware_name = "complete_firmware-psram.bin";

    if (argc > 1 && strcmp(argv[1], "-nopsram") == 0) {
        firmware_name = "complete_firmware-nopsram.bin";
    }

    const char *port_name = find_esp32_port();

    if (!port_name) return -1;

    // Download the firmware file
    if (download_file("SplinterGU/ESPeccy", firmware_name) != 0) {
        fprintf(stderr, "Firmware download error... aborting...\n");
        return 1;
    }

    // Download the flashing tool
    if (download_file("SplinterGU/esputil", ESPUTIL) != 0) {
        fprintf(stderr, "Flash tool download error... aborting...\n");
        return 1;
    }

#ifndef _WIN32
    // Try changing the file permissions in Linux
    if (chmod(ESPUTIL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        perror("Can't assign execution perms\n");
        return 1;
    }
#endif

    // Run the flashing process and check if it was successful
    if (flash_firmware(firmware_name, port_name) != 0) {
        fprintf(stderr, "Error! can't flash the firmware\n");
        return 1; // Error
    }

    return 0; // Success
}
