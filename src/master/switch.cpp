#include "switch.h"
#include <Arduino.h>

void Switch::registerDevice(String type, uint8_t wsnum, String ip,String macId)
{
    Count++;
    Id = Count;
    Type = type;
    Wsnum = wsnum;
    Ip = ip;
    MacId = macId;
}

String Switch::toString()
{
    return String("\"{'Id':") + Id + ",'Ip':'" + Ip + "','Status':" + Status + "}\"";
}

String Switch::toJson()
{
    return String("{\"Id\":\"") + Id + "\",\"Type\":\"" + Type + "\",\"Ip\":\"" + Ip + "\",\"Status\":" + Status + "}";
}
