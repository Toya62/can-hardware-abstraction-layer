#pragma once
#include "ICANInterface.hpp"
#include <cstdio>
#include <cstring>

class ACANBase : public ICANInterface {
protected:
    eBaudrate baudrate_;
    bool initialized_;
    bool active_;
    eCANError lastError_;
    MessageCallback receiveCallback_;

    uint8_t rxBuffer_[8];
    uint8_t rxLength_;
    eCANError rxError_;

public:
    ACANBase() : baudrate_(eBaudrate::BAUD_500K), initialized_(false), active_(false),
                 lastError_(eCANError::ERR_OK), receiveCallback_(nullptr),
                 rxLength_(0), rxError_(eCANError::ERR_OK) {
        memset(rxBuffer_, 0, sizeof(rxBuffer_));
    }

    bool initialize() override {
        initialized_ = true;
        lastError_ = eCANError::ERR_OK;
        return true;
    }

    bool setBaudrate(eBaudrate rate) override {
        baudrate_ = rate;
        lastError_ = eCANError::ERR_OK;
        return true;
    }

    void setActive() override { active_ = true; }
    void setPassive() override { active_ = false; }
    eCANError getLastError() override { return lastError_; }
    void registerReceiveCallback(MessageCallback cb) override { receiveCallback_ = cb; }

    void triggerHardwareInterrupt(uint8_t* data, uint8_t length, eCANError error = eCANError::ERR_OK) {
        if (length > 8) length = 8;
        memcpy(rxBuffer_, data, length);
        rxLength_ = length;
        rxError_ = error;
        handleMessage();
    }

    virtual ~ACANBase() = default;
};
