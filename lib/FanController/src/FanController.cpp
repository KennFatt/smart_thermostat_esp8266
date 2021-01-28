#include "FanController.hpp"

template<typename T>
inline constexpr T min_generic(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
inline constexpr T max_generic(T a, T b) {
    return a > b ? a : b;
}

FanController::FanController()
    : _latest_fan_speed(FanSpeed::FAN_OFF)
    , _is_initialized(false)
    , _is_static_mode(false)
    , _is_fan_active(false)
    , _min_temperature(0)
    , _max_temperature(0)
    , _range_temperature(0) {
}

void FanController::begin(int8_t min_temp_c, int8_t max_temp_c) {
    if (_is_initialized) {
        return;
    }

    setMinTemperature(min_generic(min_temp_c, max_temp_c));
    setMaxTemperature(max_generic(min_temp_c, max_temp_c));
    updateTemperatureRange(0);

    _is_static_mode = _min_temperature == _max_temperature;
    _is_initialized = true;
}

uint16_t FanController::getFanSpeed(float temperature) {
    if (!_is_initialized || !_is_fan_active) {
        _latest_fan_speed = FanSpeed::FAN_OFF;
    } else if (_is_static_mode) {
        _latest_fan_speed = FanSpeed::FAN_NORMAL;
    } else {
        _latest_fan_speed = measureFanSpeed(static_cast<int8_t>(temperature));
    }

    return _latest_fan_speed;
}

uint16_t FanController::measureFanSpeed(int8_t temperature) {
    uint16_t diff          = abs(_max_temperature) - abs(_min_temperature);
    float range_precentage = _range_temperature / 100.0F;
    float buffer           = diff * range_precentage;
    uint16_t l_length      = _min_temperature + buffer;
    uint16_t h_length      = _max_temperature - buffer;

    if (temperature < _min_temperature) {
        return FanSpeed::FAN_OFF;
    } else if (temperature >= _min_temperature && temperature <= l_length) {
        return FanSpeed::FAN_LOW;
    } else if (temperature > l_length && temperature < h_length) {
        return FanSpeed::FAN_NORMAL;
    } else {
        return FanSpeed::FAN_HIGH;
    }
}

int8_t FanController::getMinTemperature() {
    return _min_temperature;
}

int8_t FanController::getMaxTemperature() {
    return _max_temperature;
}

uint8_t FanController::getTemperatureRangePrecentage() {
    return _range_temperature;
}

void FanController::setMinTemperature(int8_t min_temp_c) {
    _min_temperature = min_temp_c;
}

void FanController::setMaxTemperature(int8_t max_temp_c) {
    _max_temperature = max_temp_c;
}

void FanController::updateTemperatureRange(uint8_t range_value) {
    _range_temperature = range_value > 20 ? 45 : (range_value + 25);
}

bool FanController::isFanActive() {
    return _is_fan_active;
}

void FanController::setFanActive(bool is_active) {
    // force to change the latest fan speed state here
    // then the lcd would recognize if the fan is currently off.
    _latest_fan_speed = FanSpeed::FAN_OFF;

    _is_fan_active = is_active;
}

bool FanController::isFanOnStaticMode() {
    return _is_static_mode;
}

void FanController::setStaticMode(bool static_mode) {
    _is_static_mode = static_mode;
}

uint8_t FanController::getFanSpeedIndicator() {
    switch (_latest_fan_speed) {
        case FanSpeed::FAN_LOW:
            return 0;
        case FanSpeed::FAN_NORMAL:
            return 1;
        case FanSpeed::FAN_HIGH:
            return 2;
        default:
            return 3;
    }
}
