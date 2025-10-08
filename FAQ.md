# Frequently Asked Questions (FAQ)

## General Questions

### What is the Modbus Modifying Proxy?

The Modbus Modifying Proxy is a device based on ESP32 that sits between Modbus master and slave devices, allowing you to intercept and modify register values in real-time. It's useful for calibration, unit conversion, testing, and legacy system integration.

### What hardware do I need?

- ESP32 or ESP32-S3 development board
- 2x RS485 transceiver modules (MAX485, MAX3485, etc.)
- Basic wiring and power supply
- See [HARDWARE.md](HARDWARE.md) for detailed requirements

### What is RIOT OS?

RIOT OS is a real-time operating system designed for IoT devices. It provides threading, networking, and hardware abstraction that makes development easier and more reliable.

### Is this an open-source project?

Yes! This project is licensed under the MIT License, which allows free use, modification, and distribution.

## Setup and Installation

### How do I build the firmware?

```bash
# Clone the repository
git clone https://github.com/the78mole/modbus-modifying-proxy.git
cd modbus-modifying-proxy

# Build for ESP32
make BOARD=esp32-wroom-32 all

# Flash to device
make BOARD=esp32-wroom-32 flash
```

See [QUICKSTART.md](QUICKSTART.md) for step-by-step instructions.

### Do I need to install RIOT OS separately?

Yes, RIOT OS should be cloned adjacent to the project directory:

```bash
cd ..
git clone https://github.com/RIOT-OS/RIOT.git
cd modbus-modifying-proxy
```

The Makefile will automatically find RIOT if it's in the expected location.

### Can I use ESP32-S3 instead of ESP32?

Yes! The code supports both ESP32 and ESP32-S3. Just change the BOARD parameter:

```bash
make BOARD=esp32s3-devkit all
```

### What if I get "RIOT not found" error?

Set the RIOTBASE environment variable:

```bash
export RIOTBASE=/path/to/RIOT
make all
```

Or specify it directly:

```bash
make RIOTBASE=/path/to/RIOT all
```

## WiFi Configuration

### How do I change the default WiFi password?

Edit the Makefile before building:

```makefile
CFLAGS += -DESP_WIFI_SSID=\"YourSSID\"
CFLAGS += -DESP_WIFI_PASS=\"YourPassword\"
```

Then rebuild and flash.

### Can I connect to my existing WiFi network?

Yes! Use the shell command via serial console:

```
wifi_connect "YourNetworkSSID" "YourPassword"
```

### How do I find the device's IP address?

Connect via serial console and look for the IP address in the boot messages, or use:

```bash
make BOARD=esp32-wroom-32 term
```

Look for lines showing network configuration.

### Does it support 5GHz WiFi?

No, ESP32 only supports 2.4GHz WiFi bands. This is a hardware limitation.

## Modbus Configuration

### What Modbus function codes are supported?

Currently supported:
- 0x03 (Read Holding Registers) - responses
- 0x04 (Read Input Registers) - responses

Future versions may support:
- 0x06 (Write Single Register)
- 0x10 (Write Multiple Registers)

### What baud rates are supported?

The default is 9600 baud, but you can use:
- 9600
- 19200
- 38400
- 57600
- 115200

Change in `modbus_proxy.c`:

```c
#define UART_BAUDRATE   19200  // Your desired baud rate
```

### Can I modify write operations?

The current implementation focuses on read responses. Modifying write operations requires additional code. See [DEVELOPMENT.md](DEVELOPMENT.md) for how to add this feature.

### How many modification rules can I have?

Default maximum is 32 rules. You can increase this by editing `modbus_proxy.h`:

```c
#define MAX_MODIFY_RULES    64  // Or your desired maximum
```

Note: More rules use more RAM.

### Can I modify multiple registers at once?

Each rule applies to a specific device address and register address combination. For multiple registers, create multiple rules.

## Modification Rules

### What modification types are available?

1. **Overwrite**: Replace value with a constant
2. **Add**: Add an offset to the value
3. **Subtract**: Subtract an offset from the value
4. **Multiply**: Multiply value by a factor
5. **Divide**: Divide value by a divisor

See [CONFIGURATION.md](CONFIGURATION.md) for detailed examples.

### Do modifications use floating point math?

No, all calculations use integer math for performance and compatibility. This means:
- Division may lose precision
- Multiplying large values may overflow
- Plan your modifications accordingly

### Can I chain modifications?

Currently, only the first matching rule is applied. To chain modifications:
- Use multiple proxy devices in series, or
- Implement custom modification type (see [DEVELOPMENT.md](DEVELOPMENT.md))

### What happens if I divide by zero?

The code includes divide-by-zero protection. If you try to divide by 0, the value remains unchanged.

### How do I remove a specific rule?

Via web interface:
1. Access http://device-ip:5683/
2. Find the rule in the list
3. Click "Remove" button

Or clear all rules and re-add the ones you need.

## Hardware and Wiring

### What RS485 transceiver should I use?

Common options:
- MAX485 (basic, 2.5 Mbps)
- MAX3485 (enhanced ESD protection)
- MAX13487E (3.3V, fail-safe)

Any RS485 transceiver compatible with 3.3V logic will work.

### Do I need termination resistors?

Yes! RS485 requires 120Ω termination resistors at both ends of the bus:
- One at the master device
- One at the last slave device

The proxy typically doesn't need termination unless it's at the end of the bus.

### What's the maximum cable length?

For 9600 baud: up to 1200 meters (4000 feet)
Higher baud rates require shorter cables:
- 19200 baud: ~600m
- 115200 baud: ~100m

### Can I use different GPIO pins?

Yes! Edit `modbus_proxy.c`:

```c
#define RS485_DE_IF1    GPIO_PIN(0, 4)  // Change port and pin
#define RS485_DE_IF2    GPIO_PIN(0, 5)  // Change port and pin
```

Also configure UART pins according to your ESP32 variant.

### My RS485 communication doesn't work. What should I check?

1. **Wiring**: Verify A-to-A and B-to-B connections
2. **Polarity**: Ensure correct A/B polarity
3. **Termination**: Check 120Ω resistors are in place
4. **Baud rate**: Verify all devices use same baud rate
5. **Ground**: Ensure common ground between devices
6. **Power**: Check 3.3V supply to transceivers

## Web Interface

### Why can't I access the web interface?

Common issues:
1. **Wrong IP**: Verify device IP address via serial console
2. **Wrong port**: Use port 5683 (CoAP), not 80
3. **Browser compatibility**: Some browsers may have issues with CoAP
4. **Network**: Ensure you're connected to the correct WiFi network
5. **Firewall**: Check if port 5683 is blocked

### What browsers are supported?

The web interface uses CoAP protocol. Best results with:
- Chrome/Edge (with CoAP extension)
- Firefox
- CoAP client tools (copper, coap-client)

### Can I access the interface remotely?

If the ESP32 is connected to your network in STA mode, yes. In AP mode, only devices connected to the ESP32's WiFi can access it.

### Is the configuration saved after reboot?

Currently, no. Configuration is stored in RAM and lost on power cycle. Configuration persistence is planned for future versions using flash storage.

## Performance and Reliability

### How much latency does the proxy add?

Typical latency: 1-2ms per message
- Frame reception: ~0.5ms
- Processing: <1ms
- Transmission: ~0.5ms

This is negligible for most Modbus applications.

### How many messages per second can it handle?

At 9600 baud:
- Theoretical maximum: ~120 frames/second
- Practical throughput: 30-40 frames/second
- Depends on frame size and Modbus timing

### Will it work with my existing Modbus system?

The proxy is transparent to both master and slave. As long as your system uses:
- Modbus RTU protocol
- RS485 physical layer
- Supported function codes (0x03, 0x04)

It should work without issues.

### What happens if the proxy crashes?

- Modbus communication stops (proxy is in the path)
- Both interfaces become non-responsive
- Device requires power cycle to restart
- Configuration is lost (if not saved to flash)

### How reliable is it for production use?

RIOT OS is designed for reliability, but:
- Test thoroughly in your environment
- Monitor performance and errors
- Have a fallback plan
- Consider redundancy for critical applications

## Troubleshooting

### The device won't boot. What should I check?

1. **Power supply**: Ensure adequate 3.3V/5V power
2. **USB connection**: Try different cable/port
3. **Bootloader**: Re-flash bootloader if necessary
4. **Serial output**: Check for error messages
5. **Hardware**: Verify no short circuits

### I see garbled text in serial monitor. What's wrong?

Baud rate mismatch. ESP32 typically uses 115200 for console:

```bash
make BOARD=esp32-wroom-32 term
```

Or manually set terminal to 115200 baud.

### Modbus frames are corrupted. How do I fix this?

1. **Check wiring**: Verify RS485 connections
2. **Reduce noise**: Use shielded twisted pair cable
3. **Lower baud rate**: Try 9600 instead of higher rates
4. **Check termination**: Verify 120Ω resistors
5. **Frame timing**: May need to adjust timeout values

### Rules are not being applied. Why?

1. **Verify addresses**: Check device and register addresses match
2. **Check rule is active**: Look in web interface
3. **Function code**: Only 0x03 and 0x04 responses are modified
4. **Monitor serial**: Look for debug messages
5. **Test with known values**: Use Modbus simulator to verify

### Memory issues / crashes. What can I do?

1. **Reduce rules**: Lower MAX_MODIFY_RULES
2. **Reduce buffers**: Lower MODBUS_MAX_FRAME_SIZE
3. **Optimize stacks**: Adjust thread stack sizes
4. **Check for leaks**: Monitor memory usage over time
5. **Update firmware**: Use latest version

## Development and Customization

### Can I add custom modification types?

Yes! See [DEVELOPMENT.md](DEVELOPMENT.md) for detailed instructions on adding custom modification logic.

### How do I add support for more function codes?

Edit `process_modbus_frame()` in `modbus_proxy.c` to handle additional function codes. See [DEVELOPMENT.md](DEVELOPMENT.md) for examples.

### Can I log Modbus traffic?

You can add logging by modifying the proxy code to print frames. Future versions may include built-in logging to SD card or remote server.

### How do I contribute to the project?

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

See [DEVELOPMENT.md](DEVELOPMENT.md) for contribution guidelines.

### Is there a simulator for testing?

You can use:
- ModbusPoll (Windows) - master simulator
- ModbusSlave (Windows) - slave simulator
- pymodbus (Python) - scriptable testing
- RIOT native board - for software testing

## Advanced Topics

### Can I use this with Modbus TCP?

Current version is Modbus RTU (RS485) only. Modbus TCP support is a future enhancement.

### Can I cascade multiple proxies?

Yes! You can chain proxies:

```
[Master] ←→ [Proxy1] ←→ [Proxy2] ←→ [Slave]
```

Each adds ~1-2ms latency. Configuration must be done separately for each.

### How do I implement data logging?

Future enhancement. Will use:
- littlefs2 for flash storage
- SD card module (optional)
- MQTT for cloud logging

### Can I use this for protocol conversion?

Not currently. The proxy maintains Modbus RTU on both sides. Protocol conversion (e.g., RTU to TCP) requires significant additional code.

### Is OTA (Over-The-Air) update supported?

Not in current version. OTA update is planned for future releases. Current method is USB cable flashing.

### Can I run this on other platforms?

RIOT OS is portable, but:
- Code is optimized for ESP32
- Uses ESP-specific WiFi
- RS485 pins are ESP32-specific

Porting to other platforms requires code changes.

## Safety and Security

### Is this suitable for safety-critical applications?

This is an open-source hobby/industrial project. For safety-critical applications:
- Perform thorough testing and validation
- Consider certified industrial devices
- Implement redundancy
- Follow industry standards (IEC 61508, etc.)

### How secure is the WiFi connection?

Default AP uses WPA2 with password. For security:
- Change default password
- Use strong passwords (12+ characters)
- Enable MAC filtering if needed
- Use STA mode with WPA2-Enterprise if available

### Can someone intercept Modbus traffic?

RS485 is a physical bus - anyone with physical access can tap it. For security:
- Secure physical access to cabling
- Use encryption (requires custom implementation)
- Monitor for unauthorized devices

### What about firmware tampering?

- Enable flash encryption (ESP32 feature)
- Enable secure boot (ESP32 feature)
- Physically secure the device
- Implement firmware verification

## Future Features

### What features are planned?

- Configuration persistence (flash storage)
- MQTT support for remote monitoring
- Data logging capabilities
- Modbus TCP gateway mode
- Web UI improvements
- OTA updates
- More function code support

### When will feature X be available?

Check the GitHub repository for:
- Roadmap in README
- Open issues
- Pull requests
- Release notes

### Can I request a feature?

Yes! Open an issue on GitHub:
https://github.com/the78mole/modbus-modifying-proxy/issues

Provide:
- Clear description
- Use case
- Expected behavior
- Examples if applicable

### How can I help with development?

- Report bugs
- Test with different hardware
- Submit pull requests
- Improve documentation
- Share use cases

See [DEVELOPMENT.md](DEVELOPMENT.md) for contribution guidelines.

## Getting Help

### Where can I get support?

1. **Documentation**: Read all .md files in repository
2. **GitHub Issues**: Search existing issues or create new one
3. **Community**: Join RIOT OS community forums
4. **Examples**: Check [EXAMPLES.md](EXAMPLES.md) for use cases

### How do I report a bug?

Open an issue on GitHub with:
- Clear title and description
- Steps to reproduce
- Expected vs actual behavior
- Hardware setup details
- Serial console output
- Code version/commit hash

### Is commercial support available?

This is an open-source community project. For commercial support:
- Contact the maintainers via GitHub
- Consider hiring embedded development consultants
- Check if authors offer commercial services

### Can I use this in a commercial product?

Yes! MIT License allows commercial use. Requirements:
- Include license notice
- Provide attribution
- No warranty is provided

### Where can I find more examples?

- [EXAMPLES.md](EXAMPLES.md) - Real-world scenarios
- [QUICKSTART.md](QUICKSTART.md) - Basic setup examples
- [CONFIGURATION.md](CONFIGURATION.md) - Configuration examples
- GitHub issues - Community discussions

## Additional Resources

### Documentation Files
- [README.md](README.md) - Project overview
- [QUICKSTART.md](QUICKSTART.md) - Quick setup guide
- [HARDWARE.md](HARDWARE.md) - Hardware setup
- [CONFIGURATION.md](CONFIGURATION.md) - Configuration guide
- [EXAMPLES.md](EXAMPLES.md) - Real-world examples
- [DEVELOPMENT.md](DEVELOPMENT.md) - Development guide
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture

### External Resources
- [RIOT OS](https://riot-os.org/)
- [Modbus Organization](https://modbus.org/)
- [ESP32 Documentation](https://docs.espressif.com/)

### Tools
- [ModbusPoll](https://www.modbustools.com/modbus_poll.html)
- [ModbusSlave](https://www.modbustools.com/modbus_slave.html)
- [pymodbus](https://pymodbus.readthedocs.io/)

---

## Still have questions?

If your question isn't answered here:
1. Check all documentation files
2. Search GitHub issues
3. Create a new issue with your question

We're here to help!
