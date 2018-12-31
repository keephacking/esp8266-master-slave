#include <stdio.h>
#include "controller.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "common.h";
#include "switch.h";

void CONTROLLER::registerSwitch()
{
    StaticJsonBuffer<500> jsonBuffer;
    String post_body = server.arg("plain");
    Serial.println(post_body);

    JsonObject &jsonBody = jsonBuffer.parseObject(server.arg("plain"));

    prepareHeader();

    if (!jsonBody.success())
    {
        Serial.println("error in parsin json body");
        server.send(400);
    }
    else
    {
        if (server.method() == HTTP_POST)
        {
            server.send(201);
        }
    }
}

void CONTROLLER::prepareHeader()
{
    // ... some code to prepare the response data...
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");
}

void CONTROLLER::handleNotFound()
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

void CONTROLLER::getDevices()
{
    char JSONmessageBuffer[500];
    StaticJsonBuffer<500> jsonBuffer;
    JsonArray &array = jsonBuffer.createArray();

    JsonObject &jsonObj = jsonBuffer.createObject();
    // jsonObj["Ip"] = slaveSwitch.Ip;

    array.prettyPrintTo(JSONmessageBuffer, sizeof(jsonObj));

    prepareHeader();
    server.send(200, "application/json", JSONmessageBuffer);
}

void CONTROLLER::onRequest(Switch data)
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

void CONTROLLER::blink()
{
    Serial.println("Inside Blink");
    // if (slaveSwitch.Status)
    // {
    //     //off here
    //     slaveSwitch.Status = 0;
    //     onRequest(slaveSwitch);
    // }
    // else
    // {
    //     //on here
    //     slaveSwitch.Status = 1;
    //     onRequest(slaveSwitch);
    // }
}
