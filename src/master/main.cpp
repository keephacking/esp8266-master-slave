#include "main.h";
#include "storage.h";
#include "common.h";
#include "controller.h";
#include <vector>
#include <ArduinoJson.h>
void Main::initRoutingConfig()
{
    server.on("/", HTTP_GET, []() {
        if (!STORAGE::handleFileRead("/index.html"))
            server.send(404, "text/plain", "FileNotFound");
    });
    // server.on("/", HTTP_GET, []() {
    //     server.send(200, "text/html",
    //                 "Welcome to the Master");
    // });
    server.on("/register", HTTP_POST, CONTROLLER::registerSwitch);
    server.on("/test", HTTP_POST, CONTROLLER::blink);
    server.onNotFound(CONTROLLER::handleNotFound);
    server.serveStatic("/", SPIFFS, "/", "max-age=86400");
}
String Main::createSwitchesJsonArray()
{
    String jArray = "[";
    for (int i = 0; i < devicesNew.size(); i++)
    {
        if (devicesNew[i].Type == "switch")
        {
            jArray += devicesNew[i].toJson() + ",";
        }
    }
    jArray = (jArray.length() > 1 ? jArray.substring(0, jArray.length() - 1) : jArray) + "]";
    return jArray;
}

void Main::deviceOn(String ip)
{
    for (int i = 0; i < devicesNew.size(); i++)
    {
        if (devicesNew[i].Type == "switch" && devicesNew[i].Ip == ip)
        {
            webSocket.sendTXT(devicesNew[i].Wsnum, "on");
        }
    }
}

void Main::deviceOff(String ip)
{
    for (int i = 0; i < devicesNew.size(); i++)
    {
        if (devicesNew[i].Type == "switch" && devicesNew[i].Ip == ip)
        {
            webSocket.sendTXT(devicesNew[i].Wsnum, "off");
        }
    }
}
void Main::toggleDevice(String ip)
{
    for (int i = 0; i < devicesNew.size(); i++)
    {
        if (devicesNew[i].Type == "switch" && devicesNew[i].Ip == ip)
        {
            webSocket.sendTXT(devicesNew[i].Wsnum, "toggle");
        }
    }
}

void Main::unregisterDevice(String ip)
{
    //delete from vector
    Serial.println("On Delete from list .");
    for (int i = 0; i < devicesNew.size(); i++)
    {
        if (devicesNew[i].Ip == ip)
        {
            Serial.printf(" Deleted : %s .", ip.c_str());
            devicesNew.erase(devicesNew.begin() + i);
        }
    }
}
void Main::registerDevice(String type, uint8_t num, String ip,String macId="")
{
    Serial.println("On Device Registration .");
    unregisterDevice(ip);
    Switch newDevice;
    newDevice.registerDevice(type, num, ip,macId);
    devicesNew.push_back(newDevice);
}
void Main::publishSwitchToBrowsers()
{
    Serial.println("On Publish to browsers .");
    for (int i = 0; i < devicesNew.size(); i++)
    {
        Serial.printf("i : [%d] ,num : %u ,Type: %s\n", (i + 1), devicesNew[i].Wsnum, devicesNew[i].Type.c_str());
        if (devicesNew[i].Type == "browser")
        {
            Serial.println(" Published to Browser");
            webSocket.sendTXT(devicesNew[i].Wsnum, createSwitchesJsonArray().c_str());
        }
    }
}
void Main::applyDeviceAction(String action, uint8_t num)
{
    Serial.println("On Websocket : " + action);
    IPAddress ip = webSocket.remoteIP(num);
    if (action == "unregister")
    {
        unregisterDevice(ip.toString());
    }
    else
    {
        StaticJsonBuffer<500> jsonBuffer;

        JsonObject &jsonBody = jsonBuffer.parseObject(action);

        if (!jsonBody.success())
        {
            Serial.println("error in parsin json body");
        }
        else
        {
            if (jsonBody["Action"] == "toggle")
                toggleDevice(jsonBody["Ip"]);
            else if (jsonBody["Action"] == "register_browser")
            {
                registerDevice("browser", num, ip.toString());
            }
            else if (jsonBody["Action"] == "register_switch")
            {
                registerDevice("switch", num, ip.toString(),jsonBody["MacId"]);
            }
        }
    }
    publishSwitchToBrowsers();
}
void Main::refreshDevices()
{
    Serial.println("On refresh devices");
    devicesNew.clear();
    webSocket.broadcastTXT("refresh");
}