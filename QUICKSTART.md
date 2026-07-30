# Quick Start Guide

## 5-Minute Setup

### Prerequisites
- ESP32 or ESP32-S3 board
- 2x RS485 transceivers (e.g., MAX485)
- USB cable
- RIOT OS installed (see main README)

### Step 1: Hardware Setup (5 minutes)

1. Connect first RS485 transceiver to ESP32:
   - TX → DI
   - RX → RO
   - GPIO4 → DE/RE
   
2. Connect second RS485 transceiver to ESP32:
   - TX → DI
   - RX → RO
   - GPIO5 → DE/RE

3. Connect power (3.3V) and GND to both transceivers

4. Connect RS485 A/B terminals to your Modbus bus

### Step 2: Build and Flash (2 minutes)

```bash
# Clone repository
git clone https://github.com/the78mole/modbus-modifying-proxy.git
cd modbus-modifying-proxy

# Build for ESP32
make BOARD=esp32-wroom-32 all

# Flash to device
make BOARD=esp32-wroom-32 flash
```

### Step 3: Connect to WiFi (1 minute)

The device starts in AP mode automatically:
- **SSID**: ModbusProxy
- **Password**: modbus123
- **IP**: 192.168.4.1

Connect your computer or phone to this network.

### Step 4: Configure Rules (2 minutes)

1. Open browser to: `http://192.168.4.1:5683/`
2. Add a rule:
   - Device Address: 1
   - Register Address: 0
   - Type: Add
   - Parameter: 10
3. Click "Add Rule"

**Done!** Your proxy is now forwarding Modbus messages and adding 10 to register 0 values.

## Common Use Cases

### Use Case 1: Sensor Calibration

**Problem**: Temperature sensor reads 2 degrees too low

**Solution**:
```
Device: 1
Register: 100  (temperature register)
Type: Add
Parameter: 2
```

### Use Case 2: Unit Conversion

**Problem**: Need to convert pressure from Pa to kPa

**Solution**:
```
Device: 2
Register: 50  (pressure register)
Type: Divide
Parameter: 1000
```

### Use Case 3: Testing/Simulation

**Problem**: Need to simulate a sensor reading for testing

**Solution**:
```
Device: 3
Register: 25  (sensor register)
Type: Overwrite
Parameter: 100  (constant test value)
```

## Troubleshooting

### Issue: No WiFi network visible
**Fix**: Check power supply, restart device

### Issue: Web interface won't load
**Fix**: Verify IP address in serial monitor, use CoAP client

### Issue: Modbus not forwarding
**Fix**: Check RS485 wiring, verify baud rate (9600)

### Issue: Rules not applying
**Fix**: Verify device/register addresses match your setup

## Next Steps

1. Read full [README.md](README.md) for detailed information
2. Review [CONFIGURATION.md](CONFIGURATION.md) for advanced setup
3. Check [HARDWARE.md](HARDWARE.md) for wiring details
4. See [EXAMPLES.md](EXAMPLES.md) for real-world scenarios

## Quick Reference

### Default Settings
- Baud Rate: 9600
- WiFi Mode: AP
- AP SSID: ModbusProxy
- AP Password: modbus123
- Web Interface: http://192.168.4.1:5683/

### Modification Types
1. Overwrite - Replace with constant
2. Add - Add offset
3. Subtract - Subtract offset
4. Multiply - Scale up
5. Divide - Scale down

### Shell Commands (via serial)
```
ps                    # List threads
free                  # Memory usage
help                  # Available commands
```

## Support

- GitHub Issues: https://github.com/the78mole/modbus-modifying-proxy/issues
- Documentation: See README.md and other .md files
- License: MIT (see LICENSE file)
