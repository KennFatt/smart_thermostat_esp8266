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
static const uint8_t PIN_LDR         = A0;
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

struct LDRState {
    /** 1023 == brightest, 0 == darkest */
    uint16_t resistance = 0U;
} ldr_state;

struct LCDState {
    bool backlight = false;
} lcd_state;

struct FanState {
    uint16_t speed = FanController::FanSpeed::FAN_OFF;

    bool active                = false;
    bool static_mode           = false;
    int8_t controlled_temp_max = 100;
    int8_t controlled_temp_min = 0;
} fan_state;

/** --------------------------------------- Internal --------------------------------------- */
static bool initSynchronize = false;
void synchronizeFanProperties();
void synchronizeLCDProperties();

inline void updateTemperatureSensor();
inline void updateLDR();
inline void handleFanController();
inline void handleLCDController();

void setup() {
    /** Setup connections */
    thing.add_wifi(SSID_NAME, SSID_PSK);
    OTAHandler.begin(false);

    /** Initialize sensors and pins */
    sensor_temperature.begin();
    lcd_controller.begin();
    fan_controller.begin(fan_state.controlled_temp_min, fan_state.controlled_temp_max);

    /** Expose public states to cloud */
    thing["sensor_values"] >> [](pson &out) -> void {
        out["temperature_c"]  = temperature_state.temperature_c;
        out["ldr_resistance"] = ldr_state.resistance;
    };

    thing["sync"] = []() -> void {
        synchronizeFanProperties();
        synchronizeLCDProperties();
    };
}

void loop() {
    /** Internet activities */
    OTAHandler.handle();
    thing.handle();

    /** Sensors, actuators, display */
    updateTemperatureSensor();
    updateLDR();
    handleFanController();
    handleLCDController();

    if (!initSynchronize) {
        synchronizeFanProperties();
        synchronizeLCDProperties();

        initSynchronize = true;
    }
}

void synchronizeFanProperties() {
    pson fan_props;
    thing.get_property("fan_state", fan_props);
    fan_state.active              = (bool) fan_props["active"];
    fan_state.static_mode         = (bool) fan_props["static_mode"];
    fan_state.controlled_temp_max = (int8_t) fan_props["controlled_temp_max"];
    fan_state.controlled_temp_min = (int8_t) fan_props["controlled_temp_min"];

    fan_controller.setFanActive(fan_state.active);
    fan_controller.setStaticMode(fan_state.static_mode);
    fan_controller.setMaxTemperature(fan_state.controlled_temp_max);
    fan_controller.setMinTemperature(fan_state.controlled_temp_min);
}

void synchronizeLCDProperties() {
    pson lcd_props;
    thing.get_property("lcd_state", lcd_props);
    lcd_state.backlight = (bool) lcd_props["backlight"];

    lcd_controller.setBlacklightOn(lcd_state.backlight);
    lcd_controller.update(temperature_state.temperature_c, fan_controller.getFanSpeedIndicator());
}

inline void updateTemperatureSensor() {
    sensor_temperature.requestTemperaturesByIndex(0);
    temperature_state.temperature_c = sensor_temperature.getTempCByIndex(0);
}

inline void updateLDR() {
    ldr_state.resistance = analogRead(PIN_LDR);
}

inline void handleFanController() {
    fan_state.speed = fan_controller.getFanSpeed(temperature_state.temperature_c);
    analogWrite(PIN_FAN_INA, fan_state.speed);
    analogWrite(PIN_FAN_INB, 0);
}

inline void handleLCDController() {
    lcd_controller.update(temperature_state.temperature_c, fan_controller.getFanSpeedIndicator());
}
