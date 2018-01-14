//
//  OlcbCan.h
//  Interface for CAN libraries
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OlcbCanClass_h
#define OlcbCanClass_h
#include <Arduino.h>
/*
typedef struct {
    union {
        uint32_t id;
        uint8_t  idb[4];
    };
    uint8_t length;
    uint8_t data[8];
    struct {
        int rtr : 1;			//!< Remote-Transmit-Request-Frame?
        int extended : 1;		//!< extended ID?
    } flags;
} Can_t;
*/

class OlcbCanClass {
  public:
    union {
        uint32_t id;
        uint8_t  idb[4];
    };
    uint8_t length;
    uint8_t data[8];
    struct {
        int rtr : 1;			//!< Remote-Transmit-Request-Frame?
        int extended : 1;		//!< extended ID?
    } flags;
    
     OlcbCanClass(){};
     ~OlcbCanClass(){};
    void init(){};
    uint8_t avail() {
        Serial.print("\nOlcbCanClass avail()");
    }
     uint8_t read();
     uint8_t txReady();
     uint8_t write();
};

#endif // OlcbCanClass_h
