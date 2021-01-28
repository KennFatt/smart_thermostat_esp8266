#ifndef KF_OTAHANDLER_H
#define KF_OTAHANDLER_H

/**
 * OTA Handler
 *
 * This class mean to be a hidden setup for OTA programming.
 * You do not put a huge boilerplated code to enable OTA programming with
 * your ESP8266 in main sketch file.
 *
 * inside your setup()
 *    OTAHandler.begin()
 *
 * inside your loop()
 *    OTAHandler.listen()
 */
class OTAHandlerClass {
 private:
    bool _initialized;

 public:
    OTAHandlerClass();

    /**
     * Initialize the connection configurations. In order of:
     * 1. WiFi (run as station mode)
     * 2. OTA Service
     *
     * You should open the .cpp file to change the configurations.
     */
    void begin(bool init_wifi = true);

    /**
     * Calling ArduinoOTA.handle() function after you begin the class.
     */
    void handle();
};

/** A high level OTA Service setup for ESP8266 */
extern OTAHandlerClass OTAHandler;

#endif    // KF_OTAHANDLER_H
