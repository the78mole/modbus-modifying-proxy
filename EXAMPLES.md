# Real-World Examples

This document provides real-world examples and use cases for the Modbus Modifying Proxy.

## Example 1: HVAC System Calibration

### Scenario
An HVAC system has multiple temperature sensors. One sensor consistently reads 3°C lower than actual temperature due to placement near a cold wall.

### Setup
- Modbus Master: Building Management System (BMS)
- Modbus Slave: HVAC Controller (Address: 10)
- Temperature Register: 40100 (register 100)

### Configuration
```
Device Address: 10
Register Address: 100
Modification Type: Add
Parameter: 3
```

### Result
If the sensor reads 20°C, the BMS will receive 23°C, compensating for the installation bias.

## Example 2: Flow Meter Unit Conversion

### Scenario
A flow meter outputs readings in L/h (liters per hour), but the SCADA system expects m³/h (cubic meters per hour).

### Setup
- Modbus Master: SCADA System
- Modbus Slave: Flow Meter (Address: 5)
- Flow Register: 30001 (register 1)

### Configuration
```
Device Address: 5
Register Address: 1
Modification Type: Divide
Parameter: 1000
```

### Result
If the meter reads 5000 L/h, the SCADA receives 5 m³/h (5000 ÷ 1000).

## Example 3: Pressure Sensor Scaling

### Scenario
A pressure transmitter outputs 0-10000 representing 0-100 bar, but the PLC expects 0-1000 for the same range.

### Setup
- Modbus Master: PLC
- Modbus Slave: Pressure Transmitter (Address: 3)
- Pressure Register: 30005 (register 5)

### Configuration
```
Device Address: 3
Register Address: 5
Modification Type: Divide
Parameter: 10
```

### Result
If transmitter outputs 5000 (50 bar), PLC receives 500.

## Example 4: Multiple Sensor Calibration

### Scenario
A water treatment plant has 3 pH sensors that need different calibration offsets.

### Setup
- Modbus Master: PLC
- Modbus Slaves: pH Sensors (Address: 1, 2, 3)
- pH Register: 0 (for all sensors)

### Configuration
```
Rule 1:
  Device Address: 1
  Register Address: 0
  Modification Type: Add
  Parameter: 5

Rule 2:
  Device Address: 2
  Register Address: 0
  Modification Type: Subtract
  Parameter: 3

Rule 3:
  Device Address: 3
  Register Address: 0
  Modification Type: Add
  Parameter: 2
```

### Result
Each sensor's reading is independently calibrated based on lab verification.

## Example 5: Test Bench Simulation

### Scenario
Testing a control algorithm without physical sensors by simulating constant values.

### Setup
- Modbus Master: Test Controller
- Modbus Slave: Sensor Simulator (Address: 1)
- Registers: 0-10

### Configuration
```
Rule 1: Device 1, Register 0, Overwrite, 250   (Simulated temperature)
Rule 2: Device 1, Register 1, Overwrite, 1013  (Simulated pressure)
Rule 3: Device 1, Register 2, Overwrite, 50    (Simulated humidity)
Rule 4: Device 1, Register 3, Overwrite, 750   (Simulated flow)
```

### Result
Controller receives predictable values for testing logic without real sensors.

## Example 6: Legacy System Integration

### Scenario
Integrating a new Modbus device that outputs values in different units than the legacy system expects.

### Setup
- Modbus Master: Legacy PLC (expects PSI)
- Modbus Slave: New Pressure Sensor (outputs kPa)
- Address: 7, Register: 100

### Configuration
Convert kPa to PSI (multiply by ~0.145, use 145 and divide by 1000):

```
Rule 1:
  Device Address: 7
  Register Address: 100
  Modification Type: Multiply
  Parameter: 145

Rule 2:
  Device Address: 7
  Register Address: 101  (Store intermediate result)
  Modification Type: Divide
  Parameter: 1000
```

**Note**: This example shows the limitation of single-step modifications. For precise conversions, consider:
- Using integer math approximations
- Implementing custom modification types
- Pre-scaling values in the sensor configuration

### Workaround
Use multiply by 1 (145/1000 = 0.145) as approximation, or multiply by 14 and divide by 100:

```
Device Address: 7
Register Address: 100
Modification Type: Multiply
Parameter: 14
```

Then read and mentally divide by 100, or use second proxy in series.

## Example 7: Alarm Threshold Adjustment

### Scenario
A high-temperature alarm threshold needs to be raised temporarily during a maintenance procedure.

### Setup
- Modbus Master: Safety Controller
- Modbus Slave: Temperature Monitor (Address: 4)
- Threshold Register: 200

### Configuration (Temporary - During Maintenance)
```
Device Address: 4
Register Address: 200
Modification Type: Overwrite
Parameter: 150  (Higher threshold during maintenance)
```

### Normal Operation
Remove the rule or set to:
```
Device Address: 4
Register Address: 200
Modification Type: Overwrite
Parameter: 80  (Normal threshold)
```

## Example 8: Data Smoothing / Filtering

### Scenario
A vibration sensor produces noisy readings. Scale down fluctuations by 50%.

### Setup
- Modbus Master: Condition Monitoring System
- Modbus Slave: Vibration Sensor (Address: 8)
- Reading Register: 10

### Configuration
```
Device Address: 8
Register Address: 10
Modification Type: Divide
Parameter: 2
```

### Result
Reduces amplitude of fluctuations by half, making trending easier to visualize.

**Note**: This is a simple approach. True filtering would require averaging over time.

## Example 9: Multi-Device Energy Monitoring

### Scenario
Three energy meters need to be summed, but one reads in Wh while others read in kWh.

### Setup
- Modbus Master: Energy Management System
- Modbus Slaves: Meters at addresses 11, 12, 13
- Energy Register: 0 (for all)

### Configuration
```
Device Address: 11
Register Address: 0
Modification Type: Divide
Parameter: 1000  (Convert Wh to kWh)
```

### Result
All meters now report in kWh, allowing proper summing in the management system.

## Example 10: Commission/Decommission Transition

### Scenario
During a phased equipment replacement, need to gradually transition from old sensor to new sensor.

### Phase 1: Use Old Sensor
```
Device Address: 20  (old sensor)
Register Address: 0
Modification Type: None  (pass through)
```

### Phase 2: Blend Values (Use Average)
Use both sensors and take average:
```
Device Address: 20  (old sensor)
Register Address: 0
Modification Type: Divide
Parameter: 2

Device Address: 21  (new sensor)
Register Address: 0
Modification Type: Divide
Parameter: 2
```

Then sum at controller level.

### Phase 3: Use New Sensor Only
```
Device Address: 21  (new sensor)
Register Address: 0
Modification Type: None  (pass through)
```

## Best Practices

### 1. Document Everything
Keep a log of all active rules with:
- Date applied
- Reason for modification
- Expected impact
- Removal date (if temporary)

### 2. Test Before Production
- Set up test environment
- Verify modifications with known values
- Check edge cases (min/max values)
- Monitor for overflow/underflow

### 3. Use Meaningful Parameters
- Prefer multiplication over division when possible (better precision)
- Use integer math carefully (no floating point)
- Consider value ranges and data types

### 4. Monitor Performance
- Check for communication delays
- Verify CRC/checksums are correct
- Look for dropped messages
- Monitor proxy CPU/memory usage

### 5. Security Considerations
- Change default WiFi password
- Limit access to configuration interface
- Log all configuration changes
- Regular security audits

### 6. Maintenance
- Regular firmware updates
- Backup configuration before changes
- Test rules after firmware updates
- Document any issues or workarounds

## Troubleshooting Common Issues

### Issue: Value Not Changing
**Possible Causes**:
- Wrong device address
- Wrong register address
- Rule not active
- Modbus function code not supported

**Solution**: Verify addresses, check Modbus function code (only 0x03 and 0x04 supported by default)

### Issue: Incorrect Calculations
**Possible Causes**:
- Integer overflow
- Division by zero protection
- Wrong parameter value

**Solution**: Check value ranges, verify parameters, add overflow protection

### Issue: Intermittent Application
**Possible Causes**:
- Multiple rules for same register
- Timing issues
- Communication errors

**Solution**: Simplify rules, check for conflicts, verify RS485 wiring

## Performance Considerations

### Latency Impact
Each proxy adds approximately:
- Processing time: < 1ms per message
- Modification lookup: < 0.1ms per rule
- Total overhead: ~1-2ms per message

### Throughput Capacity
At 9600 baud:
- ~960 bytes/second
- ~40 Modbus messages/second (typical)
- Proxy can handle this easily with headroom

### Scaling Considerations
- Each additional rule adds minimal processing time
- Maximum 32 rules by default (configurable)
- No significant impact until >50 rules

## Advanced Techniques

### Chaining Proxies
Multiple proxies can be chained for complex modifications:

```
[Master] ←→ [Proxy 1] ←→ [Proxy 2] ←→ [Slave]
```

Each proxy applies its own rules sequentially.

### Conditional Modifications
Future enhancement: Apply rules based on conditions

### Register Range Modifications
Future enhancement: Apply same modification to a range of registers

### Dynamic Rule Updates
Use the web interface to update rules in real-time without restart.

## Conclusion

The Modbus Modifying Proxy is a versatile tool for:
- Sensor calibration
- Unit conversion
- Legacy system integration
- Testing and simulation
- Data conditioning

By understanding these examples and best practices, you can effectively deploy the proxy in various industrial and automation scenarios.
