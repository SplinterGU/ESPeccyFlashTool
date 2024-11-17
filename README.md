# ESPeccy Flash Tool

**ESPeccy Flash Tool** is a command-line tool developed to easily flash the latest firmware for the ESP32-based [**ESPeccy**](https://github.com/SplinterGU/ESPeccy) emulator. This tool automatically detects the connected ESP32 device, downloads the latest firmware, and flashes it to the device without requiring user interaction. The only user input is the choice of using the version with or without PSRAM by providing the appropriate command-line option.

## Features

- Automatically detects the connected ESP32 device.
- Downloads the latest firmware for the ESP32.
- Flashes the firmware to the ESP32 device.
- No user interaction required except for the choice of firmware version (with or without PSRAM).
- Command-line based tool for ease of use.

## Usage

Run the tool from the command line as follows:

### Command:
```bash
especcy_flash_tool [options]
```

### Options:
- `-h`
  Display this help message.

- `-nopsram`
  Use the firmware version without PSRAM.

### Example:
To flash the firmware with PSRAM:
```bash
especcy_flash_tool
```

To flash the firmware without PSRAM:
```bash
especcy_flash_tool -nopsram
```

## How It Works

1. The tool automatically detects the correct COM port where the ESP32 is connected.
2. It downloads the latest firmware from the [**ESPeccy**](https://github.com/SplinterGU/ESPeccy) repository.
3. It flashes the downloaded firmware to the ESP32 device.
4. The flashing process is fully automated, requiring no additional interaction from the user, except for selecting the firmware version.

## Requirements

- A computer running **Linux** or **Windows**.
- A connected **ESP32** device.
- Internet access to download the latest firmware.

## Compilation

### Linux/macOS

1. Clone the repository:
   ```bash
   git clone https://github.com/SplinterGU/ESPeccyFlashTool.git
   cd ESPeccyFlashTool
   ```

2. Create a build directory and run **CMake**:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Compile the tool:
   ```bash
   make
   ```

4. Run the tool:
   ```bash
   ./especcy_flash_tool
   ```

### Windows

1. Clone the repository:
   ```bash
   git clone https://github.com/SplinterGU/ESPeccyFlashTool.git
   cd ESPeccyFlashTool
   ```

2. Create a build directory and run **CMake**:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. Compile using **MinGW** or **Visual Studio**:
   - For MinGW:
     ```bash
     mingw32-make
     ```

   - For Visual Studio, open the generated solution file in Visual Studio and build it.

4. Run the tool:
   ```bash
   especcy_flash_tool.exe
   ```

## Related Projects

- [**ESPeccy**](https://github.com/SplinterGU/ESPeccy)
  This is the emulator for ZX Spectrum running on ESP32 that you can flash using this tool.

## Credits

- **Developed by SplinterGU**
- GitHub repository: [https://github.com/SplinterGU/ESPeccyFlashTool](https://github.com/SplinterGU/ESPeccyFlashTool)
- This tool is designed for flashing the latest ESP32 firmware for the [**ESPeccy**](https://github.com/SplinterGU/ESPeccy) emulator.

## License

MIT License. See the [LICENSE](LICENSE) file for details.
