//
//  EventID.cpp
//  
//
//  Created by David Harris on 2017-11-19.
//
//

#include "EventID.h"
#include "Event.h"
#include "Index.h"
#include <Arduino.h>
#include "lib_debug_print_common.h"


EventID::EventID() {
    val[0] = 0;
    val[1] = 0;
    val[2] = 0;
    val[3] = 0;
    val[4] = 0;
    val[5] = 0;
    val[6] = 0;
    val[7] = 0;
}

EventID::EventID(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
    val[0] = b0;
    val[1] = b1;
    val[2] = b2;
    val[3] = b3;
    val[4] = b4;
    val[5] = b5;
    val[6] = b6;
    val[7] = b7;
}

//static int EventID::evCompare(void* a, void* b);

uint16_t EventID::hash() {
    //uint16_t r = ((uint16_t)val[6])<<6;
    //r += val[7];
    //return r;
    Index i;
    return i.hashcalc(this,8);
}

bool EventID::equals(EventID* n) {
    return  (val[0]==n->val[0])&&(val[1]==n->val[1])
    &&(val[2]==n->val[2])&&(val[3]==n->val[3])
    &&(val[4]==n->val[4])&&(val[5]==n->val[5])
    &&(val[6]==n->val[6])&&(val[7]==n->val[7]);
}

void EventID::print() {
    LDEBUG(" ");
    LDEBUG2(val[0],HEX);
    for (int i=1;i<8;i++) {
        LDEBUG(",");
        LDEBUG2(val[i],HEX);
    }
}

/**
 * Check to see if this object is equal
 * to any in an array of EventIDs
 */
/*
 int EventID::findIndexInArray(EventID* array, int len, int start) {
 for (int i = 0; i<len; i++) {
 if (equals(array+i)) return i;
 }
 return -1;
 }
 */
//EventID EventID::getEID(uint16_t i){
//    EventID r(1,2,3,4,5,6,7,8);
//    return r;
//}

int EventID::findIndexInArray(Index* eventsIndex, int len, int start) {
            //LDEBUG(F("\nIn EventID::findIndexInArray"));
            //LDEBUG(F("\nstart:")); LDEBUG(start);
            //LDEBUG(F("\nthis:")); this->print();
    Index hh;
    hh.hash = this->hash();
            //LDEBUG(F("\nhash:")); LDEBUG2(hh.hash,HEX);
    Index* ei = &eventsIndex[start];
            //LDEBUG(F("\nei->index=")); LDEBUG(ei->index);
            //LDEBUG(F(", hash=")); LDEBUG(ei->hash);
    if (start==0) {
        ei = (Index*)bsearch( (const void*)&hh.hash, (const void*)eventsIndex, len, sizeof(Index), Index::findCompare);
        if(!ei){
        	//LDEBUG("\nNot Found");
        	return -1;
        }
            //LDEBUG(F("\nSearch result:")); ei->print();
        while((ei-1)>=eventsIndex && (ei-1)->hash==hh.hash) {ei--;}
            //LDEBUG(F("\nbackup:")); ei->print();
    }
    if(ei>=(eventsIndex+len))
    	return -1;
    	ei->print();
    if(hh.hash!=ei->hash)
    	return -1;

    EventID eid = blog(ei->index);
            //LDEBUG(F("\nfound:")); eid.print();

    if( 0 == memcmp(this,&eid,8) )
    {
    		// Now figure out the index into the eventsIndex array that is pointed to by ei
	    int foundIndex = ei - eventsIndex;
    		//LDEBUG("\nMatching Index: "); LDEBUG(foundIndex);
		return foundIndex;
    }
    return -1;
}


/*
 int findIndexInArray(uint8_t* array, int len, int start);
 
 uint8_t EventID::getEID( EventID* ev, Event* events);
 int findIndexInArray(Event* events, int len, int start) {
 for (int i = start; i<len; i++) {
 EventID ev;
 getEID( &ev, events[i].ea);
 if (equals(&ev)) return i;
 }
 return -1;
 }
 */


