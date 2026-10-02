#include <cstdio>
#include <cstdint>
#include "ICANInterface.hpp"
#include "CANTypeA.hpp"
#include "CANTypeB.hpp"
#include "CANTypeC.hpp"
#include "CANTypeD.hpp"
#include "CANTypeE.hpp"

void onMessageReceived(uint8_t* data, uint8_t length, eCANError error) {
    if (error != eCANError::ERR_OK) {
        printf("[App] Receive error: %d\n", static_cast<int>(error));
        return;
    }
    printf("[App] Received %d bytes: ", length);
    for (int i = 0; i < length; i++) printf("%02X ", data[i]);
    printf("\n");
}

class Application {
private:
    ICANInterface* can_;
    CANTypeE*       canTypeE_;

public:
    Application(ICANInterface* can, CANTypeE* typeE = nullptr)
        : can_(can), canTypeE_(typeE) {}

    void setCANInterface(ICANInterface* can) { can_ = can; }

    void runUniformLogic() {
        if (!can_) return;
        can_->initialize();
        can_->setBaudrate(eBaudrate::BAUD_500K);
        can_->setActive();
        can_->registerReceiveCallback(onMessageReceived);

        uint8_t data[] = {0x10, 0x20, 0x30, 0x40};
        bool ok = can_->sendMessage(data, 4);
        if (!ok) {
            printf("[App] Send failed: %d\n", static_cast<int>(can_->getLastError()));
        }

        // Trigger driver handleMessage() through uniform ICANInterface pointer
        can_->handleMessage();
    }

    void runHardwareBypassLogic() {
        if (!canTypeE_) return;
        MessageStruct msg = { 0x1A2B, {0xAA, 0xBB, 0xCC}, 3 };
        canTypeE_->sendMessageStruct(&msg);
    }
};

int main() {
    printf("=== Using CANTypeA ===\n");
    CANTypeA typeA;
    uint8_t incoming[] = {0xDE, 0xAD, 0xBE, 0xEF};
    typeA.triggerHardwareInterrupt(incoming, 4);
    Application app(&typeA);
    app.runUniformLogic();

    printf("\n=== Using CANTypeB ===\n");
    CANTypeB typeB;
    app.setCANInterface(&typeB);
    app.runUniformLogic();

    printf("\n=== Using CANTypeC ===\n");
    CANTypeC typeC;
    app.setCANInterface(&typeC);
    app.runUniformLogic();

    printf("\n=== Using CANTypeD ===\n");
    CANTypeD typeD;
    app.setCANInterface(&typeD);
    app.runUniformLogic();

    printf("\n=== Using CANTypeE ===\n");
    CANTypeE typeE;
    app.setCANInterface(&typeE);
    app.runUniformLogic();

    printf("\n=== CANTypeE Special Function ===\n");
    Application bypassApp(&typeE, &typeE);
    bypassApp.runHardwareBypassLogic();

    return 0;
}
