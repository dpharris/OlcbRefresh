//
//  OlcbCan.h
//  Interface for CAN libraries
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OlcbCanClass_h
#define OlcbCanClass_h
//#pragma message("Compiling OlcbCanClass.h")
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
    
    // OlcbCanClass(){};
    // ~OlcbCanClass(){};
    virtual void init(){};
    virtual uint8_t avail();
    virtual  uint8_t read();
    virtual  uint8_t txReady();
    virtual  uint8_t write();
};

#endif // OlcbCanClass_h
