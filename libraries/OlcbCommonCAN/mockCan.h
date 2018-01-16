//
//  mockCan.h
//  
//
//  Created by David Harris on 2018-01-15.
//
//

#ifndef CAN_H
#define CAN_H
#define MockCAN_H

#pragma message("Compiling mockCan.h")
#include "OlcbCanClass.h"
class Can : public OlcbCanClass {
  public:
    //OlcbCanClass(){};
    // ~OlcbCanClass(){};
    //Can();
    //~Can();
    void init();
    uint8_t avail();
    uint8_t read();
    uint8_t txReady();
    uint8_t write();
};


#endif
