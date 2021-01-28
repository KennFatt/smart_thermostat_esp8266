#include "LCDController.hpp"

LCDController::LCDController()
    : _lcd(0x27, 16, 2)
    , _initialized(false)
    , _is_backlight_on(true)
    , _latest_update(0)
    , _temperature_counter(0)
    , _fan_speed_counter(1 << 7) {
}

void LCDController::begin() {
    if (_initialized) {
        return;
    }

    // init lcd
    _lcd.init();
    _lcd.setBacklight(_is_backlight_on);
    _lcd.clear();

    // load and render custom chars for the initializations
    for (uint8_t i = 0; i < _custom_chars.sizes; ++i) {
        const char* chr = _custom_chars.char_bytes[i];
        uint8_t col     = _custom_chars.char_pos[i][0];
        uint8_t row     = _custom_chars.char_pos[i][1];

        _lcd.createChar(i, chr);
        _lcd.setCursor(col, row);
        _lcd.write(i);
    }

    _lcd.setCursor(12, 0);
    _lcd.print(F("c"));

    _initialized = true;
}

void LCDController::update(float temperature, uint8_t fan_speed) {
    unsigned long current_millis = millis();

    if (((_latest_update - current_millis) < SCREEN_UPDATE_TIME) || !_initialized) {
        return;
    }

    if (!_is_backlight_on) {
        _latest_update = current_millis;
        return;
    }

    _latest_update = current_millis;

    // update dynamic data with newest data
    loadTemperature(temperature);
    loadFanSpeed(fan_speed);
}

bool LCDController::isBacklightOn() {
    return _is_backlight_on;
}

void LCDController::setBlacklightOn(bool is_on) {
    _is_backlight_on = is_on;
    _lcd.setBacklight(is_on);
}

void LCDController::loadFanSpeed(uint8_t fan_speed) {
    if (fan_speed == _fan_speed_counter) {
        return;
    }
    _fan_speed_counter = fan_speed;

    // max 6 bytes
    for (uint8_t i = 7; i <= 13; ++i) {
        _lcd.setCursor(i, 1);
        _lcd.print(" ");
    }

    _lcd.setCursor(7, 1);
    switch (fan_speed) {
        case 0:
            _lcd.print(F("low"));
            break;
        case 1:
            _lcd.print(F("normal"));
            break;
        case 2:
            _lcd.print(F("high"));
            break;
        case 3:
        default:
            _lcd.print(F("off"));
            break;
    }
}

void LCDController::loadTemperature(float temperature) {
    int16_t casted_temp = static_cast<int16_t>(temperature);
    if (casted_temp == _temperature_counter) {
        return;
    }
    _temperature_counter = casted_temp;

    // max 4 bytes
    for (uint8_t i = 7; i <= 10; ++i) {
        _lcd.setCursor(i, 0);
        _lcd.print(" ");
    }

    const char* fmt = casted_temp < 0 ? "-%d" : " %d";

    _lcd.setCursor(7, 0);
    _lcd.printf(fmt, abs(casted_temp));
}
