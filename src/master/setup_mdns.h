#ifndef SETUP_MDNS_H
#define SETUP_MDNS_H
#include <ESP8266mDNS.h>
#include "common.h"

class SETUP_MDNS
{
  public:
    static void initMDNS()
    {
        if (!MDNS.begin(MDNS_NAME))
        {
            Serial.println("Error setting up MDNS responder!");
            while (1)
            {
                delay(1000);
            }
        }
        Serial.print("mDNS responder started: http://");
        Serial.print(MDNS_NAME);
        Serial.println(".local");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", PORT);
        MDNS.addService("ws", "tcp", 81);
    }
};

#endif