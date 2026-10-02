#pragma once
#include "ACANBase.hpp"

struct messageStruct {
    uint32_t id;
    uint8_t  data[8];
    uint8_t  length;
};

using MessageStruct = messageStruct;

class CANTypeE : public ACANBase {
public:
    bool handleMessage() override {
        if (!active_ || !initialized_) return false;
        if (receiveCallback_ && rxLength_ > 0) {
            printf("[CANTypeE] Hardware Rx frame parsed (%d bytes)\n", rxLength_);
            receiveCallback_(rxBuffer_, rxLength_, rxError_);
        }
        return true;
    }

    CANTypeE() : ACANBase() {}

    bool initialize() override {
        ACANBase::initialize();
        printf("[CANTypeE] Initialized\n");
        return true;
    }

    bool setBaudrate(eBaudrate rate) override {
        ACANBase::setBaudrate(rate);
        printf("[CANTypeE] BaudRate set to %u\n", static_cast<uint32_t>(rate));
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

        printf("[CANTypeE] Sending %d bytes\n", length);
        lastError_ = eCANError::ERR_OK;
        return true;
    }

    void setActive() override {
        ACANBase::setActive();
        printf("[CANTypeE] Active\n");
    }

    void setPassive() override {
        ACANBase::setPassive();
        printf("[CANTypeE] Passive\n");
    }

    bool sendMessageStruct(const messageStruct* msg) {
        if (!msg) {
            lastError_ = eCANError::ERR_INVALID_LENGTH;
            return false;
        }
        if (!initialized_) {
            lastError_ = eCANError::ERR_NOT_INITIALIZED;
            return false;
        }
        if (!active_) {
            lastError_ = eCANError::ERR_NOT_ACTIVE;
            return false;
        }
        if (msg->length < 1 || msg->length > 8) {
            lastError_ = eCANError::ERR_INVALID_LENGTH;
            return false;
        }

        printf("[CANTypeE] Sending struct message, ID=0x%X, length=%d\n", msg->id, msg->length);
        lastError_ = eCANError::ERR_OK;
        return true;
    }
};
