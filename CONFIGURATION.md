# Configuration Guide

## Overview

This guide explains how to configure the Modbus Modifying Proxy for your specific use case.

## WiFi Configuration

### Access Point Mode (Default)

By default, the device starts in Access Point (AP) mode:

```
SSID: ModbusProxy
Password: modbus123
IP Address: 192.168.4.1
```

#### Changing Default AP Credentials

Edit the `Makefile` before building:

```makefile
CFLAGS += -DESP_WIFI_SSID=\"MyProxyAP\"
CFLAGS += -DESP_WIFI_PASS=\"MySecurePassword\"
```

Then rebuild and flash:

```bash
make BOARD=esp32-wroom-32 clean all flash
```

### Station Mode

To connect to an existing WiFi network:

1. Connect via serial console
2. Use the shell command:
   ```
   wifi_connect <ssid> <password>
   ```

Example:
```
wifi_connect "MyNetwork" "MyPassword"
```

## Modbus Configuration

### RS485 Interface Settings

Default settings (modify in `modbus_proxy.c`):

```c
#define UART_BAUDRATE       9600    // Baud rate
#define UART_IF1            UART_DEV(1)  // First interface
#define UART_IF2            UART_DEV(2)  // Second interface
```

#### Supported Baud Rates
- 9600 (default)
- 19200
- 38400
- 57600
- 115200

To change baud rate, edit `modbus_proxy.c`:

```c
#define UART_BAUDRATE       19200  // Change to desired rate
```

### GPIO Pin Configuration

Default pins (modify in `modbus_proxy.c`):

```c
#define RS485_DE_IF1        GPIO_PIN(0, 4)  // Interface 1 Driver Enable
#define RS485_DE_IF2        GPIO_PIN(0, 5)  // Interface 2 Driver Enable
```

## Modification Rules

### Rule Structure

Each modification rule consists of:
- **Device Address**: Modbus slave address (1-247)
- **Register Address**: Register number to modify (0-65535)
- **Modification Type**: Type of modification to apply
- **Parameter**: Value used for modification

### Modification Types

#### 1. Overwrite (Type 1)

Replaces the register value with a constant.

**Use Case**: Force a sensor reading to a specific value for testing.

**Example**: Always return 100 for register 40001
```
Device Address: 1
Register Address: 0
Type: Overwrite
Parameter: 100
```

#### 2. Add (Type 2)

Adds a constant offset to the register value.

**Use Case**: Calibrate a sensor with a known offset.

**Example**: Add 10 to temperature reading
```
Device Address: 1
Register Address: 5
Type: Add
Parameter: 10
```

Result: If sensor reads 20, proxy returns 30

#### 3. Subtract (Type 3)

Subtracts a constant offset from the register value.

**Use Case**: Remove bias from sensor readings.

**Example**: Subtract 5 from pressure reading
```
Device Address: 2
Register Address: 10
Type: Subtract
Parameter: 5
```

Result: If sensor reads 100, proxy returns 95

#### 4. Multiply (Type 4)

Multiplies the register value by a factor.

**Use Case**: Unit conversion or scaling.

**Example**: Convert Celsius to Fahrenheit approximation
```
Device Address: 1
Register Address: 0
Type: Multiply
Parameter: 2
```

Result: If sensor reads 20, proxy returns 40

#### 5. Divide (Type 5)

Divides the register value by a divisor.

**Use Case**: Scale down values or unit conversion.

**Example**: Convert large values to smaller units
```
Device Address: 1
Register Address: 15
Type: Divide
Parameter: 10
```

Result: If sensor reads 1000, proxy returns 100

#### 6. Remap Address (Type 6)

Remaps one register address to another.

**Use Case**: Replace a device with another that uses different register addresses.

**Example**: Map old device register 40001 (register 0) to new device register 30001 (register 0 in new numbering, but different base)
```
Device Address: 1
Register Address: 100
Type: Remap Address
Parameter: 200
```

Result: 
- When master requests register 100, the proxy requests register 200 from the slave
- When slave responds with register 200 data, the proxy presents it as register 100 to the master

**Note**: This works bidirectionally - both requests and responses are automatically remapped.

## Web Interface Configuration

### Accessing Web Interface

1. Connect to the device WiFi network
2. Open browser to: `http://192.168.4.1:5683/`
   - Note: Uses CoAP protocol, may require compatible client
3. Alternative: Use CoAP client tools

### Adding Rules via Web Interface

1. Enter device address (1-247)
2. Enter register address (0-65535)
3. Select modification type from dropdown
4. Enter parameter value
5. Click "Add Rule"

### Removing Rules

- **Remove Single Rule**: Click "Remove" next to the rule
- **Clear All Rules**: Click "Clear All Rules" button

## Advanced Configuration

### Maximum Number of Rules

Default: 32 rules (defined in `modbus_proxy.h`)

To increase:

```c
#define MAX_MODIFY_RULES    64  // Change to desired maximum
```

**Warning**: More rules require more RAM. Monitor memory usage.

### Frame Size Limit

Default: 256 bytes

To change (in `modbus_proxy.c`):

```c
#define MODBUS_MAX_FRAME_SIZE   512  // Increase if needed
```

### Timeout Settings

Frame timeout (silence detection): 5ms

To adjust (in `modbus_proxy.c`):

```c
if ((xtimer_now_usec() - last_byte_time) > 10000) {  // Change to 10ms
    break;
}
```

## Configuration Examples

### Example 1: Temperature Sensor Calibration

**Scenario**: Temperature sensor reads 3°C too low

```
Device Address: 1
Register Address: 100
Type: Add
Parameter: 3
```

### Example 2: Pressure Scaling

**Scenario**: Convert pressure from Pa to kPa (divide by 1000)

```
Device Address: 2
Register Address: 200
Type: Divide
Parameter: 1000
```

### Example 3: Test Mode Override

**Scenario**: Force flow meter to always read 50 L/min for testing

```
Device Address: 3
Register Address: 50
Type: Overwrite
Parameter: 50
```

### Example 4: Multiple Modifications

**Scenario**: Apply different modifications to different registers

```
Rule 1:
  Device: 1, Register: 0, Type: Add, Parameter: 10

Rule 2:
  Device: 1, Register: 1, Type: Multiply, Parameter: 2

Rule 3:
  Device: 2, Register: 100, Type: Overwrite, Parameter: 0
```

## Persistence

**Current Implementation**: Configuration is stored in RAM only and will be lost on power cycle.

**Future Enhancement**: Add support for saving configuration to flash memory using RIOT's VFS/littlefs.

To implement persistence:
1. Enable littlefs in Makefile (already included)
2. Save rules to file on changes
3. Load rules on startup

## Troubleshooting Configuration

### Rules Not Applied

1. **Check device address matches**: Ensure the Modbus device address in the rule matches the actual device
2. **Verify register address**: Check that register address is correct (some systems use 1-based indexing)
3. **Check rule is active**: Verify rule appears in active rules list
4. **Monitor serial output**: Look for error messages in console

### WiFi Connection Issues

1. **Check credentials**: Verify SSID and password are correct
2. **Signal strength**: Ensure device is within WiFi range
3. **Channel conflicts**: Try changing WiFi channel
4. **Restart device**: Power cycle the ESP32

### Web Interface Not Loading

1. **Verify IP address**: Check serial console for device IP
2. **Use CoAP client**: Browser compatibility may vary
3. **Check WiFi connection**: Ensure connected to correct network
4. **Firewall settings**: Some networks may block CoAP port (5683)

## Best Practices

1. **Document your rules**: Keep a list of all active modification rules
2. **Test thoroughly**: Verify modifications work as expected before production use
3. **Monitor performance**: Watch for communication delays or errors
4. **Backup configuration**: Keep a record of all settings
5. **Use meaningful naming**: If extending code, use clear variable names
6. **Version control**: Track firmware versions with configuration changes

## Security Considerations

1. **Change default password**: Always change default WiFi password
2. **Use WPA2**: Ensure WiFi security is enabled
3. **Access control**: Limit access to configuration interface
4. **Audit rules**: Regularly review active modification rules
5. **Physical security**: Protect device from unauthorized access

## Performance Tuning

### Optimizing Throughput

1. **Reduce rule count**: Fewer rules = faster processing
2. **Increase baud rate**: Higher rates allow more messages per second
3. **Optimize timeouts**: Balance reliability vs. speed
4. **Monitor latency**: Measure proxy delay impact

### Memory Optimization

1. **Reduce buffer sizes**: If handling small frames only
2. **Limit rule count**: Adjust MAX_MODIFY_RULES based on needs
3. **Disable unused features**: Comment out unnecessary modules in Makefile

## Shell Commands

Access via serial console:

- `ps` - List running threads
- `free` - Show memory usage
- `help` - List available commands
- Custom commands can be added in `main.c`

## Configuration File Format (Future)

Planned JSON configuration format:

```json
{
  "wifi": {
    "mode": "ap",
    "ssid": "ModbusProxy",
    "password": "modbus123"
  },
  "modbus": {
    "baudrate": 9600,
    "rules": [
      {
        "device": 1,
        "register": 0,
        "type": "add",
        "parameter": 10
      }
    ]
  }
}
```
