#ifndef __MY_HEART_RATE_SERVICE_H__
#define __MY_HEART_RATE_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"

#if BLE_FEATURE_GATT_SERVER

class MyHeartRateService {
public:
    const static uint16_t HEART_RATE_SERVICE_UUID              = 0xA002;
    const static uint16_t HEART_RATE_STATE_CHARACTERISTIC_UUID = 0xA003;

    MyHeartRateService(BLE &_ble) :
        ble(_ble), HeartRateState(HEART_RATE_STATE_CHARACTERISTIC_UUID, nullptr, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&HeartRateState};
        GattService heartRateService(HEART_RATE_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(heartRateService);
    }

    GattAttribute::Handle_t getValueHandle() const
    {
        return HeartRateState.getValueHandle();
    }

    void updateHeartRate(uint8_t *heart_rate) {
        ble.gattServer().write(
            HeartRateState.getValueHandle(),
            heart_rate,
            sizeof(uint8_t)
        );
    }

private:
    BLE &ble;
    ReadOnlyGattCharacteristic<uint8_t> HeartRateState;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef MBED_BLE_HEART_RATE_SERVICE_H__*/
