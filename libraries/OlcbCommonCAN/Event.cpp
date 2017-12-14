
#include "logging.h"
#include "Event.h"
//#include "EventID.h"
//extern EventID getEID(uint16_t i);


Event::Event() {
}

Event::Event(bool produce, bool consume) {
    if (produce) flags |= Event::CAN_PRODUCE_FLAG;
    if (consume) flags |= Event::CAN_CONSUME_FLAG;
}

//Event::Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7)
//  : EventID(b0, b1, b2, b3, b4, b5, b6, b7), flags(0){
//}
//Event::Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) flags(0){}
/*
bool Event::equals(Event* n) {
    EventID a = this->getEID();
    EventID b = n->getEID();
    return  a.equals(&b);  // just check ID for now
}
*/


/**
* Check to see if this object is equal
* to any in an array of Events
*/
/* original
Event* Event::findEventInArray(Event* array, int len) {
  for (int i = 0; i<len; i++) {
      if (equals(array+i)) return array+i;
  }
  return 0;
}
int Event::findIndexInArray(Event* array, int len, int start) {
    for (int i = start; i<len; i++) {
        if (equals(array+i)) return i;
    }
    return -1;
}
*/
void Event::print() {
    //Serial.print("offset:"); Serial.print(offset,HEX);
    Serial.print("flags:"); Serial.print(flags,HEX);
}


/*
Event* Event::findEventInArray(Event* events, int len) {
           //Serial.print("\nIn findEventInArray");
    this->print();
           //Serial.print("\nindex="); Serial.print(this->index);
    EventID a = this->getEID();
           //Serial.print("\nEventID a="); a.print();
    Event* e = bsearch(this, events, len, sizeof(events[0]), events[0].evCompare);
           //e->print();
    EventID b = e->getEID();
           //Serial.print(" e eid=");
           //b.print();
    if (a.equals(&b)) return e;
    return 0;
}

int findIndexInArray(Event* events, int len, int start) {
            //Serial.print("\nIn Event::findIndexInArray");
            //Serial.print("\nstart:"); Serial.print(start);
            //Serial.print("\nthis:"); this->print();
    int hh = this->hash;
            //Serial.print("\nhash:"); Serial.print(hh);
    Event* e = &events[start];
    if (start==0) {
        e = bsearch( (const void*)&hh, (const void*)events, len, sizeof(Event), e->hashCompare);
        if(!e) return -1;
            //Serial.print("\nSearch result:"); e->print();
        while((e-1)>=events && (e-1)->hash==hh) {e--;}
            //Serial.print("\nbackup:"); e->print();
    }
    if(hh!=e->hash) return -1;
            //Serial.print("\ne:"); e->print();
            //EventID eid = getEID(e->index);
            //Serial.print("\nfound:"); eid.print();
    if(this->equals(e)) return e->index;
    return -1;
}
*/




