# Pico OS Project

A Raspberry Pi Pico firmware project with networking and USB support.

## Features

- TCP/IP networking via lwIP
- USB filesystem support
- CLI interface
- Configuration storage
- Custom kernel implementation

## Prerequisites

- Raspberry Pi Pico board
- CMake 3.12 or later
- ARM GNU Toolchain
- Pico SDK

## Building

1. Clone the repository:
   ```bash
   git clone https://github.com/nycdanielp/pico.git
   cd pico
   ```

2. Initialize and update submodules:
   ```bash
   git submodule update --init --recursive
   ```

3. Create and configure the build directory:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

4. Build the project:
   ```bash
   ninja
   ```

## Flashing to Device

Use the Pico tools to load the compiled firmware onto your device.

## CLI Features

The firmware includes a command-line interface with built-in file editing capabilities:

- **File Editor**: Use `edit [filename]` command to edit files directly from the CLI

## Configuration

### WiFi Configuration

The `wifi.txt` file contains WiFi credentials for connecting to your network. 

**To update WiFi settings:**
1. Edit `wifi.txt` with your WiFi SSID and password
2. Copy the updated `wifi.txt` file to a USB drive
3. Connect the USB drive to the Pico
4. The file will automatically overwrite the existing configuration in the firmware image

## Project Structure

- `main.c/h` - Main application entry point
- `kernel.c/h` - Custom kernel implementation
- `tcp_server.c/h` - TCP networking server
- `usb_fs.c/h` - USB filesystem support
- `cli.c/h` - Command-line interface
- `config_store.c/h` - Configuration management
- `pico_sdk_import.cmake` - Pico SDK integration
- `wifi.txt` - WiFi credentials (SSID and password)
- `.vscode/` - VS Code project configuration

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.
