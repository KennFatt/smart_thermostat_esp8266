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
static const uint8_t PIN_PIR         = D0;
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
    uint16_t resistance        = 0;
    uint16_t resistance_mapped = 0;

    uint8_t precentage = 0;
} ldr_state;

struct PIRState {
    bool has_living_object = false;

    /** Trigger thing.write_bucket for every 5s if `has_living_object` == true */
    const unsigned long UPDATE_INTERVAL = 5000UL;
    unsigned long update_last_ts        = 0UL;
    unsigned long update_curr_ts        = 0UL;
} pir_state;

struct LCDState {
    bool backlight = false;
} lcd_state;

struct FanState {
    uint16_t speed = FanController::FanSpeed::FAN_OFF;

    bool motor_active                       = false;
    bool motor_static_mode                  = false;
    bool motor_off_brightness               = false;
    uint8_t motor_off_brightness_precentage = 25;
    int8_t desired_temp_c                   = 28;
    int8_t desired_temp_threshold_c         = 5;
} fan_state;

/** --------------------------------------- Internal --------------------------------------- */
static bool initSynchronize = false;
void synchronizeFanProperties();
void synchronizeLCDProperties();

inline void updateTemperatureSensor();
inline void updateLDR();
inline void updatePIR();
inline void handleFanController();
inline void handleLCDController();

void setup() {
    /** Setup connections */
    thing.add_wifi(SSID_NAME, SSID_PSK);
    OTAHandler.begin(false);

    /** Initialize sensors and pins */
    sensor_temperature.begin();

    /** Part of PIR system */
    pinMode(PIN_PIR, INPUT);
    pinMode(BUILTIN_LED, OUTPUT);

    lcd_controller.begin();
    fan_controller.begin(fan_state.desired_temp_c, fan_state.desired_temp_threshold_c);

    /** Expose public states to cloud */
    thing["sensor_values"] >> [](pson &out) -> void {
        out["temperature_c"]  = temperature_state.temperature_c;
        out["ldr_resistance"] = ldr_state.resistance;
        out["ldr_precentage"] = ldr_state.precentage;
    };

    thing["pir_sensor_value"] >> [](pson &out) -> void {
        out["has_living_object"] = pir_state.has_living_object;
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
    updatePIR();
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
    fan_state.motor_active                    = (bool) fan_props["motor_active"];
    fan_state.motor_static_mode               = (bool) fan_props["motor_static_mode"];
    fan_state.motor_off_brightness            = (bool) fan_props["motor_off_brightness"];
    fan_state.motor_off_brightness_precentage = (uint8_t) fan_props["motor_off_brightness_precentage"];
    fan_state.desired_temp_c                  = (int8_t) fan_props["desired_temperature"];
    fan_state.desired_temp_threshold_c        = (int8_t) fan_props["desired_temperature_threshold"];

    fan_controller.setFanActive(fan_state.motor_active);
    fan_controller.setStaticMode(fan_state.motor_static_mode);
    fan_controller.setDesiredTemperature(fan_state.desired_temp_c);
    fan_controller.setDesiredTemperatureThreshold(fan_state.desired_temp_threshold_c);
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
    ldr_state.resistance        = analogRead(PIN_LDR);
    ldr_state.resistance_mapped = max<uint16_t>(0, min<uint16_t>(ldr_state.resistance, 1000));
    ldr_state.precentage        = static_cast<uint8_t>(ldr_state.resistance_mapped / 10);
}

inline void updatePIR() {
    pir_state.has_living_object = digitalRead(PIN_PIR) == HIGH;

    pir_state.update_curr_ts = millis();
    bool allow_to_update     = pir_state.update_curr_ts - pir_state.update_last_ts > pir_state.UPDATE_INTERVAL;

    if (pir_state.has_living_object) {
        digitalWrite(BUILTIN_LED, HIGH);

        if (allow_to_update) {
            pir_state.update_last_ts = pir_state.update_curr_ts;
            thing.write_bucket("smart_thermostat_pir", "pir_sensor_value");
        }
    } else {
        digitalWrite(BUILTIN_LED, LOW);
    }
}

inline void handleFanController() {
    if (fan_state.motor_off_brightness) {
        fan_state.motor_active = ldr_state.precentage <= fan_state.motor_off_brightness_precentage ? false : true;
        fan_controller.setFanActive(fan_state.motor_active);
    }

    fan_state.speed = fan_controller.getFanSpeed(temperature_state.temperature_c);
    analogWrite(PIN_FAN_INA, fan_state.speed);
    analogWrite(PIN_FAN_INB, 0);
}

inline void handleLCDController() {
    lcd_controller.update(temperature_state.temperature_c, fan_controller.getFanSpeedIndicator());
}
