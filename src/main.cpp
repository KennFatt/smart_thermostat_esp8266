#include <Arduino.h>
#include <DallasTemperature.h>
#include <OTAHandler.h>
#include <OneWire.h>
#include <ThingerESP8266.h>

#include <FanController.hpp>
#include <LCDController.hpp>

/** -------------------------------------- Definitions ------------------------------------- */
#ifndef SSID_NAME
#define SSID_NAME ""
#endif
#ifndef SSID_PSK
#define SSID_PSK ""
#endif

#ifndef THINGER_USERNAME
#define THINGER_USERNAME ""
#endif
#ifndef THINGER_DEVICE_ID
#define THINGER_DEVICE_ID ""
#endif
#ifndef THINGER_DEVICE_CREDS
#define THINGER_DEVICE_CREDS ""
#endif

/** ----------------------------------------- Pins ----------------------------------------- */
static const uint8_t PIN_TEMPERATURE = D7;
static const uint8_t PIN_FAN_INA     = D5;
static const uint8_t PIN_FAN_INB     = D6;

/** ----------------------------------- Library Instance ----------------------------------- */
ThingerESP8266 thing(THINGER_USERNAME, THINGER_DEVICE_ID, THINGER_DEVICE_CREDS);
OneWire one_wire(PIN_TEMPERATURE);
DallasTemperature sensor_temperature(&one_wire);
LCDController lcd_controller;
FanController fan_controller;

/** ---------------------------------------- States ---------------------------------------- */
struct TemperatureSensorState {
    float temperature_c = 0.0F;
} temperature_state;

struct LCDState {
    bool backlight = false;
} lcd_state;

struct FanState {
    uint16_t speed = FanController::FanSpeed::FAN_OFF;

    bool active                 = false;
    bool static_mode            = false;
    int8_t controlled_temp_max  = 100;
    int8_t controlled_temp_min  = 0;
} fan_state;

/** --------------------------------------- Internal --------------------------------------- */
static const unsigned long FETCH_TIME = 5000UL;
unsigned long last_fetch_time         = 0UL;
unsigned long millis_counter          = 0UL;

inline void onFetch();
inline void handleTemperatureSensor();
inline void handleFanController();
inline void handleLCDController();

void setup() {
    /** Initialize sensors and pins */
    sensor_temperature.begin();
    lcd_controller.begin();
    fan_controller.begin(0, 100);

    /** Setup connections */
    thing.add_wifi(SSID_NAME, SSID_PSK);
    OTAHandler.begin(false);

    /** Expose public states to cloud */
    thing["temperature_value"] >> [](pson &out) -> void {
        out = temperature_state.temperature_c;
    };
}

void loop() {
    /** Internet activities */
    OTAHandler.handle();
    thing.handle();

    /** Update millis counter */
    millis_counter = millis();

    /** Sensors, actuators, display */
    handleTemperatureSensor();
    handleFanController();
    handleLCDController();

    /** Timings */
    if (millis_counter - last_fetch_time > FETCH_TIME) {
        onFetch();

        last_fetch_time = millis_counter;
    }
}

inline void onFetch() {
    // lcd
    pson lcd_props;
    thing.get_property("lcd_state", lcd_props);
    lcd_state.backlight = lcd_props["backlight"];

    // fan
    pson fan_props;
    thing.get_property("fan_state", fan_props);
    fan_state.active              = fan_props["active"];
    fan_state.static_mode         = fan_props["static_mode"];
    fan_state.controlled_temp_max = fan_props["controlled_temp_max"];
    fan_state.controlled_temp_min = fan_props["controlled_temp_min"];
}

inline void handleTemperatureSensor() {
    sensor_temperature.requestTemperaturesByIndex(0);
    temperature_state.temperature_c = sensor_temperature.getTempCByIndex(0);
}

inline void handleFanController() {
    fan_controller.setFanActive(fan_state.active);
    fan_controller.setStaticMode(fan_state.static_mode);
    fan_controller.setMaxTemperature(fan_state.controlled_temp_max);
    fan_controller.setMinTemperature(fan_state.controlled_temp_min);

    fan_state.speed = fan_controller.getFanSpeed(temperature_state.temperature_c);
    analogWrite(PIN_FAN_INA, fan_state.speed);
    analogWrite(PIN_FAN_INB, 0);
}

inline void handleLCDController() {
    lcd_controller.setBlacklightOn(lcd_state.backlight);
    lcd_controller.update(temperature_state.temperature_c, fan_controller.getFanSpeedIndicator());
}
