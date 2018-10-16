
/*------------------------------------------------------------------------------
  12/28/2016
  Author: Makerbro
  Platforms: ESP8266
  Language: C++/Arduino
  File: webserver_mdns.ino
  ------------------------------------------------------------------------------
  Description: 
  Code for YouTube video demonstrating how to use mDNS.
  https://youtu.be/ewrF1LzrxH8 
  ------------------------------------------------------------------------------
  Please consider buying products from ACROBOTIC to help fund future
  Open-Source projects like this! We'll always put our best effort in every
  project, and release all our design files and code for you to use. 
  https://acrobotic.com/
  ------------------------------------------------------------------------------
  License:
  Please see attached LICENSE.txt file for details.
------------------------------------------------------------------------------*/
//"http://esp.local";
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "switch.h";

int Switch::Count = 0;
Switch slaveSwitch;
#define PORT 80

const char *ap_ssid = "esp_switch"; // ap_ssid of server (Access Point (AP))
const char *ap_password = "";       // ap_password of server (Access Point (AP))
ESP8266WebServer server(PORT);
MDNSResponder mdns;
bool startBlink = false;
int init_wifi_ap()
{

    Serial.println("Connecting to WiFi AP..........");

    WiFi.mode(WIFI_AP); // Set WiFi to AP and station mode
    if (WiFi.softAP(ap_ssid, ap_password))
    {
        // Display the server address
        Serial.print("Connected, My address: ");
        Serial.print("http://");
        Serial.print(WiFi.softAPIP());
        Serial.println("/");
    }
    else
        Serial.print("Not able to start AP.");
}

void register_switch()
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
            // Switch newSwitch(jsonBody["Ip"], jsonBody["Status"]);
            // slaveSwitch.Ip =  (String)jsonBody["Ip"];;
            slaveSwitch.Ip = jsonBody["Ip"].asString();
            slaveSwitch.Status = jsonBody["Status"];
            server.send(201);
            delay(3000);
            startBlink=true;
        }

        // if (exist == 0)
        // {
        //     server.send(404);
        // }
    }
}
void blink()
{
    Serial.println("Inside Blink");
    if (slaveSwitch.Status)
    {
        //off here
        slaveSwitch.Status = 0;
        onRequest(slaveSwitch);
    }
    else
    {
        //on here
        slaveSwitch.Status = 1;
        onRequest(slaveSwitch);
    }
}

void onRequest(Switch data)
{
    Serial.println("On Request");
    HTTPClient http; //Declare object of class HTTPClient
    Serial.println(data.Ip);
    http.begin("http://" + data.Ip + "/update");        //Specify request destination
    http.addHeader("Content-Type", "application/json"); //Specify content-type header

    char JSONmessageBuffer[500];
    StaticJsonBuffer<500> jsonBuffer;

    JsonObject &jsonObj = jsonBuffer.createObject();
    jsonObj["Status"] = data.Status;
    jsonObj["Ip"] = data.Ip;
    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

    Serial.println(JSONmessageBuffer);

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload

    Serial.println(httpCode); //Print HTTP return code
    Serial.println(payload);  //Print request response payload

    http.end(); //Close connection
}

void get_devices()
{
    char JSONmessageBuffer[500];
    StaticJsonBuffer<500> jsonBuffer;
    JsonArray &array = jsonBuffer.createArray();

    JsonObject &jsonObj = jsonBuffer.createObject();
    jsonObj["Ip"] = slaveSwitch.Ip;

    array.prettyPrintTo(JSONmessageBuffer, sizeof(jsonObj));

    _prepareHeader();
    server.send(200, "application/json", JSONmessageBuffer);
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
                    "Welcome to the Master");
    });
    server.on("/register", HTTP_POST, register_switch);
    server.on("/test", HTTP_POST, blink);
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

void setup()
{
    Serial.begin(115200);

    init_wifi_ap();

    config_rest_server_routing();

    if (mdns.begin("esp", WiFi.softAPIP()))
        Serial.println("MDNS responder started");

    server.begin();
    Serial.println("HTTP REST Server Started");

    mdns.addService("http", "tcp", 80);
}

int stationsCount = 0;
void loop()
{
    server.handleClient();

    if (startBlink)
    {
        blink();
        delay(2000);
    }
    //debug only
    if (stationsCount != WiFi.softAPgetStationNum())
    {
        stationsCount = WiFi.softAPgetStationNum();
        Serial.printf("Stations connected to soft-AP = %d\n", stationsCount);
    }
}
