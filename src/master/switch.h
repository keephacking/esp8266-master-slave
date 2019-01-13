#ifndef Switch_H
#define Switch_H
#include <Arduino.h>
class Switch
{
private:
  int Id;

public:
  int Status;
  String Ip;
  String Type;
  String MacId;
  uint8_t Wsnum;
  static int Count;
  void registerDevice(String type, uint8_t wsnum, String ip,String macId);
  String toString();
  String toJson();
};

#endif