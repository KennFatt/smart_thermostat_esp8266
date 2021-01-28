#include "OTAHandler.h"

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#include "otaconfig.h"

OTAHandlerClass::OTAHandlerClass()
    : _initialized(false) {
}

void OTAHandlerClass::begin(bool init_wifi) {
    if (init_wifi) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(OTAH_SSID, OTAH_PSK);

        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
            delay(OTAH_TIMEOUT);
            ESP.restart();
        }
    }

    ArduinoOTA.setHostname(OTAH_HOSTNAME);
    ArduinoOTA.setPassword(OTAH_AUTH);
    ArduinoOTA.setPasswordHash(OTAH_AUTH_HASH);
    ArduinoOTA.setPort(OTAH_PORT);
    ArduinoOTA.setRebootOnSuccess(OTAH_REBOOT);
    ArduinoOTA.begin(OTAH_MDNS);
    _initialized = true;
}

void OTAHandlerClass::handle() {
    if (_initialized) {
        ArduinoOTA.handle();
    }
}

OTAHandlerClass OTAHandler;
