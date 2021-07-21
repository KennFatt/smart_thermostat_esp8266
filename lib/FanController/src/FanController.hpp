#ifndef KF_FANCONTROLLER_HPP
#define KF_FANCONTROLLER_HPP

#include <Arduino.h>

/**
 * Fan Controller
 *
 * High level library to handle DC Motor with L9110 module.
 *
 * Features:
 * 1. Adaptive fan speed that dependant with the temperature
 * 2. Static fan speed that _independant_ with the temperature
 * 3. Toggling the fan
 * 4. Adjustable controlled temperature
 */
class FanController {
    uint16_t _latest_fan_speed;

    bool _is_initialized;

    /** Fan speed will be a constant value */
    bool _is_static_mode;
    /** Togglable fan. True for active, otherwise off */
    bool _is_fan_active;

    int8_t _desired_temperature;
    int8_t _desired_temperature_threshold;

 public:
    /**
     * These values are used as `analogWrite(PIN, val)` value
     *
     * In my case, I'm using L9110 module to powered up a DC Motor.
     * the best value you should adjust based on my test:
     * Vcc 3.3V: LOW = 50%, NORMAL = 75%, HIGH = 100%
     * Vcc 5V: LOW = 25%, NORMAL = 50%, HIGH = 75%
     *
     * If you pushing to use 100% PWM Duty Cycle with Vcc 3.3V, it will
     * makes your board unresponsive (both your sketch and WiFi connections).
     */
    enum FanSpeed : uint16_t {
        FAN_OFF    = 0,
        FAN_LOW    = 512,
        FAN_NORMAL = 768,
        FAN_HIGH   = 1023
    };

    /**
     * FanController's constructor
     */
    FanController();

    /**
     * Copy constructor is not allowed
     */
    FanController(const FanController &) = delete;

    /**
     * Initialize the library and start basic configuration
     *
     * @param min_temp_c Minimum temperature in Celcius degree
     * @param max_temp_c Maximum temperature in Celcius degree
     */
    void begin(int8_t min_temp_c, int8_t max_temp_c);

    /**
     * Get respective fan's speed that respect the temperature value.
     *
     * @param temperature Value to let the system measure the best fan's speed
     *
     * @return uint16_t
     */
    uint16_t getFanSpeed(float temperature);

    /**
     * Get fan speed index:
     * 0 -> Low
     * 1 -> Normal
     * 2 -> High
     * default -> Off
     *
     * @return uint8_t
     */
    uint8_t getFanSpeedIndicator();

    /**
     * Check whether fan active or turned off
     *
     * @return bool
     */
    bool isFanActive();

    /**
     * Set the fan active mode
     *
     * @param is_active True to keep the fan active, otherwise will toggle it off
     */
    void setFanActive(bool is_active);

    bool isFanOnStaticMode();
    void setStaticMode(bool static_mode);

    int8_t getDesiredTemperature();
    void setDesiredTemperature(int8_t desired_temp_c);
    int8_t getDesiredTemperatureThreshold();
    void setDesiredTemperatureThreshold(int8_t desired_temp_threshold_c = 5);

 private:
    /**
     * Adjust the best fan's speed for a conditional temperature
     *
     * @param temperature The measurement parameter
     *
     * @return uint16_t
     */
    uint16_t measureFanSpeed(float temperature);
};

#endif    // KF_FANCONTROLLER_HPP
