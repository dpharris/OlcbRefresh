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

extern "C" {
    extern EventID getEID(unsigned int i);
    extern uint16_t getOffset(uint16_t index);
}

bool EventID::equals(EventID* n) {
    //Serial.print("\nequals("); this->print();
    //n->print(); Serial.print(")");
    return  (val[0]==n->val[0])&&(val[1]==n->val[1])
         &&(val[2]==n->val[2])&&(val[3]==n->val[3])
         &&(val[4]==n->val[4])&&(val[5]==n->val[5])
         &&(val[6]==n->val[6])&&(val[7]==n->val[7]);
}

void EventID::print() {
    LDEBUG(" ");
    LDEBUG2(val[0],HEX);
    Serial.print(" ");
    Serial.print(val[0],HEX);
    for (int i=1;i<8;i++) {
        LDEBUG(",");
        LDEBUG2(val[i],HEX);
        Serial.print(",");
        Serial.print(val[i],HEX);
    }
}

static int findCompare(const void* a, const void* b){
    //  EventID* a, uint16_t* b
    EventID* searchEID = (EventID*)a;
    uint16_t ib = *(uint16_t*)b;
    EventID eid = getEID(ib);
    Serial.print("\nIn findCompare!! ia=");
    Serial.print("\nia->"); searchEID->print();
    Serial.print("\nib->"); eid.print();
    for(int i=0; i<8; i++) {
        //if(searchEID->val[i]>getEID(ib).val[i]) return 1;
        //if(searchEID->val[i]<getEID(ib).val[i]) return -1;
        if(searchEID->val[i]>eid.val[i]) return 1;
        if(searchEID->val[i]<eid.val[i]) return -1;
    }
    return 0; // they are equal
}

int EventID::findIndexInArray(uint16_t* eventIndex, int len, int start) {
    //LDEBUG(F("\nIn EventID::findIndexInArray"));
    //LDEBUG(F("\nstart:")); LDEBUG(start);
    //LDEBUG(F("\nthis:")); this->print();
    EventID eid;
    if(start<0 || start>=len) return -1;
    uint16_t* ei = &eventIndex[start];
    //LDEBUG(F("\nei->index=")); LDEBUG(*ei);
    //Serial.print("\n ei="); Serial.print((ei-&eventIndex[0]));
    if (start==0) {
        ei = (uint16_t*)bsearch( (const void*)this, (const void*)eventIndex, len, sizeof(uint16_t), findCompare);
        if(!ei){
            //LDEBUG("\nNot Found");
            //Serial.print("\nNot found1");
            return -1;
        }
        //LDEBUG(F("\nSearch result:")); eventid[*ei]->print();
        Serial.print("\none:"); Serial.print(*ei);
        while((ei-1)>=eventIndex ) {
            eid = getEID(*(ei-1));
            //Serial.print("\nbacking:");Serial.print(((ei-1)-&eventIndex[0]));
            //Serial.print(":");Serial.print(*(ei-1));
            //Serial.print(":");eid.print();
            if ( !this->equals(&eid) ) break;
            ei--;
        }
        //LDEBUG(F("\nbackup:")); LDEBUG(*ei));
        //Serial.print("\nbacked:");Serial.print((ei-&eventIndex[0]));
        //Serial.print(":");Serial.print(*ei);
        //Serial.print(":");eid.print();
    } else ei++;
    //Serial.print("\nthis:"); this->print();
    if(ei>=(eventIndex+len)) return -1;
    eid = getEID(*ei);
    if (!this->equals(&eid) ) return -1;
    //LDEBUG(F("\nFound: ")); LDEBUG(*ei)); //eventid[*ei]->print();
    //Serial.print("\nfound:");Serial.print((ei-&eventIndex[0]));
    //Serial.print(":");Serial.print(*ei);
    //Serial.print(":");eid.print();
    return ei-&eventIndex[0];  // return the eventIndex indice.
}





