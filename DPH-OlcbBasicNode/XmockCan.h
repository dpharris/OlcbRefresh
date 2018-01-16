#ifndef CAN_H
#define CAN_H
#include "OlcbCanClass.h"
class Can : public OlcbCanClass {
  public:
    //OlcbCanClass(){};
    // ~OlcbCanClass(){};
    void init(){ 
      Serial.print("\nIn mockCan::init()"); 
    }
    uint8_t avail() {
      Serial.print("\nIn mockCan::avail()"); 
    }
    uint8_t read(){ 
      Serial.print("\nIn mockCan::init()"); 
    }
    uint8_t txReady(){
      Serial.print("\nIn mockCan::read()"); 
    }
    uint8_t write(){ 
      Serial.print("\nIn mockCan::write()"); 
    }
};



#endif
