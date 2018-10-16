class Switch
{
private:
  int Id;

public:
  byte Status;
  String Ip;
  static int Count;

  Switch(){}

  void set(String ip, byte status)
  {
    Count++;
    Id = Count;
    Ip = ip;
    Status = status;
  }
};
