#ifndef WS_H
#define WS_H
#include <WebSocketsServer.h>
#include "common.h"
#include "main.h"
class WS
{
  public:
    static void initWebSocket()
    {                                      // Start a WebSocket server
        webSocket.begin();                 // start the websocket server
        webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
        Serial.println("WebSocket server started.");
    }
    static void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
    { // When a WebSocket message is received
        IPAddress ip = webSocket.remoteIP(num);
        switch (type)
        {
        case WStype_DISCONNECTED: // if the websocket is disconnected
            Main::applyDeviceAction("unregister", num);
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
        { // if a new websocket connection is established
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            break;
        }
        break;
        case WStype_TEXT: // if new text data is received
            Serial.printf("[%u] get Text: %s\n", num, payload);
            Main::applyDeviceAction((char *)payload, num);
            break;
        }
    }
};
#endif