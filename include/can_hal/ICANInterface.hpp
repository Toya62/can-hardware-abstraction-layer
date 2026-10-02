#pragma once
#include <cstdint>

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

using MessageCallback = void(*)(uint8_t* data, uint8_t length, eCANError error);

class ICANInterface {
public:
    virtual bool initialize() = 0;
    virtual bool setBaudrate(eBaudrate rate) = 0;
    virtual bool sendMessage(uint8_t* message, uint8_t length) = 0;
    virtual bool handleMessage() = 0;
    virtual void setActive() = 0;
    virtual void setPassive() = 0;
    virtual eCANError getLastError() = 0;
    virtual void registerReceiveCallback(MessageCallback cb) = 0;
    virtual ~ICANInterface() = default;
};
