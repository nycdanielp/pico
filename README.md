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

## Project Structure

- `main.c/h` - Main application entry point
- `kernel.c/h` - Custom kernel implementation
- `tcp_server.c/h` - TCP networking server
- `usb_fs.c/h` - USB filesystem support
- `cli.c/h` - Command-line interface
- `config_store.c/h` - Configuration management
- `pico_sdk_import.cmake` - Pico SDK integration
- `.vscode/` - VS Code project configuration

## License

[Add your license here]
