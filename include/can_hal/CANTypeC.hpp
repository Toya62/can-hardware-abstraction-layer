#pragma once
#include "ACANBase.hpp"

class CANTypeC : public ACANBase {
protected:
    void* messageReceiver_;

private:
    bool initTypeSpecific() {
        printf("[CANTypeC] Type-specific initialization\n");
        return true;
    }

public:
    bool handleMessage() override {
        if (!active_ || !initialized_) return false;
        if (receiveCallback_ && rxLength_ > 0) {
            printf("[CANTypeC] Hardware Rx frame parsed (%d bytes)\n", rxLength_);
            receiveCallback_(rxBuffer_, rxLength_, rxError_);
        }
        return true;
    }

    CANTypeC() : ACANBase(), messageReceiver_(nullptr) {}

    bool initialize() override {
        ACANBase::initialize();
        initTypeSpecific();
        printf("[CANTypeC] Initialized\n");
        return true;
    }

    bool setBaudrate(eBaudrate rate) override {
        ACANBase::setBaudrate(rate);
        printf("[CANTypeC] BaudRate set to %u\n", static_cast<uint32_t>(rate));
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

        printf("[CANTypeC] Sending %d bytes\n", length);
        lastError_ = eCANError::ERR_OK;
        return true;
    }

    void setActive() override {
        ACANBase::setActive();
        printf("[CANTypeC] Active\n");
    }

    void setPassive() override {
        ACANBase::setPassive();
        printf("[CANTypeC] Passive\n");
    }
};
