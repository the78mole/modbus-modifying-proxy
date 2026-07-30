# Hardware Setup Guide

## Overview

This guide provides detailed instructions for setting up the hardware for the Modbus Modifying Proxy.

## Bill of Materials (BOM)

| Component | Quantity | Description |
|-----------|----------|-------------|
| ESP32 or ESP32-S3 | 1 | Main microcontroller board |
| RS485 Transceiver (MAX485/MAX3485) | 2 | For RS485 communication |
| Resistors 120Ω | 2 | Termination resistors for RS485 |
| Breadboard/PCB | 1 | For assembly |
| Jumper wires | - | Various lengths |
| Power supply 5V/3.3V | 1 | Depends on board requirements |

## Circuit Connections

### RS485 Interface 1

```
ESP32 Pin      →  MAX485 Pin    →  RS485 Bus
---------------------------------------------------
UART1 TX       →  DI (Driver Input)
UART1 RX       →  RO (Receiver Output)
GPIO 4         →  DE/RE (Driver/Receiver Enable)
3.3V           →  VCC
GND            →  GND
               →  A              →  A (Data+)
               →  B              →  B (Data-)
```

### RS485 Interface 2

```
ESP32 Pin      →  MAX485 Pin    →  RS485 Bus
---------------------------------------------------
UART2 TX       →  DI (Driver Input)
UART2 RX       →  RO (Receiver Output)
GPIO 5         →  DE/RE (Driver/Receiver Enable)
3.3V           →  VCC
GND            →  GND
               →  A              →  A (Data+)
               →  B              →  B (Data-)
```

## ESP32 Pin Mapping

### ESP32-WROOM-32

| Function | GPIO | Alternative Pin |
|----------|------|-----------------|
| UART1 TX | GPIO 10 | GPIO 17 |
| UART1 RX | GPIO 9 | GPIO 16 |
| UART2 TX | GPIO 17 | GPIO 25 |
| UART2 RX | GPIO 16 | GPIO 26 |
| RS485_1_DE | GPIO 4 | Any GPIO |
| RS485_2_DE | GPIO 5 | Any GPIO |

### ESP32-S3

| Function | GPIO | Alternative Pin |
|----------|------|-----------------|
| UART1 TX | GPIO 17 | GPIO 43 |
| UART1 RX | GPIO 18 | GPIO 44 |
| UART2 TX | GPIO 8 | GPIO 9 |
| UART2 RX | GPIO 3 | GPIO 10 |
| RS485_1_DE | GPIO 4 | Any GPIO |
| RS485_2_DE | GPIO 5 | Any GPIO |

## RS485 Bus Termination

Important: RS485 networks require proper termination!

1. Install 120Ω termination resistor between A and B at **both ends** of the RS485 bus
2. Keep RS485 cable length under 1200m (4000ft) for 9600 baud
3. Use twisted pair cable for better noise immunity

### Termination Example

```
Master Device                Proxy                    Slave Device
     [120Ω]─┐            ┌─────────┐            ┐─[120Ω]
            │            │         │            │
    A ──────┴────────────┤ A     A ├────────────┴────── A
    B ───────────────────┤ B     B ├──────────────────── B
                         │         │
                         └─────────┘
```

## Power Supply Considerations

### Option 1: USB Power (Development)
- Connect ESP32 via USB cable
- Provides 5V power for ESP32 and can power MAX485 at 3.3V

### Option 2: External Power (Production)
- Use regulated 5V or 3.3V power supply
- Ensure sufficient current: 500mA minimum
- Add bypass capacitors (100nF) near each IC

## Assembly Steps

1. **Mount ESP32**
   - Place ESP32 on breadboard or PCB
   - Ensure good contact with all pins

2. **Connect RS485 Transceivers**
   - Place two MAX485 ICs on breadboard
   - Wire VCC and GND to power rails
   - Add 100nF bypass capacitor near each IC

3. **Wire UART Connections**
   - Connect ESP32 UART pins to MAX485 DI/RO pins
   - Keep wires short to minimize noise

4. **Wire Driver Enable**
   - Connect GPIO 4 to DE and RE pins of first MAX485
   - Connect GPIO 5 to DE and RE pins of second MAX485
   - Note: DE and RE pins should be connected together

5. **RS485 Bus Connections**
   - Connect A and B terminals to RS485 bus
   - Ensure correct polarity (A to A, B to B)
   - Install termination resistors if at end of bus

6. **Power Connection**
   - Double-check all power connections
   - Verify 3.3V/5V levels are correct
   - Ensure common ground between all devices

## Testing Hardware

1. **Visual Inspection**
   - Check all connections
   - Verify no short circuits
   - Ensure proper polarity

2. **Power Test**
   - Apply power
   - Measure voltages at VCC pins (should be 3.3V)
   - Check for excessive current draw

3. **Continuity Test**
   - Test RS485 bus continuity
   - Verify A-A and B-B connections
   - Check termination resistors (should read ~60Ω between A-B when both terminations are in place)

4. **Communication Test**
   - Flash test firmware
   - Send test Modbus message
   - Verify response on both interfaces

## Troubleshooting

### No Communication
- Check RS485 A/B polarity
- Verify termination resistors
- Test with oscilloscope for signal integrity

### Intermittent Errors
- Add bypass capacitors if not present
- Check for loose connections
- Reduce cable length
- Shield cables from noise sources

### Device Not Powering Up
- Check power supply voltage and current
- Verify ESP32 is seated properly
- Check for short circuits

## Safety Considerations

1. **Electrical Safety**
   - Disconnect power before making changes
   - Use proper ESD precautions
   - Avoid touching components while powered

2. **RS485 Bus**
   - Do not hot-plug RS485 connections
   - Ensure all devices share common ground
   - Keep bus within voltage specifications

3. **Enclosure**
   - Use proper enclosure for industrial environments
   - Ensure adequate ventilation
   - Provide strain relief for cables

## Advanced Topics

### Multiple Proxies in Series
- Can chain multiple proxy devices
- Each proxy adds ~1ms latency
- Consider cumulative termination effects

### Long Cable Runs
- For distances > 500m, consider:
  - Reducing baud rate
  - Using RS485 repeaters
  - Upgrading to RS422

### Industrial Installation
- Use DIN rail mounting
- Install surge protection
- Ground shields properly
- Follow industrial wiring standards
