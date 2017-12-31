/*
#include "MemStruct.h"
#include "Event.h"
#include "EventID.h"

//#pragma message( "!!! compiling MemStruct.cpp")

// Set of routines that are common to the Memory Models.

  void EventID::writeEID(int index) {
    // All write to EEPROM, may have to restore to RAM.
    Serial.print("\nwriteEID() "); Serial.print(index);
    uint8_t offset = eventidOffset[index];
    for (int i=0;i<8;i++) EEPROM.update(offset+i, this->val[i]);
  }
EventID getEID(unsigned i);
EventID blog(unsigned i) { return getEID(i); }

// === LARGE ============================================
#ifdef MEM_MODEL_LARGE
MemStruct mem = EEPROM.get(0,mem);

  // Initialize the events[], eventsIndex[] tables
  //  - initializes the hash, index and flag fields 
  void initTables() {
    //EEPROM.get(0, mem);
    printRawMem();
    //userInitEventIDOffsets();
    uint8_t* m = (uint8_t*)&mem;
    for (int e=0; e<NUM_EVENT; e++) {
      events[e].flags = 0;
      EventID* p = (EventID*)&m[eventidOffset[e]];
      eventsIndex[e].hash = p->hash();
      eventsIndex[e].index = e;
    }
    qsort( eventsIndex, NUM_EVENT, sizeof(Index), Index::sortCompare);
    //printEventsIndex();
    //printEvents();
    //Serial.print(F("\nOut initTables Large"));
  }
 EventID getEID(unsigned index) {
    EventID r;
    uint8_t* m = (uint8_t*)&mem;
    uint16_t offset = eventidOffset[index];
    for(int i=0;i<8;i++) r.val[i] = m[i+offset];
    return r;
 }
 void printRawMem() {
  uint8_t* m = (uint8_t*)&mem;
  int rows = sizeof(MemStruct)/16 + 1;
  uint8_t c;
  //for(int r=0; r<64; r++) {
    Serial.print("\nprintRawMem()");
  for(int r=0; r<rows; r++) {
    Serial.print("\n");
    if(r==0) Serial.print(0);
    Serial.print(r*16,HEX);
    for(int i=0;i<16;i++) {
      c = m[r*16+i];
      Serial.print(" ");
      if(c<16) Serial.print(0);
      Serial.print(c,HEX);
    }
    Serial.print("->");
    for(int i=0;i<16;i++) {
      char c = m[r*16+i];
      if(c>' '&&c<'~') Serial.print(c);
      else Serial.print(".");
    }
  }
 }
 void restore() {
   Serial.print("\nIn restore() Large");
   EEPROM.get(0,mem);  // reload all of eeprom
   // initTables();
 }
#endif

// === MEDIUM ===========================================

#ifdef MEM_MODEL_MEDIUM

  // Initialize the events[], eventsIndex[] tables
  //  - initializes the hash, index and flag fields 
  void initTables() {
    Serial.print("\nIn initTables "); Serial.print("MEM_MODEL_MEDIUM");
    //userInitEventIDOffsets();
    for (int e=0; e<NUM_EVENT; e++) {
      events[e].flags = 0;
      EEPROM.get(eventidOffset[e], eventids[e]);
      eventsIndex[e].hash = eventids[e].hash();
      eventsIndex[e].index = e;
    }
    qsort( eventsIndex, NUM_EVENT, sizeof(Index), Index::sortCompare);
    //printEventsIndex();
    //printEvents();
    Serial.print("\nOut initTables M");
  }
 EventID getEID(unsigned i) {
           //Serial.print("\ngetEID Medium index="); Serial.print(i);
    return eventids[i];
 }
 void restore() {
  //initTables
 }
#endif

// === SMALL =================================================

#ifdef MEM_MODEL_SMALL

  void initTables() {
    Serial.print("\ninitTables");
    //userInitEventIDOffsets();        //  ?????  trial
    for (int e=0; e<NUM_EVENT; e++) {
      EventID eid;
      EEPROM.get(eventidOffset[e], eid);
      eventsIndex[e].hash = eid.hash();
      eventsIndex[e].index = e;
      events[e].flags = 0;
    }
    eventsIndex->sort(NUM_EVENT);
    //qsort( eventsIndex, NUM_EVENT, sizeof(Index), Index::sortCompare);
    //printEventsIndex();
    //printEvents();
    Serial.print(F("\nOut initTables S"));
  }
 EventID getEID(unsigned i) {
             //Serial.print(F("\ngetEID small"));
    int offset = eventidOffset[i];
    EventID r;
    for (int i=0;i<8;i++) r.val[i] = EEPROM.read(offset+i);
    return r;
 }
 void restore() {
   Serial.print(F("\nIn restore() Medium"));
   //initTables();
 }
#endif
// ^^^^^^^^^^^^^^^^^^^^^^^^ SMALL ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Extras
void printEventsIndex() {
  Serial.print(F("\nprintEventsIndex"));
  for(int i=0;i<NUM_EVENT;i++) {
    Serial.print("\n");
    eventsIndex[i].print();
    //Serial.print(F("\n hash: ")); Serial.print(eventsIndex[i].hash,HEX);
    //Serial.print(F("  index: ")); Serial.print(eventsIndex[i].index,HEX);
  }
}
void printEvents() {
  Serial.print(F("\nprintEvents "));
  //Serial.print(MEM_MODEL);
  for(int i=0;i<8;i++) {
    //Serial.print(F("\n  offset: ")); Serial.print(events[i].offset,HEX);
    Serial.print(F(" flags: ")); Serial.print(events[i].flags,HEX);
    #ifdef MEM_MODEL_MEDIUM
       Serial.print(F(" eventID: ")); eventids[i].print();
    #endif
  }
}
*/
