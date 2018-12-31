#include <ESP8266WiFi.h>
#include "common.h";

class WIFI_SETUP
{

public:
  static void initWiFi()
  {
    WiFi.mode(WIFI_STA);

    Serial.print("Connecting to WiFi AP..........");
    //Device as AP
    if (WiFi.softAP(AP_SSID, AP_PASSWORD))
    {
      // Display the server address
      Serial.print("Connected, AP Address: ");
      Serial.print("http://");
      Serial.print(WiFi.softAPIP());
      Serial.println("/");
    }
    else
      Serial.print("Not able to start AP.");

    //Connecting to WIFI
    int retries = 0;

    Serial.print("Connecting to WiFi ..........");
    Serial.println(SSID);

    WiFi.begin(SSID, SS_PASSWORD);
    // check the status of WiFi connection to be WL_CONNECTED
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY))
    {
      retries++;
      delay(WIFI_RETRY_DELAY);
      Serial.print("#");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("Connetted to ");
      Serial.print(SSID);
      Serial.print("--- IP: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.print("Error connecting to: ");
      Serial.println(SSID);
    }
  }
};
