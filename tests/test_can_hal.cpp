#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#include "can_hal/ICANInterface.hpp"
#include "can_hal/ACANBase.hpp"
#include "can_hal/CANTypeA.hpp"
#include "can_hal/CANTypeB.hpp"
#include "can_hal/CANTypeC.hpp"
#include "can_hal/CANTypeD.hpp"
#include "can_hal/CANTypeE.hpp"

// Global verification context for callbacks
static uint8_t g_lastRxData[8] = {0};
static uint8_t g_lastRxLength = 0;
static eCANError g_lastRxError = eCANError::ERR_HARDWARE_FAULT;
static int g_rxCallbackCount = 0;

void testRxCallback(uint8_t* data, uint8_t length, eCANError error) {
    g_rxCallbackCount++;
    g_lastRxLength = length;
    g_lastRxError = error;
    if (data && length <= 8) {
        memcpy(g_lastRxData, data, length);
    }
}

void resetCallbackTracker() {
    memset(g_lastRxData, 0, sizeof(g_lastRxData));
    g_lastRxLength = 0;
    g_lastRxError = eCANError::ERR_HARDWARE_FAULT;
    g_rxCallbackCount = 0;
}

void test_initialization_and_state() {
    printf("[TEST] Running test_initialization_and_state...\n");
    CANTypeA dev;

    uint8_t payload[] = {0x01, 0x02};

    // Sending before init should fail
    bool ok = dev.sendMessage(payload, 2);
    assert(!ok);
    assert(dev.getLastError() == eCANError::ERR_NOT_INITIALIZED);

    // Initialize
    assert(dev.initialize());
    assert(dev.getLastError() == eCANError::ERR_OK);

    // Sending when initialized but passive should fail
    ok = dev.sendMessage(payload, 2);
    assert(!ok);
    assert(dev.getLastError() == eCANError::ERR_NOT_ACTIVE);

    // Activate
    dev.setActive();
    ok = dev.sendMessage(payload, 2);
    assert(ok);
    assert(dev.getLastError() == eCANError::ERR_OK);

    // Set passive again
    dev.setPassive();
    ok = dev.sendMessage(payload, 2);
    assert(!ok);
    assert(dev.getLastError() == eCANError::ERR_NOT_ACTIVE);

    printf("  -> Passed.\n");
}

void test_payload_length_validation() {
    printf("[TEST] Running test_payload_length_validation...\n");
    CANTypeB dev;
    dev.initialize();
    dev.setActive();

    uint8_t validPayload[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    // 0 bytes (invalid)
    assert(!dev.sendMessage(validPayload, 0));
    assert(dev.getLastError() == eCANError::ERR_INVALID_LENGTH);

    // 1 to 8 bytes (valid)
    for (uint8_t len = 1; len <= 8; ++len) {
        assert(dev.sendMessage(validPayload, len));
        assert(dev.getLastError() == eCANError::ERR_OK);
    }

    // >8 bytes (invalid for standard CAN 2.0)
    assert(!dev.sendMessage(validPayload, 9));
    assert(dev.getLastError() == eCANError::ERR_INVALID_LENGTH);

    printf("  -> Passed.\n");
}

void test_baudrate_configuration() {
    printf("[TEST] Running test_baudrate_configuration...\n");
    CANTypeC dev;
    assert(dev.initialize());

    assert(dev.setBaudrate(eBaudrate::BAUD_125K));
    assert(dev.getLastError() == eCANError::ERR_OK);

    assert(dev.setBaudrate(eBaudrate::BAUD_250K));
    assert(dev.getLastError() == eCANError::ERR_OK);

    assert(dev.setBaudrate(eBaudrate::BAUD_500K));
    assert(dev.getLastError() == eCANError::ERR_OK);

    printf("  -> Passed.\n");
}

void test_async_rx_callback_and_interrupt() {
    printf("[TEST] Running test_async_rx_callback_and_interrupt...\n");
    resetCallbackTracker();

    CANTypeD dev;
    dev.initialize();
    dev.setActive();
    dev.registerReceiveCallback(testRxCallback);

    uint8_t isrData[4] = {0xCA, 0xFE, 0xBA, 0xBE};
    dev.triggerHardwareInterrupt(isrData, 4, eCANError::ERR_OK);

    assert(g_rxCallbackCount == 1);
    assert(g_lastRxLength == 4);
    assert(g_lastRxError == eCANError::ERR_OK);
    assert(memcmp(g_lastRxData, isrData, 4) == 0);

    // Test with simulated error in ISR
    resetCallbackTracker();
    uint8_t errData[2] = {0xEE, 0x01};
    dev.triggerHardwareInterrupt(errData, 2, eCANError::ERR_HARDWARE_FAULT);

    assert(g_rxCallbackCount == 1);
    assert(g_lastRxLength == 2);
    assert(g_lastRxError == eCANError::ERR_HARDWARE_FAULT);

    printf("  -> Passed.\n");
}

void test_polymorphic_dispatch() {
    printf("[TEST] Running test_polymorphic_dispatch across all 5 drivers...\n");
    CANTypeA devA;
    CANTypeB devB;
    CANTypeC devC;
    CANTypeD devD;
    CANTypeE devE;

    ICANInterface* controllers[] = {&devA, &devB, &devC, &devD, &devE};
    uint8_t testFrame[] = {0xAA, 0x55};

    for (size_t i = 0; i < 5; ++i) {
        ICANInterface* iface = controllers[i];
        assert(iface->initialize());
        assert(iface->setBaudrate(eBaudrate::BAUD_250K));
        iface->setActive();
        assert(iface->sendMessage(testFrame, sizeof(testFrame)));
        assert(iface->getLastError() == eCANError::ERR_OK);
        iface->setPassive();
    }

    printf("  -> Passed.\n");
}

void test_hardware_bypass_type_e() {
    printf("[TEST] Running test_hardware_bypass_type_e...\n");
    CANTypeE devE;
    assert(devE.initialize());
    devE.setActive();

    MessageStruct msg;
    msg.id = 0x7E8;
    msg.length = 3;
    msg.data[0] = 0x41;
    msg.data[1] = 0x0C;
    msg.data[2] = 0x1A;

    assert(devE.sendMessageStruct(&msg));
    assert(devE.getLastError() == eCANError::ERR_OK);

    // Invalid length via struct
    msg.length = 10;
    assert(!devE.sendMessageStruct(&msg));
    assert(devE.getLastError() == eCANError::ERR_INVALID_LENGTH);

    // Null pointer
    assert(!devE.sendMessageStruct(nullptr));

    printf("  -> Passed.\n");
}

int main() {
    printf("==================================================\n");
    printf("     CAN Hardware Abstraction Layer Unit Tests     \n");
    printf("==================================================\n");

    test_initialization_and_state();
    test_payload_length_validation();
    test_baudrate_configuration();
    test_async_rx_callback_and_interrupt();
    test_polymorphic_dispatch();
    test_hardware_bypass_type_e();

    printf("\nAll 6 test suites passed successfully!\n");
    return 0;
}
