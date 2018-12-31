#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "switch.h"
class CONTROLLER
{
public:
  static void registerSwitch();
  static void blink();
  static void onRequest(Switch data);
  static void handleNotFound();
  static void getDevices();
private:
  static void prepareHeader();
};

#endif