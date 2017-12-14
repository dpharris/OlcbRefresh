#ifndef EventID_h
#define EventID_h

#include "stdint.h"
//#include "Event.h"
#include "Index.h"

//#include <Arduino.h>
class Event;
class EventsIndex;
class Index;

class EventID;
extern EventID blog(unsigned i);

class EventID {
  public: 
    uint8_t val[8];
    
    EventID();
    
    EventID(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
    
    //static int evCompare(void* a, void* b);
    
    uint16_t hash();
    
    bool equals(EventID* n);
    
    void print();
    
    void writeEID(int index);
    
    int findIndexInArray(Index* events, int len, int start);

    EventID getEID(uint16_t index);
    
};


#endif
