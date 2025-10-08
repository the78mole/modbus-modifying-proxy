# Architecture Diagrams

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32/ESP32-S3                          │
│                         RIOT OS                                 │
│                                                                 │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐        │
│  │    WiFi      │  │    Modbus    │  │     Web      │        │
│  │   Manager    │  │    Proxy     │  │  Interface   │        │
│  └──────────────┘  └──────────────┘  └──────────────┘        │
│         │                  │                  │                │
│         │                  │                  │                │
│    ┌────▼────┐       ┌────▼────┐       ┌────▼────┐          │
│    │ ESP WiFi│       │ RS485-1 │       │  CoAP   │          │
│    │  Stack  │       │ RS485-2 │       │ Server  │          │
│    └────┬────┘       └────┬────┘       └────┬────┘          │
└─────────┼─────────────────┼─────────────────┼────────────────┘
          │                 │                 │
          │                 │                 │
     ┌────▼────┐       ┌────▼────┐      ┌────▼────┐
     │  WiFi   │       │ Modbus  │      │ Browser │
     │ Network │       │ Devices │      │ /Client │
     └─────────┘       └─────────┘      └─────────┘
```

## Data Flow

### Modbus Frame Processing

```
┌──────────────┐
│ RS485 IF1 RX │
└──────┬───────┘
       │
       ▼
┌──────────────────┐
│ UART RX Buffer   │
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ Frame Detection  │ (Timeout-based)
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ Parse Modbus     │
│ - Device Addr    │
│ - Function Code  │
│ - Registers      │
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ Apply Rules      │ ◄─── Configuration
│ - Lookup addr/reg│
│ - Apply modifier │
│ - Overwrite/Math │
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ Recalculate CRC  │
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ UART TX Buffer   │
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│ RS485 IF2 TX     │
└──────────────────┘
```

## Threading Model

```
┌─────────────────────────────────────────────────────────────┐
│                         RIOT OS Scheduler                    │
└─────────────────────────────────────────────────────────────┘
     │          │              │              │          │
     ▼          ▼              ▼              ▼          ▼
┌────────┐ ┌────────┐  ┌──────────┐  ┌──────────┐ ┌────────┐
│  Main  │ │ IF1→2  │  │  IF2→1   │  │   Web    │ │  WiFi  │
│ Thread │ │ Thread │  │  Thread  │  │  Server  │ │ Thread │
└────────┘ └────────┘  └──────────┘  └──────────┘ └────────┘
    │                                      │            │
    ▼                                      ▼            ▼
┌────────┐                           ┌──────────┐ ┌────────┐
│ Shell  │                           │  CoAP    │ │ ESP    │
│ Loop   │                           │ Handler  │ │ Stack  │
└────────┘                           └──────────┘ └────────┘
```

### Thread Priorities

```
Priority Level    Thread
──────────────────────────────────
Main - 2          Web Server
Main - 1          Modbus IF1→2
Main - 1          Modbus IF2→1
Main              Main Thread
(Lower)           WiFi Thread
```

## Memory Layout

```
┌─────────────────────────────────────────┐
│         ESP32 Memory Map                │
├─────────────────────────────────────────┤
│  IRAM (Instruction RAM)                 │
│  - RIOT kernel                          │
│  - Time-critical code                   │
├─────────────────────────────────────────┤
│  DRAM (Data RAM)                        │
│  ┌─────────────────────────────────┐   │
│  │ Stack (Main Thread)             │   │
│  ├─────────────────────────────────┤   │
│  │ Stack (IF1→2 Thread)            │   │
│  ├─────────────────────────────────┤   │
│  │ Stack (IF2→1 Thread)            │   │
│  ├─────────────────────────────────┤   │
│  │ Stack (Web Server Thread)       │   │
│  ├─────────────────────────────────┤   │
│  │ Global Variables                │   │
│  │ - config (modbus_config_t)      │   │
│  │ - buffers                       │   │
│  ├─────────────────────────────────┤   │
│  │ Heap                            │   │
│  │ - Dynamic allocations           │   │
│  └─────────────────────────────────┘   │
├─────────────────────────────────────────┤
│  Flash (Program Storage)                │
│  - Application code                     │
│  - RIOT OS kernel                       │
│  - Constants (HTML, strings)            │
└─────────────────────────────────────────┘
```

## Configuration Storage (Future)

```
┌─────────────────────────────────────────┐
│         Flash Memory Layout             │
├─────────────────────────────────────────┤
│  Bootloader                             │
├─────────────────────────────────────────┤
│  Application Code                       │
├─────────────────────────────────────────┤
│  littlefs Filesystem                    │
│  ┌─────────────────────────────────┐   │
│  │ /config.bin                     │   │
│  │ - modbus_config_t structure     │   │
│  │                                 │   │
│  │ /wifi.conf                      │   │
│  │ - SSID                          │   │
│  │ - Password                      │   │
│  │ - Mode                          │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
```

## Network Architecture

### Access Point Mode

```
┌──────────────────────────────────────────┐
│         ESP32 Access Point               │
│         IP: 192.168.4.1                  │
│         DHCP Server: 192.168.4.2-100     │
└───────────┬──────────────────────────────┘
            │ WiFi (2.4 GHz)
            │
    ┌───────┴───────┬───────────────┐
    │               │               │
┌───▼────┐     ┌───▼────┐     ┌───▼────┐
│Laptop  │     │ Phone  │     │Tablet  │
│.4.100  │     │ .4.101 │     │ .4.102 │
└────────┘     └────────┘     └────────┘
```

### Station Mode

```
┌──────────────────────────────────────────┐
│     Existing WiFi Router/AP              │
│     DHCP Server                          │
└───────────┬──────────────────────────────┘
            │
    ┌───────┴───────┬───────────────┐
    │               │               │
┌───▼────┐     ┌───▼────┐     ┌───▼─────┐
│ ESP32  │     │Laptop  │     │ Other   │
│ (DHCP) │     │        │     │ Devices │
└────────┘     └────────┘     └─────────┘
```

## RS485 Physical Layer

```
┌─────────────┐                           ┌─────────────┐
│   Modbus    │                           │   Modbus    │
│   Master    │                           │   Slave     │
└──────┬──────┘                           └──────┬──────┘
       │                                         │
    ┌──▼──┐                                   ┌──▼──┐
    │ MAX │                                   │ MAX │
    │ 485 │                                   │ 485 │
    └─┬─┬─┘                                   └─┬─┬─┘
      │ │                                       │ │
      A B                                       A B
      │ │    [120Ω]                            │ │  [120Ω]
      └─┴──────┤                                └─┴────┤
                │                                      │
        ┌───────┴──────────────────────────────────────┴───────┐
        │              RS485 Bus (Twisted Pair)                │
        └───────┬──────────────────────────────────────┬───────┘
                │                                      │
             ┌──▼──┐                               ┌──▼──┐
             │ MAX │   ESP32 Proxy                │ MAX │
             │ 485 │   Interface 1                │ 485 │
             └─┬─┬─┘   Interface 2                └─┬─┬─┘
               │ │                                  │ │
            ┌──▼─▼─────────────────────────────────▼─▼──┐
            │           ESP32 Modbus Proxy              │
            │                                           │
            │  UART1 ◄──► IF1  │  IF2 ◄──► UART2      │
            └───────────────────────────────────────────┘
```

## Modbus Frame Format

```
┌──────┬──────┬──────┬────────────┬─────┬─────┐
│ Addr │ Func │ Data │    ...     │ CRC │ CRC │
│  1B  │  1B  │  nB  │            │  L  │  H  │
└──────┴──────┴──────┴────────────┴─────┴─────┘

Example Read Holding Registers (0x03) Request:
┌──────┬──────┬─────────┬─────────┬─────────┬─────────┬─────┬─────┐
│ 0x01 │ 0x03 │ 0x00    │ 0x00    │ 0x00    │ 0x0A    │ CRC │ CRC │
│ Addr │ Func │ Start H │ Start L │ Count H │ Count L │  L  │  H  │
└──────┴──────┴─────────┴─────────┴─────────┴─────────┴─────┴─────┘

Example Read Holding Registers (0x03) Response:
┌──────┬──────┬──────┬─────────┬─────────┬─────────┬─────┬─────┐
│ 0x01 │ 0x03 │ 0x14 │ 0x12    │ 0x34    │   ...   │ CRC │ CRC │
│ Addr │ Func │Bytes │ Reg1 H  │ Reg1 L  │   ...   │  L  │  H  │
└──────┴──────┴──────┴─────────┴─────────┴─────────┴─────┴─────┘
                          ▲
                          │
                    Modification applied here
```

## Web Interface Flow

```
┌─────────────┐
│   Browser   │
└──────┬──────┘
       │ HTTP GET /
       ▼
┌──────────────┐
│  CoAP Server │
└──────┬───────┘
       │ Serve HTML
       ▼
┌─────────────┐
│   Browser   │
│ Display UI  │
└──────┬──────┘
       │ POST /add?addr=1&reg=0&type=2&param=10
       ▼
┌──────────────┐
│  CoAP Server │
└──────┬───────┘
       │ Parse parameters
       ▼
┌──────────────┐
│modbus_add_   │
│    rule()    │
└──────┬───────┘
       │ Update config
       ▼
┌──────────────┐
│  Return OK   │
└──────┬───────┘
       │
       ▼
┌─────────────┐
│   Browser   │
│  Refresh    │
└─────────────┘
```

## State Machine

### Modbus Proxy State

```
         ┌──────────┐
         │   INIT   │
         └────┬─────┘
              │
              ▼
         ┌──────────┐
    ┌───►│   IDLE   │◄───┐
    │    └────┬─────┘    │
    │         │          │
    │         │ Frame    │ Frame
    │         │ Start    │ Complete
    │         ▼          │
    │    ┌──────────┐    │
    │    │RECEIVING │────┘
    │    └────┬─────┘
    │         │
    │         │ Timeout
    │         ▼
    │    ┌──────────┐
    │    │PROCESSING│
    │    └────┬─────┘
    │         │
    │         ▼
    │    ┌──────────┐
    └────┤FORWARDING│
         └──────────┘
```

### WiFi Connection State

```
         ┌──────────┐
         │  START   │
         └────┬─────┘
              │
              ▼
         ┌──────────┐
         │  AP Mode │
         └────┬─────┘
              │
              ▼
         ┌──────────┐
    ┌───►│  READY   │
    │    └────┬─────┘
    │         │
    │         │ Connect Command
    │         ▼
    │    ┌──────────┐
    │    │CONNECTING│
    │    └────┬─────┘
    │         │
    │         ├───── Success ────┐
    │         │                  │
    │         └── Fail           │
    │             │              │
    │             ▼              ▼
    │    ┌──────────┐      ┌──────────┐
    └────┤  AP Mode │      │ STA Mode │
         └──────────┘      └────┬─────┘
                                │
                                │ Disconnect
                                ▼
                           ┌──────────┐
                           │  AP Mode │
                           └──────────┘
```

## Performance Characteristics

### Latency Analysis

```
Total Latency = RX_Time + Process_Time + TX_Time

RX_Time:     ~0.5ms  (for typical 8-byte frame at 9600 baud)
Process_Time: <1ms   (frame parsing + modification)
TX_Time:     ~0.5ms  (for typical 8-byte frame at 9600 baud)

Total:       ~2ms    (typical)
```

### Throughput Capacity

```
Baud Rate: 9600 bps
Effective: ~960 bytes/second (with overhead)

Typical Modbus Frame: 8-20 bytes
Max Throughput: ~48-120 frames/second

Actual Throughput: ~30-40 frames/second
(includes protocol overhead and processing time)
```

## Error Handling

```
┌─────────────┐
│ Frame RX    │
└──────┬──────┘
       │
       ▼
┌─────────────┐      NO    ┌─────────────┐
│ CRC Valid?  │────────────►│   Discard   │
└──────┬──────┘            └─────────────┘
       │ YES
       ▼
┌─────────────┐
│ Apply Rules │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ Recalc CRC  │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Forward    │
└─────────────┘
```

## Future Enhancements Architecture

### With Configuration Persistence

```
┌─────────────────────────────────────────┐
│              ESP32                      │
│  ┌──────────────────────────────────┐  │
│  │         Application              │  │
│  └────────┬─────────────────────────┘  │
│           │                            │
│  ┌────────▼─────────────────────────┐  │
│  │   VFS (Virtual File System)      │  │
│  └────────┬─────────────────────────┘  │
│           │                            │
│  ┌────────▼─────────────────────────┐  │
│  │   littlefs2 Filesystem           │  │
│  └────────┬─────────────────────────┘  │
│           │                            │
│  ┌────────▼─────────────────────────┐  │
│  │   Flash Memory                   │  │
│  │   - config.bin                   │  │
│  │   - wifi.conf                    │  │
│  └──────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### With MQTT Support

```
┌──────────────┐       MQTT        ┌──────────────┐
│   ESP32      │◄─────────────────►│ MQTT Broker  │
│ (Publisher)  │                   │              │
└──────────────┘                   └───────┬──────┘
                                           │
                                           │
                                   ┌───────▼──────┐
                                   │ Subscribers  │
                                   │ - Monitor    │
                                   │ - Logger     │
                                   │ - Analytics  │
                                   └──────────────┘

Topics:
- modbus/proxy/status
- modbus/proxy/stats
- modbus/proxy/config
- modbus/proxy/frames
```

## Legend

```
┌─────┐
│ Box │   Component or Module
└─────┘

  │     Vertical connection
  ─     Horizontal connection
  
  ▼     Data flow direction
  
  ◄──►  Bidirectional communication

[120Ω]  Resistor or passive component
```
