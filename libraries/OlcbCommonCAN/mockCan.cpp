//
//  mockCan.cpp
//  
//
//  Created by David Harris on 2018-01-15.
//
//
#ifdef MockCAN_H

#pragma message("Compiling mockCan.cpp")

#include <stdio.h>
#include "OlcbCanClass.h"
#include "mockCan.h"

//OlcbCanClass(){};
// ~OlcbCanClass(){};
    //Can(){}
    //~Can(){}
void Can::init(){
        Serial.print("\nIn mockCan::init()");
}

uint8_t Can::avail() {
        Serial.print("\nIn mockCan::avail()");
}

uint8_t Can::read(){
        Serial.print("\nIn mockCan::init()");
}

uint8_t Can::txReady(){
        Serial.print("\nIn mockCan::read()");
}

uint8_t Can::write(){
        Serial.print("\nIn mockCan::write()");
}

#endif