#if 1==0

#include "MemStruct.h"
#include "Event.h"
#include "EventID.h"

//#pragma message( "!!! compiling MemStruct.cpp")

/*
 *  Set of routines that are common to the Memory Models.
 */

  void EventID::writeEID(int index) {
    // All write to EEPROM, may have to restore to RAM.
    Serial.print("\nwriteEID() "); Serial.print(index);
    uint8_t offset = eventidOffset[index];
    for (int i=0;i<8;i++) EEPROM.update(offset+i, this->val[i]);
  }
EventID getEID(unsigned i);
EventID blog(unsigned i) { return getEID(i); }

// === LARGE ============================================

#if MEM_MODEL == MEM_LARGE
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
 //EventID Event::getEID() {
 //          //Serial.print("\ngetEID Large index=");
 //          //Serial.print(index);
 //   int offset = this->offset;
 //   uint8_t* m = (uint8_t*)&mem;
 //   EventID r;
 //   for(int i=0;i<8;i++) r.val[i] = m[i+offset];
 //   return r;
 // }
 EventID getEID(unsigned index) {
    EventID r;
    uint8_t* m = (uint8_t*)&mem;
    uint16_t offset = eventidOffset[index];
    for(int i=0;i<8;i++) r.val[i] = m[i+offset];
    return r;
 }
 /*
 EventID Index::getEID() {
           //Serial.print("\ngetEID Large index=");
           //Serial.print(index);
    //uint16_t i = this->index;
    EventID r;
    uint8_t* m = (uint8_t*)&mem;
    uint16_t offset = eventidOffset[this->index];
    for(int i=0;i<8;i++) r.val[i] = m[i+offset];
    return r;
    //return events[index].getEID();;
 }
 */
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
 /*
 void initEventOffsets() {
        Serial.print("\ninitEventOffsets - Large");
    //for (int i=0;i<sizeof(MemStruct;i++) 
    EEPROM.get(0,mem);
    printRawMem();
    userFillEventOffsets();
 }
 */
 void restore() {
   Serial.print("\nIn restore() Large");
   EEPROM.get(0,mem);  // reload all of eeprom
 }
#endif

// === MEDIUM ===========================================

#if MEM_MODEL == MEM_MEDIUM

  // Initialize the events[], eventsIndex[] tables
  //  - initializes the hash, index and flag fields 
  void initTables() {
    Serial.print("\nIn initTables "); Serial.print(MEM_MODEL);
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
   
 //EventID Event::getEID() {
 //            Serial.print("\ngetEID medium offset=");
 //            printEvents();
 //            Serial.print(this->offset,HEX);
 //            eventids[this->offset].print();
 //  return eventids[this->offset];
 //}
 EventID getEID(unsigned i) {
           //Serial.print("\ngetEID Medium index="); Serial.print(i);
    return eventids[i];
 }
 /*
 EventID Index::getEID() {
    uint16_t index = this->index;
           //Serial.print("\ngetEID Medium index="); Serial.print(index);
    return eventids[index];
 }
 void restore() {
    Serial.print("\nIn restore() Medium");
    for(int e=0; e<8; e++) {
      int offset = events[e].offset;
      for(int i=0;i<8;i++) eventids[e].val[i] = EEPROM.read(offset+i);    
    }
 }
 */
 void restore() {}  // nil to do
#endif

// === SMALL =================================================

#if MEM_MODEL == MEM_SMALL

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
  /*
 EventID Index::getEID() {
    unsigned i = this->index;
    return blog(i);
           //Serial.print(F("\ngetEID Medium index="); Serial.print(index);
    //return events[index].getEID();;
    //int offset = eventidOffset[i];
    //EventID r;
    //for (int i=0;i<8;i++) r.val[i] = EEPROM.read(offset+i);
    //return r;
 } 
 
 void initEventOffsets() {
        Serial.print(F("\ninitEventOffsets - Small"));
    userInitEventIDOffsets();
 }
 */
 void restore() {
   Serial.print(F("\nIn restore() Medium"));
   // restore EEPROM -- nil to do, they remain in EEPROM
 }
#endif
// ^^^^^^^^^^^^^^^^^^^^^^^^ SMALL ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Extras

/*
void printRawEEPROM() {
  Serial.print(F("\nprintRawEEPROM"));
  uint8_t c;
  int rows = sizeof(MemStruct)/16 + 1;
  //for(int r=0; r<64; r++) {
  for(int r=0; r<rows; r++) {
    Serial.print("\n");
    if(r==0) Serial.print(0);
    Serial.print(r*16,HEX);
    for(int i=0;i<16;i++) {
      c = EEPROM.read(r*16+i);
      Serial.print(" ");
      if(c<16) Serial.print(0);
      Serial.print(c,HEX);
    }
    Serial.print(F("->"));
    for(int i=0;i<16;i++) {
      char c = EEPROM.read(r*16+i);
      if(c>' '&&c<'~') Serial.print(c);
      else Serial.print(".");
    }
  }
}
*/
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
#if MEM_MODEL == MEM_MEDIUM
    Serial.print(F(" eventID: ")); eventids[i].print();
#endif
  }
}

#endif
