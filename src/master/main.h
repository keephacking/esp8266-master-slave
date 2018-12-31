#include "common.h"
#ifndef MAIN_H
#define MAIN_H
class Main
{
  public:
    static void initRoutingConfig();
    static void applyDeviceAction(String action, uint8_t num);
    static void refreshDevices();
    static void deviceOn(String ip);
    static void deviceOff(String ip);
    static void toggleDevice(String ip);
    
  private:
    static String createSwitchesJsonArray();
    static void unregisterDevice(String ip);
    static void registerDevice(String type, uint8_t num,String ip);
    static void publishSwitchToBrowsers();
};
#endif
