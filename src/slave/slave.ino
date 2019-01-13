#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>
#include <mDNSResolver.h> //https://github.com/madpilot/mDNSResolver

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

#define USE_SERIAL Serial

const char *ssid = "esp_switch";
const char *password = "";

//Web/Server address to read/write from
const char *host = "esp.local";

WiFiUDP udp;
mDNSResolver::Resolver resolver(udp);

// ESP8266WebServer server(HTTP_REST_PORT);
WebSocketsClient webSocket;
//switch details
struct Switch
{
    String Ip;
    byte Pin;
    byte Status;
    String MacId;
} device;

void init_switch(String ip)
{
    device.Ip = ip;
    device.Pin = 5;
    device.Status = 0;
    device.MacId = WiFi.macAddress();
    pinMode(device.Pin, OUTPUT);
}
// void register_switch()
// {
//     HTTPClient http; //Declare object of class HTTPClient

//       char JSONmessageBuffer[500];
//     StaticJsonBuffer<500> jsonBuffer;
//     JsonArray &array = jsonBuffer.createArray();

//     JsonObject &jsonObj = jsonBuffer.createObject();
//     jsonObj["Status"] = device.Status;
//     jsonObj["Ip"] = device.Ip;

//     jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
//     // Serial.println( jsonObj["Ip"].toString());
//     Serial.println(device.Ip);
//     Serial.println(JSONmessageBuffer);
//     IPAddress ip = resolver.search(host);
//     Serial.println(ip.toString());
//     http.begin("http://" + ip.toString() + "/register"); //Specify request destination
//     http.addHeader("Content-Type", "application/json");  //Specify content-type header

//     int httpCode = http.POST(JSONmessageBuffer); //Send the request
//     String payload = http.getString();  //Get the response payload

//     Serial.println(httpCode); //Print HTTP return code
//     Serial.println(payload);  //Print request response payload

//     http.end(); //Close connection
// }
// void device_update()
// {

//     StaticJsonBuffer<500> jsonBuffer;
//     String post_body = server.arg("plain");
//     Serial.println(post_body);

//     JsonObject &jsonBody = jsonBuffer.parseObject(server.arg("plain"));

//     _prepareHeader();

//     if (!jsonBody.success())
//     {
//         Serial.println("error in parsin json body");
//         server.send(400);
//     }
//     else
//     {
//         if (server.method() == HTTP_POST)
//         {

//             Serial.print("POST Request: INITIAL STATUS:");

//             Serial.println(device.Status);
//             Serial.print("POST Request: SEND STATUS ");
//             byte data = jsonBody["Status"];
//             Serial.println(data);

//             if (jsonBody["Status"] == 1)
//             {
//                 Serial.println("LED ON COMMAND");
//                 digitalWrite(device.Pin, 1);
//             }
//             else
//             {
//                 Serial.println("LED OFF COMMAND");
//                 digitalWrite(device.Pin, 0);
//             }

//             server.send(201);
//         }
//     }
// }
void deviceOn()
{
    device.Status = 1;
    digitalWrite(device.Pin, 1);
}
void deviceOff()
{
    device.Status = 0;
    digitalWrite(device.Pin, 0);
}
void toggle()
{
    if (device.Status == 1)
    {
        deviceOff();
        Serial.println("Device Off");
    }
    else
    {
        deviceOn();
        Serial.println("Device On");
    }
}
int init_wifi()
{
    int retries = 0;

    Serial.println("Connecting to WiFi AP..........");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // check the status of WiFi connection to be WL_CONNECTED
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY))
    {
        retries++;
        delay(WIFI_RETRY_DELAY);
        Serial.print("#");
    }
    return WiFi.status(); // return the WiFi connection status
}
void registerSwitch()
{
    String message=String("{\"Action\":\"register_switch\"")+",\"MacId\":\""+device.MacId+"\"}";
    webSocket.sendTXT(message);
}
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        USE_SERIAL.printf("[WSc] Disconnected!\n");
        break;
    case WStype_CONNECTED:
    {
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
        // send message to server when Connected
        registerSwitch();
    }
    break;
    case WStype_TEXT:
        USE_SERIAL.printf("[WSc] get text: %s\n", payload);

        // send message to server
        // webSocket.sendTXT("message here");
        if (String((char *)payload) == "refresh")
            registerSwitch();
        else if (String((char *)payload) == "on")
            deviceOn();
        else if (String((char *)payload) == "off")
            deviceOff();
        else if (String((char *)payload) == "toggle")
            toggle();
        break;
    case WStype_BIN:
        USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
        hexdump(payload, length);

        // send data to server
        // webSocket.sendBIN(payload, length);
        break;
    }
}
void setup()
{
    Serial.begin(115200);
    if (init_wifi() == WL_CONNECTED)
    {
        Serial.print("Connetted to ");
        Serial.print(ssid);
        Serial.print("--- IP: ");
        Serial.println(WiFi.localIP());
        init_switch(WiFi.localIP().toString());
        // register_switch();
    }
    else
    {
        Serial.print("Error connecting to: ");
        Serial.println(ssid);
    }
    // config_rest_server_routing();
    // server.begin();
    // Serial.println("HTTP REST Server Started");

    // server address, port and URL
    IPAddress ip = resolver.search(host);
    Serial.println(ip.toString());
    webSocket.begin(ip.toString(), 81, "/");

    // event handler
    webSocket.onEvent(webSocketEvent);

    // use HTTP Basic Authorization this is optional remove if not needed
    // webSocket.setAuthorization("user", "Password");

    // try ever 5000 again if connection has failed
    webSocket.setReconnectInterval(5000);
}

void loop()
{
    // server.handleClient();
    webSocket.loop();
}

// void _prepareHeader()
// {
//     // ... some code to prepare the response data...
//     server.sendHeader("Access-Control-Allow-Origin", "*");
//     server.sendHeader("Access-Control-Max-Age", "10000");
//     server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
//     server.sendHeader("Access-Control-Allow-Headers", "*");
// }
// void config_rest_server_routing()
// {
//     server.on("/", HTTP_GET, []() {
//         server.send(200, "text/html",
//                     "Welcome to Slave");
//     });
//     server.on("/update", HTTP_POST, device_update);
//     server.onNotFound(handleNotFound);
// }
// void handleNotFound()
// {
//     if (server.method() == HTTP_OPTIONS)
//     {
//         server.sendHeader("Access-Control-Allow-Origin", "*");
//         server.sendHeader("Access-Control-Max-Age", "10000");
//         server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
//         server.sendHeader("Access-Control-Allow-Headers", "*");
//         server.send(204);
//     }
//     else
//     {
//         String message = "File Not Found\n\n";
//         message += "URI: ";
//         message += server.uri();
//         message += "\nMethod: ";
//         message += (server.method() == HTTP_GET) ? "GET" : "POST";
//         message += "\nArguments: ";
//         message += server.args();
//         message += "\n";

//         for (uint8_t i = 0; i < server.args(); i++)
//         {
//             message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
//         }

//         server.send(404, "text/plain", message);
//     }
// }