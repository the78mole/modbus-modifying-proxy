# Development Container Setup

This directory contains the configuration for a VS Code development container that provides a complete development environment for the Modbus Modifying Proxy project.

## What's Included

- **ESP-IDF v5.1**: Complete ESP32 development framework
- **RIOT OS**: Real-time operating system (cloned in project root)
- **Cross-compilation toolchain**: xtensa-esp32-elf-gcc compiler
- **Development tools**: git, make, build-essential, python3, etc.
- **VS Code extensions**: C/C++ tools, CMake support, Python support

## Quick Start

1. **Install Prerequisites**:
   - [Docker](https://docker.com)
   - [VS Code](https://code.visualstudio.com)
   - [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

2. **Open in Container**:
   - Open this project in VS Code
   - Press `F1` and select "Dev Containers: Reopen in Container"
   - Wait for the container to build and setup to complete

3. **Start Development**:
   ```bash
   # Source ESP-IDF environment
   get_idf
   
   # Build the project
   build-esp32
   
   # Flash to device (with device connected)
   flash-esp32
   
   # Monitor serial output
   monitor-esp32
   ```

## Available Commands

The development container provides several convenient aliases:

- `get_idf` - Source the ESP-IDF environment
- `build-esp32` - Build for ESP32-WROOM-32
- `flash-esp32` - Flash firmware to ESP32
- `monitor-esp32` - Monitor serial output
- `clean-build` - Clean build artifacts

## Ports

The following ports are automatically forwarded:

- **5683**: CoAP web interface
- **8080**: Alternative HTTP port

## Troubleshooting

### Container Build Issues
- Ensure Docker is running
- Try rebuilding: `F1` → "Dev Containers: Rebuild Container"

### ESP32 Device Access
- Make sure USB device is properly forwarded to the container
- Check device permissions on the host system

### Build Issues
- Run `get_idf` to ensure ESP-IDF environment is loaded
- Check that RIOT OS was properly cloned in the project root

## Manual Commands

If you prefer not to use the aliases:

```bash
# Source ESP-IDF
. /opt/esp/idf/export.sh

# Build
make BOARD=esp32-wroom-32 all

# Flash
make BOARD=esp32-wroom-32 flash

# Monitor
make BOARD=esp32-wroom-32 term
```

## Container Configuration

The container configuration is defined in:
- `devcontainer.json` - Main configuration
- `setup.sh` - Post-creation setup script