#include <Arduino.h>
#include <FanController.hpp>

static const int DRIVER_INA = D5;
static const int DRIVER_INB = D6;

static const float temperature = 26.04f;

FanController fan_controller;

// this value is accessible AND mutable by the internet
bool is_fan_active = true;

void setup() {
    fan_controller.begin(20, 38);

    // fetching the data from internet ...
}

void loop() {
    // ... listen for incoming data from the internet

    // in any moment `is_fan_active` can be toggled on or off by the user over the internet.
    // best approach to use a scheduler, but not that realtime

    // the function will not modify any memory if the value are equals the previous one
    fan_controller.setFanActive(is_fan_active);
    analogWrite(DRIVER_INA, fan_controller.getFanSpeed(temperature));
    analogWrite(DRIVER_INB, 0);
}
