# Architecture & Technical Design Specification

## 1. Executive Summary & Design Motivation

In modern automotive, defense, and industrial embedded systems, the **Controller Area Network (CAN)** bus is the backbone of inter-ECU communication. However, physical CAN transceivers and controllers vary wildly across hardware revisions:
- **Integrated Controllers**: Located directly on the microcontroller bus (e.g., STM32 bxCAN/FDCAN, NXP FlexCAN).
- **External Transceivers**: Interfaced via peripheral buses such as SPI or I²C (e.g., Microchip MCP2515).
- **Memory-Mapped Controllers**: Controlled via dedicated DMA or mapped memory blocks.

Without a rigorous architectural abstraction, upper-layer application software (diagnostics, vehicle telemetry, cyclic command frames) becomes tightly coupled to specific silicon implementations. Replacing hardware or migrating to a next-generation SoC requires invasive application rewrites.

**CAN Hardware Abstraction Layer (CAN HAL)** solves this through a clean, decoupled 3-tier architecture adhering to Modern C++17 design principles.

---

## 2. High-Level Architectural Model

```
+-------------------------------------------------------------+
|                     Application Layer                       |
|           (Vehicle Telemetry, Diagnostics, Control)         |
+-------------------------------------------------------------+
                              |
                [Uniform Polymorphic Interface]
                              |
                              v
+-------------------------------------------------------------+
|              Uniform Interface (ICANInterface)              |
|        - Pure virtual contract for all CAN controllers      |
|        - Zero silicon dependencies                          |
+-------------------------------------------------------------+
                              |
                      [Inherits & Extends]
                              |
                              v
+-------------------------------------------------------------+
|                 Abstract Base (ACANBase)                    |
|        - Common state machine (Initialized, Active)         |
|        - Shared Rx ring-buffers & error state tracking      |
|        - Asynchronous Interrupt Service Routine (ISR) hook  |
+-------------------------------------------------------------+
                              |
       +----------------------+----------------------+
       |                      |                      |
       v                      v                      v
+--------------+       +--------------+       +--------------+
|   CANTypeA   |  ...  |   CANTypeD   |       |   CANTypeE   |
| (On-Chip MCU)|       | (FIFO Buffer)|       | (Struct Ext) |
+--------------+       +--------------+       +--------------+
                                                     ^
                                                     |  [Direct Bypass]
                 Application (Hardware Bypass) ------+
```

---

## 3. Design Patterns & Principles

### A. Interface Segregation Principle (ISP) — `ICANInterface`
`ICANInterface` defines the minimal universal surface required for standard CAN operations:
- Controller initialization: `initialize()`
- Baudrate arbitration: `setBaudrate(eBaudrate rate)`
- Synchronous frame transmission: `sendMessage(uint8_t* message, uint8_t length)`
- State transitions: `setActive()`, `setPassive()`
- Error querying: `getLastError()`
- Asynchronous notification registration: `registerReceiveCallback(MessageCallback cb)`

### B. Template Method & Code Reuse — `ACANBase`
Rather than duplicating buffer management, baudrate storage, and operational flags across every driver, `ACANBase` implements universal state tracking. Concrete drivers override `handleMessage()` to unpack controller-specific registers into the common RX buffer and invoke the application callback.

### C. Asynchronous Observer / ISR Callback Hook
Vehicle networks operate under strict real-time deadlines. Asynchronous frame reception is decoupled using non-blocking function pointer callbacks (`MessageCallback`). When hardware triggers an interrupt, the driver parses incoming payload frames and dispatches to registered application handlers without blocking the bus.

### D. Controlled Hardware Bypass Pattern
Occasionally, specific silicon provides proprietary extensions (such as hardware-level multi-frame structs, timestamping, or filter masks). Rather than polluting the pure `ICANInterface` with vendor-specific methods, the **Hardware Bypass Pattern** permits the application to hold a direct typed reference to the concrete controller (`CANTypeE*`) exclusively for proprietary routines, while routing all standard vehicle traffic through the uniform interface.

---

## 4. Strong Typing & Error Model

The architecture enforces compile-time safety and self-documenting code via scoped enumerations:

```cpp
enum class eBaudrate {
    BAUD_125K = 125000,
    BAUD_250K = 250000,
    BAUD_500K = 500000
};

enum class eCANError {
    ERR_OK = 0,
    ERR_NOT_INITIALIZED,
    ERR_NOT_ACTIVE,
    ERR_INVALID_LENGTH,
    ERR_HARDWARE_FAULT
};
```

All driver methods validate frame length against standard CAN 2.0A/B specifications (1–8 bytes). Transmissions exceeding bounds or attempted while the controller is uninitialized or passive immediately return deterministic error codes.
