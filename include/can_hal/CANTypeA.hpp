#pragma once
#include "ACANBase.hpp"

class CANTypeA : public ACANBase {
public:
    bool handleMessage() override {
        if (!active_ || !initialized_) return false;
        if (receiveCallback_ && rxLength_ > 0) {
            printf("[CANTypeA] Hardware Rx frame parsed (%d bytes)\n", rxLength_);
            receiveCallback_(rxBuffer_, rxLength_, rxError_);
        }
        return true;
    }

    CANTypeA() : ACANBase() {}

    bool initialize() override {
        ACANBase::initialize();
        printf("[CANTypeA] Initialized\n");
        return true;
    }

    bool setBaudrate(eBaudrate rate) override {
        ACANBase::setBaudrate(rate);
        printf("[CANTypeA] BaudRate set to %u\n", static_cast<uint32_t>(rate));
        return true;
    }

    bool sendMessage(uint8_t* message, uint8_t length) override {
        (void)message;
        if (!initialized_) {
            lastError_ = eCANError::ERR_NOT_INITIALIZED;
            return false;
        }
        if (!active_) {
            lastError_ = eCANError::ERR_NOT_ACTIVE;
            return false;
        }
        if (length < 1 || length > 8) {
            lastError_ = eCANError::ERR_INVALID_LENGTH;
            return false;
        }

        printf("[CANTypeA] Sending %d bytes\n", length);
        lastError_ = eCANError::ERR_OK;
        return true;
    }

    void setActive() override {
        ACANBase::setActive();
        printf("[CANTypeA] Active\n");
    }

    void setPassive() override {
        ACANBase::setPassive();
        printf("[CANTypeA] Passive\n");
    }
};
