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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define FD HANDLE
#else
    #include <fcntl.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <dirent.h>
    #define FD int
#endif

// Configuración de puerto serie
int configure_port(FD fd) {
#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific configuration
    DCB dcbSerialParams = {0};
    if (!GetCommState(fd, &dcbSerialParams)) return -1;

    // Configurar parámetros de la comunicación serial
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // Configurar la comunicación sin paridad, 1 bit de parada, y 8 bits de datos
    dcbSerialParams.fBinary = TRUE;              // Comunicación binaria
    dcbSerialParams.fParity = FALSE;             // Sin paridad
    dcbSerialParams.fOutxCtsFlow = FALSE;        // Sin flujo de control CTS
    dcbSerialParams.fOutxDsrFlow = FALSE;        // Sin flujo de control DSR
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE; // Control de flujo DTR
    dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE; // Control de flujo RTS
    dcbSerialParams.fOutX = FALSE;               // Sin control de flujo XON/XOFF
    dcbSerialParams.fInX = FALSE;                // Sin control de flujo XON/XOFF
    dcbSerialParams.fErrorChar = FALSE;          // Sin caracteres de error
    dcbSerialParams.fNull = FALSE;               // Sin caracteres nulos

    // Configurar el número de bits de datos y el número de bits de parada
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;                // 8 bits de datos
    dcbSerialParams.StopBits = ONESTOPBIT;       // 1 bit de parada
    dcbSerialParams.Parity = NOPARITY;           // Sin paridad

    if (!SetCommState(fd, &dcbSerialParams)) return -1;

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;  // Timeout entre caracteres
    timeouts.ReadTotalTimeoutConstant = 500;  // Timeout total de lectura
    timeouts.ReadTotalTimeoutMultiplier = 10;  // Multiplicador del timeout

    SetCommTimeouts(fd, &timeouts);
#else
    // Linux-specific configuration
    struct termios options;
    if (tcgetattr(fd, &options) != 0) return -1;

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= CRTSCTS;
    options.c_cflag |= CREAD | CLOCAL;

    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options) == -1) return -1;
#endif
    return 0;
}

// Función para reiniciar el ESP32
void reset_esp32(FD fd) {
#if 1
#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific reset using DTR and RTS control
    Sleep(100);                         // Esperar 100 ms
    EscapeCommFunction(fd, CLRDTR);     // Clear DTR
    EscapeCommFunction(fd, SETRTS);     // Set RTS
    Sleep(100);                         // Esperar 100 ms
    EscapeCommFunction(fd, SETDTR);     // Set DTR
    EscapeCommFunction(fd, CLRRTS);     // Clear RTS
    Sleep(100);                         // Esperar 50 ms
    EscapeCommFunction(fd, CLRDTR);     // Clear RTS
#else
    // Linux-specific reset using ioctl
    int dtr_flag = TIOCM_DTR;
    int rts_flag = TIOCM_RTS;
    usleep(100000);                     // Esperar 100 ms
    ioctl(fd, TIOCMBIC, &dtr_flag);     // Clear DTR
    ioctl(fd, TIOCMBIS, &rts_flag);     // Set RTS
    usleep(100000);                     // Esperar 100 ms
    ioctl(fd, TIOCMBIS, &dtr_flag);     // Set DTR
    ioctl(fd, TIOCMBIC, &rts_flag);     // Clear RTS
    usleep(50000);                      // Esperar 50 ms
    ioctl(fd, TIOCMBIS, &dtr_flag);     // Clear RTS
#endif
#else
#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific reset using DTR and RTS control
    EscapeCommFunction(fd, CLRDTR);     // Clear DTR
    EscapeCommFunction(fd, CLRRTS);     // Clear RTS
    Sleep(100);                         // Esperar 100 ms
    EscapeCommFunction(fd, SETRTS);     // Set RTS
#else
    // Linux-specific reset using ioctl
    int dtr_flag = TIOCM_DTR;
    int rts_flag = TIOCM_RTS;
    ioctl(fd, TIOCMBIC, &dtr_flag);     // Clear DTR
    ioctl(fd, TIOCMBIC, &rts_flag);     // Clear RTS
    usleep(100000);                     // Esperar 100 ms
    ioctl(fd, TIOCMBIS, &rts_flag);     // Set RTS
#endif
#endif
}

// Función para verificar si es un ESP32
int is_esp32(const char *port) {
    FD fd;
#if defined(_WIN32) || defined(_WIN64)
    fd = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (fd == INVALID_HANDLE_VALUE) return 0;
#else
    fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) return 0; // Could not open port
#endif

    if (configure_port(fd) == -1) {
    #if defined(_WIN32) || defined(_WIN64)
        CloseHandle(fd);  // Close for Windows
    #else
        close(fd);  // Close for Linux
    #endif
        return 0;
    }

    // Reset ESP32
    reset_esp32(fd);

    // Leer respuesta
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

#if defined(_WIN32) || defined(_WIN64)
    DWORD bytesRead;
    ReadFile(fd, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
#else
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
#endif

#if defined(_WIN32) || defined(_WIN64)
    CloseHandle(fd);  // Close for Windows
#else
    close(fd);  // Close for Linux
#endif

    if (bytesRead > 0) {
        if (strstr(buffer, "ets Jun") || strstr(buffer, "rst:0x")
            || strstr(buffer, "ESP-IDF") // detecta 2do stage de bootloader
           ) {
            return 1; // ESP32 detectado
        }
    }
    return 0; // No es ESP32
}

// Listar puertos serie y buscar ESP32
const char * find_esp32_port() {
#if defined(_WIN32) || defined(_WIN64)
    // En Windows, puedes usar una librería para detectar los puertos COM disponibles
    // Esto es más complicado porque Windows no tiene un /dev equivalente.
    printf("Scanning for ESP32 on serial port ");
    for (int i = 1; i < 64; i++) {
        char port[20];
        snprintf(port, sizeof(port), "\\\\.\\COM%d", i);
        printf("%s... ", port+4);
        if (is_esp32(port)) {
            printf("ESP32 found!\n");
            return strdup(port+4);
        }
        for (int j = 0; j < strlen(port); j++) putchar('\b');
    }
#else
    struct dirent *entry;
    DIR *dir = opendir("/dev");

    if (!dir) {
        perror("Failed to open /dev directory");
        return NULL;
    }

    printf("Scanning for ESP32 on serial port ");

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "ttyUSB", 6) == 0 || strncmp(entry->d_name, "ttyACM", 6) == 0) {
            char port_path[256] = "";
            if (port_path[0]) for (int j = 0; j < strlen(port_path) + 4; j++) putchar('\b');
            snprintf(port_path, sizeof(port_path), "/dev/%s", entry->d_name);
            printf("%s... ", port_path);
            if (is_esp32(port_path)) {
                printf("ESP32 found!\n");
                closedir(dir);
                return strdup(port_path);
            }
        }
    }

    closedir(dir);
#endif

    printf("ESP32 not found!\n");

    return NULL;
}
