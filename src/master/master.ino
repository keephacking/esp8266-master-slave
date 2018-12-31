//"http://esp.local";
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "switch.h";
#include "common.h";
#include "main.h";
#include "setup_mdns.h";
#include "ws.h";
#include "setup_wifi.h";
#include "storage.h";
#include <Ticker.h>
int Switch::Count = 0;
ESP8266WebServer server(PORT);
WebSocketsServer webSocket(81);
std::vector<Switch> devicesNew;
Ticker ticker;

void setup()
{
    Serial.begin(115200); // Start the Serial communication to send messages to the computer
    delay(10);
    Serial.println("\r\n");

    WIFI_SETUP::initWiFi();

    STORAGE::initSPIFFS();

    WS::initWebSocket(); // Start a WebSocket server

    SETUP_MDNS::initMDNS();

    // Start a HTTP server with a file read handler and an upload handler
    Main::initRoutingConfig();

    server.begin(); // start the HTTP server
    Serial.println("HTTP server started.");

    ticker.attach(10, Main::refreshDevices);
}

int stationsCount = 0;
void loop()
{

    webSocket.loop();      // constantly check for websocket events
    server.handleClient(); // run the server

    //debug only
    if (stationsCount != WiFi.softAPgetStationNum())
    {
        stationsCount = WiFi.softAPgetStationNum();
        Serial.printf("Stations connected to soft-AP = %d\n", stationsCount);
    }
}
