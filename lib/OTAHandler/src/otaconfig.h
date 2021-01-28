#ifndef KF_OTACONFIG_H
#define KF_OTACONFIG_H

// WiFi SSID
#ifndef OTAH_SSID
#define OTAH_SSID ""
#endif
// WiFi passowrd
#ifndef OTAH_PSK
#define OTAH_PSK ""
#endif

// Restart the device after 10 seconds if auth failed
#define OTAH_TIMEOUT 10000UL

// ArduinoOTA configuration
// Device hostname
#ifndef OTAH_HOSTNAME
#define OTAH_HOSTNAME ""
#endif

// Device authentication (optional)
#ifndef OTAH_AUTH
#define OTAH_AUTH ""
#endif
// Authentication's md5 checksum (optional)
#ifndef OTAH_AUTH_HASH
#define OTAH_AUTH_HASH ""
#endif

// Upload port
#ifndef OTAH_PORT
#define OTAH_PORT 8266
#endif

// Reboot the device after OTA succeed
#ifndef OTAH_REBOOT
#define OTAH_REBOOT true
#endif

// Start OTA Service with mDNS
#ifndef OTAH_MDNS
#define OTAH_MDNS false
#endif

#endif    // KF_OTACONFIG_H
