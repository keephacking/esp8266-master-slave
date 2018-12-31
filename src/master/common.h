#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
// #include <stdio.h>
// #include <Arduino.h>
#include "switch.h"
#define DBG_OUTPUT_PORT Serial
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50
#define PORT 80
#define SWITCH_LEN 8
#define MDNS_NAME "esp"
#define SSID "Asianet"
#define SS_PASSWORD "95679760"
#define AP_SSID "esp_switch" // ap_ssid of server (Access Point (AP))
#define AP_PASSWORD ""       // ap_password of server (Access Point (AP))

extern ESP8266WebServer server;
extern WebSocketsServer webSocket;
extern std::vector<Switch> devicesNew;
