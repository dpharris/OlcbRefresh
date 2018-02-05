class Inface : public Print {
  bool begin();
  bool rxAvail();
  void read(int* handle, int* command, int* len, uint8_t* data);
  void read(void *msg, int len)
  bool txAvail();
  void write(int* handle, int* command, int* len, uint8_t* data);
  void write(void *msg, int len)
  void end();
};
class SerialInf : public Inface {
  void begin() { Serial.begin(1152000); }
  bool rxAvail() { return Serial.available(); }
  void read(int* handle, int* command, int* len, uint8_t* data) {
    if(read()!="(") return;
    handle = readTo(',');
    command = readTo(',');
    int i=0;
    while( !peek(')') ) {
      data[i] = readTo(',');
    }
  }
  void write(int* handle, int* command, int* len, uint8_t* data) {
    P("("); PH(*handle); P(","); P(command,HEX); 
    for(int i=0;i<len;i++) P(data[i],HEX);
  }
}
class LocoClass {
  Inface* inf;
  String name;
  float speed;
  uint8_t f[28];
  LocoClass(Inface* i) {inf=i;}
};
class LocoMsg {
  int handle;
  int command;
  int data;
};

class DCCLoco : public LocoClass {
  int steps;
  int dccAddr;
  bool forward;
  DCCLoco(Inface i, int dcc) { inf=i; dccAddr=d; steps = 128; }
  DCCLoco(Inface i, int dcc, int s) { inf=i; dccAddr=dcc; steps = s; }
  void setSpeed(float s) {
    spd = s;
    int dccspd;
    if(s<0) {
      forward = false;
      dccspd = -steps * s +1;
    }
    if(s>=0) {
      forward = true;
      dccspd = steps *s +1;
    }
    int d[2];
    d[0] = forward;
    d[1] = dccspd;
    Msg m = Msg(handle, SETSPEED, 2, d);
    inf->send(m);
  }
};



void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
