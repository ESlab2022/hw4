#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "mbed-trace/mbed_trace.h"

#ifndef __MAGNETIC_FIELD_SERVICE
#define __MAGNETIC_FIELD_SERVICE

inline void int16touint8(int16_t ab, uint8_t *a, uint8_t *b){
    *a = uint8_t(ab >> 8);
    *b = uint8_t(ab & 0xFF);
}

inline void process_magnetic_field_data(int16_t *int16_arr, uint8_t *uint8_arr){
    int16_t raw_int16;
    uint8_t a, b;
    
    for(int i = 0; i < 3; i++){
        raw_int16 = int16_arr[i];
        int16touint8(raw_int16, &a, &b);
        uint8_arr[i*2] = a;
        uint8_arr[i*2+1] = b;
    }
}

class MagneticFieldService {
public:
    const static uint16_t MAGNETIC_FIELD_SERVICE_UUID              = 0xA000;
    const static uint16_t MAGNETIC_FIELD_STATE_CHARACTERISTIC_UUID = 0xA001;

    MagneticFieldService(BLE &_ble) :
        ble(_ble), MagneticFieldState(MAGNETIC_FIELD_STATE_CHARACTERISTIC_UUID, nullptr, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&MagneticFieldState};
        GattService magneticFieldService(MAGNETIC_FIELD_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(magneticFieldService);
    }

    GattAttribute::Handle_t getValueHandle() const
    {
        return MagneticFieldState.getValueHandle();
    }

    void updateMageneticField(int16_t *magnetic_field) {
        uint8_t processed_magnetic_field[]={0,0,0,0,0,0};
        process_magnetic_field_data(magnetic_field, processed_magnetic_field);
        
        ble.gattServer().write(
            MagneticFieldState.getValueHandle(),
            processed_magnetic_field,
            sizeof(uint8_t) * 6
        );
    }

private:
    BLE &ble;
    ReadOnlyArrayGattCharacteristic<uint8_t, 6> MagneticFieldState;
};

#endif /* #ifndef __MAGNETIC_FIELD_SERVICE */