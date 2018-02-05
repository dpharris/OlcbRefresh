#include "Event.h"
#include "lib_debug_print_common.h"

Event::Event() {
}

Event::Event(bool produce, bool consume) {
    if (produce) flags |= Event::CAN_PRODUCE_FLAG;
    if (consume) flags |= Event::CAN_CONSUME_FLAG;
}

bool Event::isConsumer() {
    return flags&Event::CAN_CONSUME_FLAG;
}

bool Event::isProducer() {
    return flags&Event::CAN_PRODUCE_FLAG;
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
    //LDEBUG("offset:"); LDEBUG2(offset,HEX);
    //LDEBUG("flags:"); LDEBUG2(flags,HEX);
    
}


/*
Event* Event::findEventInArray(Event* events, int len) {
           //LDEBUG("\nIn findEventInArray");
    this->print();
           //LDEBUG("\nindex="); LDEBUG(this->index);
    EventID a = this->getEID();
           //LDEBUG("\nEventID a="); a.print();
    Event* e = bsearch(this, events, len, sizeof(events[0]), events[0].evCompare);
           //e->print();
    EventID b = e->getEID();
           //LDEBUG(" e eid=");
           //b.print();
    if (a.equals(&b)) return e;
    return 0;
}

int findIndexInArray(Event* events, int len, int start) {
            //LDEBUG("\nIn Event::findIndexInArray");
            //LDEBUG("\nstart:"); LDEBUG(start);
            //LDEBUG("\nthis:"); this->print();
    int hh = this->hash;
            //LDEBUG("\nhash:"); LDEBUG(hh);
    Event* e = &events[start];
    if (start==0) {
        e = bsearch( (const void*)&hh, (const void*)events, len, sizeof(Event), e->hashCompare);
        if(!e) return -1;
            //LDEBUG("\nSearch result:"); e->print();
        while((e-1)>=events && (e-1)->hash==hh) {e--;}
            //LDEBUG("\nbackup:"); e->print();
    }
    if(hh!=e->hash) return -1;
            //LDEBUG("\ne:"); e->print();
            //EventID eid = getEID(e->index);
            //LDEBUG("\nfound:"); eid.print();
    if(this->equals(e)) return e->index;
    return -1;
}
*/




