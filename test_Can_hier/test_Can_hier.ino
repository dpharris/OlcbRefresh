#define P(x) Serial.print(x)
#define PH(x) if(x<16)P(0);Serial.print(x,HEX)
#define PL P("\n")
#include "menu.h"
class OlcbCanCLass {
  public: 
    union {
      uint32_t id;
      uint8_t id8[4];
    };
    union {
      uint64_t data64;
      uint8_t data[8];
    };
    uint8_t len;

    /*
    bool avail(){ Serial.print("\nIn OlcbCanCLass::avail()"); return true; }
    uint8_t read(){ Serial.print("\nIn OlcbCanCLass::read()"); return 0; }
    bool txAvail(){ Serial.print("\nIn OlcbCanCLass::txAvail()"); return true; }
    uint8_t write(){ Serial.print("\nIn OlcbCanCLass::write()"); return 0; }
    */
    
    virtual bool init();
    virtual bool avail();
    virtual uint8_t read();
    virtual bool txAvail();
    virtual uint8_t write();
  protected: 
    void print() {
      P("\n["); PH(id8[0]); PH(id8[1]); PH(id8[2]); PH(id8[3]); P("]");
      P("("); P(len); P(") "); PH(data[0]);
      for(int i=1;i<len;i++) { P(","); PH(data[i]); }
    }
};

class Can : public OlcbCanCLass {
  public: 
    Can(){}
    Can(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
      id8[0]=a; id8[1]=a; id8[2]=a; id8[3]=a; 
    }
    /*
    bool init() override { Serial.print("\nIn Can::init()"); return true; }
    bool avail() override { Serial.print("\nIn Can::avail()"); return true; }
    uint8_t read() override { Serial.print("\nIn Can::read()"); return 0; }
    bool txAvail() override { Serial.print("\nIn Can::txAvail()"); return true; }
    uint8_t write() override { Serial.print("\nIn Can::write()"); this->print(); return 0; }  
    */
    bool init()  { Serial.print("\nIn Can::init()"); return true; }
    bool avail()  { Serial.print("\nIn Can::avail()"); return true; }
    uint8_t read()  { Serial.print("\nIn Can::read()"); return 0; }
    bool txAvail()  { Serial.print("\nIn Can::txAvail()"); return true; }
    uint8_t write()  { Serial.print("\nIn Can::write()"); this->print(); return 0; }  

};

Can can;

uint64_t swap64(uint64_t num) {
  return ((num>>24)&0xff    ) |   // move byte 3 to byte 0
         ((num<<8) &0xff0000) |   // move byte 1 to byte 2
         ((num>>8) &0xff00  ) |   // move byte 2 to byte 1
         ((num<<24)&0xff000000);  // byte 0 to byte 3
}

void test(Can* can) {
  can->init();
  can->id = (uint64_t)0x00404F19;
  can->id += 0x123; // alias
  can->len = 8;
  can->data64 = 0x0807060504030201;
  if(can->txAvail()) {
    can->write();
  }
  if(can->avail()) {
    can->read();
  }
}


void setup() {
  while(!Serial){}
  Serial.begin(115200);
  P("\nTesting.");
  can.init();
  //can.id = 0x00404F19;
  can.id = swap64(0x19474000);
  can.id += 0x123; // alias
  can.len = 8;
  can.data64 = 0x0807060504030201;
  if(can.txAvail()) {
    can.write();
  }
  if(can.avail()) {
    can.read();
  }
  menu1();
}

void loop() {
  // put your main code here, to run repeatedly:
  menu_process();
}
