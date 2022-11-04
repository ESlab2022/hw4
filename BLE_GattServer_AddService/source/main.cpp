#include <cstdlib>
#include "MyHeartRateService.h"
#include "MagneticFieldService.h"
#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"

#include "pretty_printer.h"
#include "mbed-trace/mbed_trace.h"
#include "stm32l475e_iot01_magneto.h"


using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "MagneticField";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

class MagneticFieldDemo : ble::Gap::EventHandler {
public:
    MagneticFieldDemo(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _magnetic_field_uuid(MagneticFieldService::MAGNETIC_FIELD_SERVICE_UUID),
        _magnetic_field_value(nullptr),
        _magnetic_field_service(ble),
        _heartrate_uuid(MyHeartRateService::HEART_RATE_SERVICE_UUID),
        _heartrate_value(100),
        _heartrate_service(ble),
        _adv_data_builder(_adv_buffer)
    {
    }

    void start()
    {
        _ble.init(this, &MagneticFieldDemo::on_init_complete);

        _event_queue.dispatch_forever();
    }

private:
    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params)
    {
        if (params->error != BLE_ERROR_NONE) {
            printf("Ble initialization failed.");
            return;
        }

        print_mac_address();

        /* this allows us to receive events like onConnectionComplete() */
        _ble.gap().setEventHandler(this);

        /* heart rate value updated every second */
        _event_queue.call_every(
            1000ms,
            [this] {
                update_mag_sensor_value();
                update_hr_sensor_value();
            }
        );

        start_advertising();
    }

    void start_advertising()
    {
        /* Create advertising parameters and payload */

        ble::AdvertisingParameters adv_parameters(
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
            ble::adv_interval_t(ble::millisecond_t(100))
        );

        _adv_data_builder.setFlags();
        _adv_data_builder.setAppearance(ble::adv_data_appearance_t::GENERIC_HEART_RATE_SENSOR);
        _adv_data_builder.setLocalServiceList({&_magnetic_field_uuid, 1});
        _adv_data_builder.setLocalServiceList({&_heartrate_uuid, 1});
        _adv_data_builder.setName(DEVICE_NAME);

        /* Setup advertising */

        ble_error_t error = _ble.gap().setAdvertisingParameters(
            ble::LEGACY_ADVERTISING_HANDLE,
            adv_parameters
        );

        if (error) {
            printf("_ble.gap().setAdvertisingParameters() failed\r\n");
            return;
        }

        error = _ble.gap().setAdvertisingPayload(
            ble::LEGACY_ADVERTISING_HANDLE,
            _adv_data_builder.getAdvertisingData()
        );

        if (error) {
            printf("_ble.gap().setAdvertisingPayload() failed\r\n");
            return;
        }

        /* Start advertising */

        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            printf("_ble.gap().startAdvertising() failed\r\n");
            return;
        }

        printf("Magnetic field sensor and heart rate sensor advertising, please connect\r\n");
    }

    void update_mag_sensor_value(){
        int16_t magnetic_field[3]={10,11,12};
        BSP_MAGNETO_GetXYZ(magnetic_field);
        printf("%d %d %d\n", magnetic_field[0],magnetic_field[1],magnetic_field[2]);
        _magnetic_field_service.updateMageneticField(magnetic_field);
    }

    void update_hr_sensor_value()
    {
        /* you can read in the real value but here we just simulate a value */
        _heartrate_value++;
        /*  60 <= bpm value < 110 */
        if (_heartrate_value == 110) {
            _heartrate_value = 60;
        }
        printf("%d bpm", _heartrate_value);
        _heartrate_service.updateHeartRate(&_heartrate_value);
    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;

    UUID _magnetic_field_uuid;
    uint16_t *_magnetic_field_value;
    MagneticFieldService _magnetic_field_service;

    UUID _heartrate_uuid;
    uint8_t _heartrate_value;
    MyHeartRateService _heartrate_service;

    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
};



/* Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    BSP_MAGNETO_Init();

    mbed_trace_init();

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    MagneticFieldDemo demo(ble, event_queue);
    demo.start();

    return 0;
}
