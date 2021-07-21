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
    , _desired_temperature(0)
    , _desired_temperature_threshold(5) {
}

void FanController::begin(int8_t desired_temp_c, int8_t desired_temp_threshold_c = 5) {
    if (_is_initialized) {
        return;
    }

    setDesiredTemperature(desired_temp_c);
    setDesiredTemperatureThreshold(desired_temp_threshold_c);

    _is_initialized = true;
}

uint16_t FanController::getFanSpeed(float temperature) {
    if (!_is_initialized || !_is_fan_active) {
        _latest_fan_speed = FanSpeed::FAN_OFF;
    } else if (_is_static_mode) {
        _latest_fan_speed = FanSpeed::FAN_NORMAL;
    } else {
        _latest_fan_speed = measureFanSpeed(temperature);
    }

    return _latest_fan_speed;
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

int8_t FanController::getDesiredTemperature() {
    return _desired_temperature;
}

void FanController::setDesiredTemperature(int8_t desired_temp_c) {
    _desired_temperature = desired_temp_c;
}

int8_t FanController::getDesiredTemperatureThreshold() {
    return _desired_temperature_threshold;
}

void FanController::setDesiredTemperatureThreshold(int8_t desired_temp_threshold_c) {
    _desired_temperature_threshold = desired_temp_threshold_c;
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

uint16_t FanController::measureFanSpeed(float temperature) {
    float lower_threshold = _desired_temperature - _desired_temperature_threshold;
    float upper_threshold = _desired_temperature + _desired_temperature_threshold;

    if (temperature < lower_threshold) {
        return FanSpeed::FAN_OFF;
    } else if (temperature > upper_threshold) {
        return FanSpeed::FAN_HIGH;
    } else if (temperature < _desired_temperature && temperature > lower_threshold) {
        return FanSpeed::FAN_LOW;
    } else {
        return FanSpeed::FAN_NORMAL;
    }
}
