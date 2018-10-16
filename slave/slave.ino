#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <mDNSResolver.h>

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

const char *ssid = "esp_switch";
const char *password = "";

//Web/Server address to read/write from
const char *host = "esp.local";

WiFiUDP udp;
mDNSResolver::Resolver resolver(udp);

ESP8266WebServer server(HTTP_REST_PORT);

//switch details
struct Switch
{
    String Ip;
    byte Pin;
    byte Status;
} device;

void init_switch(String ip)
{
    device.Ip = ip;
    device.Pin = 5;
    device.Status = 0;
    pinMode(device.Pin, OUTPUT);
}
void register_switch()
{
    HTTPClient http; //Declare object of class HTTPClient

      char JSONmessageBuffer[500];
    StaticJsonBuffer<500> jsonBuffer;
    JsonArray &array = jsonBuffer.createArray();

    JsonObject &jsonObj = jsonBuffer.createObject();
    jsonObj["Status"] = device.Status;
    jsonObj["Ip"] = device.Ip;

    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    // Serial.println( jsonObj["Ip"].toString());
    Serial.println(device.Ip);
    Serial.println(JSONmessageBuffer);
    IPAddress ip = resolver.search(host);
    Serial.println(ip.toString());
    http.begin("http://" + ip.toString() + "/register"); //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();  //Get the response payload

    Serial.println(httpCode); //Print HTTP return code
    Serial.println(payload);  //Print request response payload

    http.end(); //Close connection
}
void device_update()
{

    StaticJsonBuffer<500> jsonBuffer;
    String post_body = server.arg("plain");
    Serial.println(post_body);

    JsonObject &jsonBody = jsonBuffer.parseObject(server.arg("plain"));

    _prepareHeader();

    if (!jsonBody.success())
    {
        Serial.println("error in parsin json body");
        server.send(400);
    }
    else
    {
        if (server.method() == HTTP_POST)
        {

            Serial.print("POST Request: INITIAL STATUS:");

            Serial.println(device.Status);
            Serial.print("POST Request: SEND STATUS ");
            byte data = jsonBody["Status"];
            Serial.println(data);

            if (jsonBody["Status"] == 1)
            {
                Serial.println("LED ON COMMAND");
                digitalWrite(device.Pin, 1);
            }
            else
            {
                Serial.println("LED OFF COMMAND");
                digitalWrite(device.Pin, 0);
            }

            server.send(201);
        }
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
        register_switch();
    }
    else
    {
        Serial.print("Error connecting to: ");
        Serial.println(ssid);
    }
    config_rest_server_routing();
    server.begin();
    Serial.println("HTTP REST Server Started");
}

void loop()
{
    server.handleClient();
}

void _prepareHeader()
{
    // ... some code to prepare the response data...
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
}
void config_rest_server_routing()
{
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html",
                    "Welcome to Slave");
    });
    server.on("/update", HTTP_POST, device_update);
    server.onNotFound(handleNotFound);
}
void handleNotFound()
{
    if (server.method() == HTTP_OPTIONS)
    {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Max-Age", "10000");
        server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "*");
        server.send(204);
    }
    else
    {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";

        for (uint8_t i = 0; i < server.args(); i++)
        {
            message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
        }

        server.send(404, "text/plain", message);
    }
}