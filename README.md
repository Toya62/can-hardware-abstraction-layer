# CAN Hardware Abstraction Layer (HAL) 🚗⚡

[![CI](https://github.com/Toya62/can-hardware-abstraction-layer/actions/workflows/ci.yml/badge.svg)](https://github.com/Toya62/can-hardware-abstraction-layer/actions/workflows/ci.yml)
[![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build](https://img.shields.io/badge/Build-CMake%203.20%2B-brightgreen.svg)](https://cmake.org)

A modern **C++17 Hardware Abstraction Layer (HAL)** designed to decouple upper-layer automotive/industrial embedded applications from physical, vendor-specific **Controller Area Network (CAN)** transceiver hardware.

---

## 🌟 Why This Exists

In automotive ECUs, autonomous mobile robots (AMRs), and defense platforms, CAN controllers vary significantly:
- Some reside directly on the microcontroller bus (e.g., STM32, NXP).
- Others are connected via external peripheral buses (SPI, I²C) or memory-mapped peripherals.

Tightly coupling vehicle telemetry or diagnostic code to concrete silicon creates maintenance bottlenecks. This repository demonstrates a production-grade **3-tier hardware abstraction architecture**:

```
+-------------------------------------------------------------------+
|                        Application Layer                          |
|             (Telemetry, Diagnostics, Cyclic Commands)             |
+-------------------------------------------------------------------+
                                 │
                   [Uniform Abstract Interface]
                                 ▼
+-------------------------------------------------------------------+
|                  ICANInterface (Pure Abstract)                    |
|      - initialize()        - setBaudrate()       - sendMessage()  |
|      - setActive()         - setPassive()        - getLastError() |
|      - registerReceiveCallback()                 - handleMessage()|
+-------------------------------------------------------------------+
                                 │
                        [Shared Base State]
                                 ▼
+-------------------------------------------------------------------+
|                     ACANBase (Abstract Base)                      |
|       Buffer management, state flags, simulated ISR interrupts    |
+-------------------------------------------------------------------+
         │                 │                 │                 │
         ▼                 ▼                 ▼                 ▼
   ┌───────────┐     ┌───────────┐     ┌───────────┐     ┌───────────┐
   │ CANTypeA  │     │ CANTypeB  │     │ CANTypeC  │     │ CANTypeE  │
   │(On-Chip)  │     │(Stateful) │     │(Mem-Mapped│     │(Struct Ext│
   └───────────┘     └───────────┘     └───────────┘     └───────────┘
```

---

## 🚀 Key Architectural Features

- **Polymorphic Driver Dispatch**: Upper layers operate purely on `ICANInterface*` pointers. Swapping physical hardware requires zero application code changes.
- **Asynchronous ISR Callbacks**: Hardware Interrupt Service Routines (ISRs) parse incoming CAN frames non-blockingly and dispatch them through registered function pointers.
- **State Machine & Frame Validation**: Strictly enforces controller states (`Initialized`, `Active`, `Passive`) and CAN 2.0A/B payload constraints (1–8 bytes).
- **Controlled Hardware Bypass Pattern**: Vendor-specific controller capabilities (e.g., hardware-level multi-frame structs in `CANTypeE`) can be directly accessed without polluting the universal interface.
- **Strong Typing**: Scoped enumerations (`eBaudrate`, `eCANError`) guarantee compile-time safety and eliminate magic numbers.
- **Modern Build & Test Tooling**: Clean CMake build system with zero warnings under `-Wall -Wextra -Wpedantic` and unit test coverage.

---

## 📁 Repository Structure

```text
can-hardware-abstraction-layer/
├── CMakeLists.txt              # Standard modern CMake build definition
├── LICENSE                     # MIT License
├── README.md                   # Project overview and quickstart guide
├── .github/
│   └── workflows/ci.yml        # Multi-compiler CI workflow (GCC, Clang, macOS)
├── docs/
│   ├── ARCHITECTURE.md         # Detailed architectural whitepaper
│   └── CAN_UML.drawio          # Comprehensive UML class & component diagram
├── include/
│   └── can_hal/
│       ├── ICANInterface.hpp   # Universal pure abstract CAN interface
│       ├── ACANBase.hpp        # Shared base class (state & buffer management)
│       ├── CANTypeA.hpp        # On-chip microcontroller peripheral driver
│       ├── CANTypeB.hpp        # Stateful reset-capable driver
│       ├── CANTypeC.hpp        # Memory-mapped peripheral driver
│       ├── CANTypeD.hpp        # Custom state-managed driver
│       └── CANTypeE.hpp        # Advanced controller with struct-bypass extension
├── src/
│   └── main.cpp                # Multi-controller bus demo application
└── tests/
    └── test_can_hal.cpp        # Test suite verifying lifecycle, bounds, and callbacks
```

---

## 🛠️ Build & Run Instructions

### Prerequisites
- Modern C++ compiler supporting **C++17** (GCC 9+, Clang 10+, Apple Clang, MSVC 2019+)
- **CMake 3.20+**

### 1. Build the Project
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### 2. Run the Demo Application
```bash
./build/can_hal_demo
```

### 3. Run the Automated Unit Tests
```bash
ctest --test-dir build --output-on-failure
```

---

## 💻 Code Example

### Uniform Interface Usage & Asynchronous Reception
```cpp
#include "can_hal/ICANInterface.hpp"
#include "can_hal/CANTypeA.hpp"
#include <cstdio>

void onMessageReceived(uint8_t* data, uint8_t length, eCANError error) {
    if (error == eCANError::ERR_OK) {
        printf("Received %d bytes from CAN bus!\n", length);
    }
}

int main() {
    CANTypeA hardwareController;
    ICANInterface* can = &hardwareController;

    // Standard lifecycle via polymorphic interface
    can->initialize();
    can->setBaudrate(eBaudrate::BAUD_500K);
    can->setActive();
    can->registerReceiveCallback(onMessageReceived);

    // Synchronous transmission
    uint8_t payload[] = {0x10, 0x20, 0x30, 0x40};
    can->sendMessage(payload, 4);

    return 0;
}
```

---

## 🧪 Unit Test Coverage

The test suite in [`tests/test_can_hal.cpp`](tests/test_can_hal.cpp) validates:
1. **Lifecycle & State Transitions**: Validates `initialize()`, `setActive()`, `setPassive()` enforcement.
2. **Payload Validation**: Ensures compliance with standard CAN payload sizes (rejects 0 or >8 bytes).
3. **Baudrate Arbitration**: Verifies 125k, 250k, and 500k rate configurations.
4. **Asynchronous ISR Callbacks**: Simulates hardware interrupts and verifies non-blocking payload dispatch.
5. **Polymorphic Dispatch**: Executes uniform transmission logic seamlessly across all 5 controller types.
6. **Hardware Bypass Security**: Validates struct-based transmission, pointer safety, and bounds checking on `CANTypeE`.

---

## 📄 License
This project is licensed under the [MIT License](LICENSE).
