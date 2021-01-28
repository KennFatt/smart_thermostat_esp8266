#ifndef KF_LCDCONTROLLER_HPP
#define KF_LCDCONTROLLER_HPP

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

/**
 * Struct CustomCharacters
 *
 * it holds custom character that used on the display screen.
 */
struct CustomCharacters {
    const uint8_t sizes = 7;
    const char char_bytes[7][8] = {
       {0x01, 0x02, 0x1E, 0x02, 0x0E, 0x02, 0x06, 0x02},
       {0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
       {0x02, 0x03, 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x07},
       {0x08, 0x18, 0x1C, 0x1E, 0x1E, 0x1E, 0x1E, 0x1C},
       {0x04, 0x0A, 0x0A, 0x0A, 0x0E, 0x1F, 0x1F, 0x0E},    // temp
       {0x07, 0x05, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00},    // deg
       {0x00, 0x0C, 0x05, 0x17, 0x1C, 0x04, 0x06, 0x00},    // fan
    };
    const uint8_t char_pos[7][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}, {5, 0}, {11, 0}, {5, 1}};
};

/**
 * LCD Controller
 *
 * High level library to control LCD 16x2 (address 0x27) with I2C communication.
 * It is designed and built for my own project.
 *
 * Features:
 * 1. Toggle on and off
 * 2. Dynamic data (temperature and fan speed)
 */
class LCDController {
 private:
    /** Struct that holds our custom characters map */
    CustomCharacters _custom_chars;

    LiquidCrystal_I2C _lcd;
    bool _initialized;

    /** Turn on/off the display screen */
    bool _is_backlight_on;

    /**
     * LCD will update its screen periodically,
     *  I'm using the power of timing with `millis()` tp handle it.
     *
     * Communication with i2c and updating the LCD oftenly is really expensive!
     */
    const unsigned long SCREEN_UPDATE_TIME = 5000UL;
    unsigned long _latest_update;

    /** Some local cache */
    int16_t _temperature_counter;
    uint8_t _fan_speed_counter;

 public:
    /**
     * LCD Controller is a high level library to control
     * the LCD (0x27) with screen size 16x2 usable segments.
     *
     * It also integrated with Fan Controller library and temperature.
     */
    LCDController();

    /** Copy constructor is not allowed here */
    LCDController(const LCDController &) = delete;

    /** Initiate the library */
    void begin();

    /**
     * Update the screen with newest parameters.
     *
     * The LCD will not immediately update and re-render the screen,
     * though it will wait till it hit its timing.
     *
     * @param temperature Temperature real-time sensor's value
     * @param fan_speed Fan speed indicator (0, 1, 2, 3)
     */
    void update(float temperature, uint8_t fan_speed);

    /**
     * Check whether the LCD screen is turned on or off
     *
     * @return bool
     */
    bool isBacklightOn();

    /**
     * Toggle the LCD screen
     *
     * @param on True to keep it on, otherwise off
     */
    void setBlacklightOn(bool on);

 private:
    /** Re-render the fan's speed data */
    void loadFanSpeed(uint8_t fan_speed);
    /** Re-render the temperature sensor data */
    void loadTemperature(float temperature);
};

#endif // KF_LCDCONTROLLER_HPP
