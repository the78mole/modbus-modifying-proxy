# Architecture Diagrams

## System Overview

```mermaid
graph TB
    subgraph ESP32["ESP32/ESP32-S3 - RIOT OS"]
        WiFiMgr[WiFi Manager]
        ModbusProxy[Modbus Proxy]
        WebIf[Web Interface]
        
        WiFiMgr --> ESPWiFi[ESP WiFi Stack]
        ModbusProxy --> RS485_1[RS485-1]
        ModbusProxy --> RS485_2[RS485-2]
        WebIf --> CoAP[CoAP Server]
    end
    
    ESPWiFi --> WiFiNet[WiFi Network]
    RS485_1 --> ModbusDev[Modbus Devices]
    RS485_2 --> ModbusDev
    CoAP --> Browser[Browser/Client]
```

## Data Flow

### Modbus Frame Processing

```mermaid
flowchart TD
    A[RS485 IF1 RX] --> B[UART RX Buffer]
    B --> C[Frame Detection<br/>Timeout-based]
    C --> D[Parse Modbus<br/>- Device Addr<br/>- Function Code<br/>- Registers]
    D --> E[Apply Rules<br/>- Lookup addr/reg<br/>- Apply modifier<br/>- Overwrite/Math]
    Config[Configuration] -.-> E
    E --> F[Recalculate CRC]
    F --> G[UART TX Buffer]
    G --> H[RS485 IF2 TX]
```

## Threading Model

```mermaid
graph TB
    Scheduler[RIOT OS Scheduler]
    
    Scheduler --> MainThread[Main Thread]
    Scheduler --> IF1to2[IF1→2 Thread]
    Scheduler --> IF2to1[IF2→1 Thread]
    Scheduler --> WebServer[Web Server Thread]
    Scheduler --> WiFiThread[WiFi Thread]
    
    MainThread --> Shell[Shell Loop]
    WebServer --> CoAP[CoAP Handler]
    WiFiThread --> ESPStack[ESP Stack]
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

```mermaid
graph TB
    subgraph ESP32["ESP32 Memory Map"]
        IRAM["IRAM (Instruction RAM)<br/>- RIOT kernel<br/>- Time-critical code"]
        
        subgraph DRAM["DRAM (Data RAM)"]
            Stack1[Stack - Main Thread]
            Stack2[Stack - IF1→2 Thread]
            Stack3[Stack - IF2→1 Thread]
            Stack4[Stack - Web Server Thread]
            Global[Global Variables<br/>- config<br/>- buffers]
            Heap[Heap<br/>- Dynamic allocations]
        end
        
        Flash["Flash (Program Storage)<br/>- Application code<br/>- RIOT OS kernel<br/>- Constants"]
    end
```

## Configuration Storage (Future)

```mermaid
graph TB
    subgraph Flash["Flash Memory Layout"]
        Boot[Bootloader]
        App[Application Code]
        subgraph FS["littlefs Filesystem"]
            Config["/config.bin<br/>- modbus_config_t structure"]
            WiFiConf["/wifi.conf<br/>- SSID<br/>- Password<br/>- Mode"]
        end
    end
    
    Boot -.-> App
    App -.-> FS
```

## Network Architecture

### Access Point Mode

```mermaid
graph TB
    AP["ESP32 Access Point<br/>IP: 192.168.4.1<br/>DHCP: 192.168.4.2-100"]
    
    AP -->|WiFi 2.4 GHz| Laptop[Laptop<br/>.4.100]
    AP -->|WiFi 2.4 GHz| Phone[Phone<br/>.4.101]
    AP -->|WiFi 2.4 GHz| Tablet[Tablet<br/>.4.102]
```

### Station Mode

```mermaid
graph TB
    Router["Existing WiFi Router/AP<br/>DHCP Server"]
    
    Router --> ESP32[ESP32<br/>DHCP]
    Router --> Laptop[Laptop]
    Router --> Other[Other Devices]
```

## RS485 Physical Layer

```mermaid
graph LR
    Master[Modbus Master] --> MAX1[MAX 485]
    Slave[Modbus Slave] --> MAX2[MAX 485]
    
    MAX1 -->|A/B| Term1[120Ω]
    MAX2 -->|A/B| Term2[120Ω]
    
    Term1 --> Bus[RS485 Bus<br/>Twisted Pair]
    Term2 --> Bus
    
    Bus --> ProxyIF1[MAX 485<br/>Interface 1]
    Bus --> ProxyIF2[MAX 485<br/>Interface 2]
    
    ProxyIF1 --> Proxy[ESP32 Modbus Proxy<br/>UART1 ◄──► IF1 │ IF2 ◄──► UART2]
    ProxyIF2 --> Proxy
```

## Modbus Frame Format

**Generic Modbus Frame:**
```
┌──────┬──────┬──────┬────────────┬─────┬─────┐
│ Addr │ Func │ Data │    ...     │ CRC │ CRC │
│  1B  │  1B  │  nB  │            │  L  │  H  │
└──────┴──────┴──────┴────────────┴─────┴─────┘
```

**Example Read Holding Registers (0x03) Request:**
```
┌──────┬──────┬─────────┬─────────┬─────────┬─────────┬─────┬─────┐
│ 0x01 │ 0x03 │ 0x00    │ 0x00    │ 0x00    │ 0x0A    │ CRC │ CRC │
│ Addr │ Func │ Start H │ Start L │ Count H │ Count L │  L  │  H  │
└──────┴──────┴─────────┴─────────┴─────────┴─────────┴─────┴─────┘
```

**Example Read Holding Registers (0x03) Response:**
```
┌──────┬──────┬──────┬─────────┬─────────┬─────────┬─────┬─────┐
│ 0x01 │ 0x03 │ 0x14 │ 0x12    │ 0x34    │   ...   │ CRC │ CRC │
│ Addr │ Func │Bytes │ Reg1 H  │ Reg1 L  │   ...   │  L  │  H  │
└──────┴──────┴──────┴─────────┴─────────┴─────────┴─────┴─────┘
                          ▲
                          │
                    Modification applied here
```

## Web Interface Flow

```mermaid
sequenceDiagram
    participant Browser
    participant CoAP as CoAP Server
    participant ModbusAPI as modbus_add_rule()
    participant Config
    
    Browser->>CoAP: HTTP GET /
    CoAP->>Browser: Serve HTML
    Browser->>Browser: Display UI
    Browser->>CoAP: POST /add?addr=1&reg=0&type=2&param=10
    CoAP->>CoAP: Parse parameters
    CoAP->>ModbusAPI: Call function
    ModbusAPI->>Config: Update config
    Config-->>ModbusAPI: OK
    ModbusAPI-->>CoAP: Return OK
    CoAP-->>Browser: Response
    Browser->>Browser: Refresh
```
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

```mermaid
stateDiagram-v2
    [*] --> INIT
    INIT --> IDLE
    IDLE --> RECEIVING: Frame Start
    RECEIVING --> IDLE: Frame Complete
    RECEIVING --> PROCESSING: Timeout
    PROCESSING --> FORWARDING
    FORWARDING --> IDLE
```

### WiFi Connection State

```mermaid
stateDiagram-v2
    [*] --> START
    START --> AP_Mode1: Initialize
    AP_Mode1 --> READY
    READY --> CONNECTING: Connect Command
    CONNECTING --> STA_Mode: Success
    CONNECTING --> AP_Mode2: Fail
    STA_Mode --> AP_Mode3: Disconnect
    AP_Mode2 --> READY
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

```mermaid
flowchart TD
    A[Frame RX] --> B{CRC Valid?}
    B -->|NO| C[Discard]
    B -->|YES| D[Apply Rules]
    D --> E[Recalc CRC]
    E --> F[Forward]
```

## Future Enhancements Architecture

### With Configuration Persistence

```mermaid
graph TB
    subgraph ESP32
        App[Application]
        VFS[VFS - Virtual File System]
        LFS[littlefs2 Filesystem]
        Flash[Flash Memory<br/>- config.bin<br/>- wifi.conf]
        
        App --> VFS
        VFS --> LFS
        LFS --> Flash
    end
```

### With MQTT Support

```
```mermaid
graph TB
    ESP32[ESP32<br/>Publisher] <-->|MQTT| Broker[MQTT Broker]
    Broker --> Sub[Subscribers<br/>- Monitor<br/>- Logger<br/>- Analytics]
```

**MQTT Topics:**
- `modbus/proxy/status`
- `modbus/proxy/stats`
- `modbus/proxy/config`
- `modbus/proxy/frames`

## Legend

**Diagram Symbols:**
- Boxes: Components or Modules
- Arrows: Data flow direction
- Double arrows (↔): Bidirectional communication
- `[120Ω]`: Resistor or passive component

