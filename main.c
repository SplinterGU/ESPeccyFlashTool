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
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef _WIN32
#include <sys/wait.h>
#include <termios.h>
#endif

#include "download_file.h"
#include "esp32_detect.h"

#ifdef _WIN32
    #define ESPUTIL             "esputil.exe"
#elif defined(__APPLE__)
    #define ESPUTIL             "esputil_macos"
#else
    #define ESPUTIL             "esputil_linux"
#endif

int get_baud_rate(int baud) {
#ifdef _WIN32
    // En Windows, solo devolvemos el valor porque se usa directamente
    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
        case 230400:
        case 460800:
        case 500000:
        case 576000:
        case 921600:
        case 1000000:
        case 1152000:
        case 1500000:
        case 2000000:
        case 2500000:
        case 3000000:
        case 3500000:
        case 4000000:
            return baud;
    }
#else
    // En Unix-like, retornamos las constantes definidas en termios.h
    switch (baud) {
        case 9600:    return B9600;
        case 19200:   return B19200;
        case 38400:   return B38400;
        case 57600:   return B57600;
        case 115200:  return B115200;
        case 230400:  return B230400;
#ifndef __APPLE__
        case 460800:  return B460800;
        case 500000:  return B500000;
        case 576000:  return B576000;
        case 921600:  return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
#endif
    }
#endif
    fprintf(stderr, "Unsupported baud rate: %d\n", baud);
    return -1;
}

// Function to show the help message
void show_help() {
    printf("Usage: especcy_flash_tool [options]\n");
    printf("Options:\n");
    printf("  -h                This help\n");
    printf("  -nopsram          Use no PSRAM firmware\n");
    printf("  -b|-baud [rate]   Specify baud rate (default: 115200)\n");
    printf("                    Supported rates:\n");
    printf("                      9600, 19200, 38400, 57600, 115200, 230400\n");
#ifndef __APPLE__
    printf("                      460800, 500000, 576000, 921600, 1000000\n");
    printf("                      1152000, 1500000, 2000000, 2500000, 3000000\n");
    printf("                      3500000, 4000000\n");
#endif
    printf("\n");
    printf("GitHub: https://github.com/SplinterGU/ESPeccyFlashTool\n");
}

// Function to flash the firmware
int flash_firmware(const char *firmware_name, const char *port_name, int baud) {
    char full_command[512];
    snprintf(full_command, sizeof(full_command),
#ifndef _WIN32
                                "./"
#endif
                                ESPUTIL " -p %s -b %d flash 0x0 %s", port_name, baud, firmware_name);
    int ret_code = system(full_command);

#ifdef _WIN32
    return (ret_code == 0) ? 0 : -1;
#else
    if (ret_code == -1) {
        perror("Error executing command");
        return -1;
    }
    return (WIFEXITED(ret_code) && WEXITSTATUS(ret_code) == 0) ? 0 : -1;
#endif
}

int main(int argc, char *argv[]) {
    printf("ESPeccy Flash Tool - v1.2.1\n");
    printf("Copyright (c) 2024 SplinterGU\n\n");

    const char *firmware_name = "complete_firmware-psram.bin";
    int baud_rate = 115200;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0) {
            show_help();
            return 0;
        } else if (strcmp(argv[i], "-nopsram") == 0) {
            firmware_name = "complete_firmware-nopsram.bin";
        } else if (strcmp(argv[i], "-baud") == 0 || strcmp(argv[i], "-b") == 0) {
            if (i + 1 < argc) {
                baud_rate = atoi(argv[++i]);
                if (get_baud_rate(baud_rate) == -1) {
                    fprintf(stderr, "Invalid baud rate specified: %d\n", baud_rate);
                    return 1;
                }
            } else {
                fprintf(stderr, "Missing value for -baud option\n");
                return 1;
            }
        }
    }

    const char *port_name = find_esp32_port();
    if (!port_name) return -1;

    if (download_file("SplinterGU/ESPeccy", firmware_name) != 0) {
        fprintf(stderr, "Firmware download error... aborting...\n");
        return 1;
    }

    if (download_file("SplinterGU/esputil", ESPUTIL) != 0) {
        fprintf(stderr, "Flash tool download error... aborting...\n");
        return 1;
    }

#ifndef _WIN32
    if (chmod(ESPUTIL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0) {
        perror("Can't assign execution perms\n");
        return 1;
    }
#endif

    if (flash_firmware(firmware_name, port_name, baud_rate) != 0) {
        fprintf(stderr, "Error! can't flash the firmware\n");
        return 1;
    }

    return 0;
}
