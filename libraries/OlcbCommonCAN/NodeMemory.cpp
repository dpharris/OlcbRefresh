
#include "NodeMemory.h"

#include "logging.h"
#include "EventID.h"
#include "Event.h"
#include "NodeID.h"
//#include <EEPROM.h>
#include "processor.h"

#define KEYSIZE 4
#define COUNTSIZE 2

// ToDo: NodeID* not kept in object member to save RAM space, may be false economy

NodeMemory::NodeMemory(int start) {
    startAddress = start;
    count = 0;
}

void NodeMemory::forceInitAll() {
    Serial.print("\nforceInitAll");
    EEPROM.write(0,0xFF);
    EEPROM.write(1,0xFF);
}

void NodeMemory::forceInitEvents() {
    Serial.print("\nforceInitEvents");
    EEPROM.write(2,0x33);
    EEPROM.write(3,0xCC);
}

extern void printEvents();
extern void initTables();
extern void userInitEventIDOffsets();
//void NodeMemory::setup(NodeID* nid, Event* cE, uint8_t nC, uint8_t* data, uint16_t extraBytes, uint16_t clearBytes) {
//void NodeMemory::setup(NodeID* nid, Event* cE, const uint16_t* eventidOffset, uint8_t nC, uint8_t* data, uint16_t extraBytes, uint16_t clearBytes) {
void NodeMemory::setup(Nodal_t* nodal, uint8_t* data, uint16_t extraBytes, uint16_t clearBytes) {
    Serial.print("\nIn NodeMemory::setup");
    NodeID* nid = nodal->nid;
    Event* cE  = nodal->events;
    const uint16_t* eventidOffset = nodal->eventidOffset;
    uint16_t nC = nodal->nevent;
    //userInitEventIDOffsets();

    Serial.println();
    for(int i=0;i<nC;i++) { Serial.print(eventidOffset[i],HEX); Serial.print(",");}
    
    if (checkNidOK()) {
        Serial.print("\ncheckNidOK'd");
        
        // leave NodeID and count, reset rest
        
        // read NodeID from non-volative memory
        uint8_t* p;
        int addr = startAddress+KEYSIZE+COUNTSIZE; // skip check word and count
        p = (uint8_t*)nid;
        for (unsigned int i=0; i<sizeof(NodeID); i++) 
           *p++ = EEPROM.read(addr++);

        // load count
        uint8_t part1 = EEPROM.read(startAddress+KEYSIZE);
        uint8_t part2 = EEPROM.read(startAddress+KEYSIZE+1);
        count = (part1<<8)+part2;

        // handle the rest
        reset(nid, cE, eventidOffset, nC, clearBytes);

    } else if (!checkAllOK()) {
        Serial.print("\n!checkAllOK");
        // fires a factory reset
        count = 0;
        // handle the rest
        reset(nid, cE, eventidOffset, nC, clearBytes);
    }
    
    // read NodeID from non-volative memory
    uint8_t* p;
    int addr = startAddress+KEYSIZE+COUNTSIZE; // skip check word and count
    p = (uint8_t*)nid;
    for (uint8_t i=0; i<sizeof(NodeID); i++) 
        *p++ = EEPROM.read(addr++);

    /*
    // read events
    p = (uint8_t*)cE;
    for (uint8_t k=0; k<nC; k++)
        for (unsigned int i=0; i<sizeof(Event); i++) 
            *p++ = EEPROM.read(addr++);
    */
    
    initTables();
    
    // read extra data & load into RAM
    p = data;
    addr = KEYSIZE+COUNTSIZE+sizeof(NodeID)+nC*(sizeof(Event));
    for (uint8_t k=0; k<extraBytes; k++)
        *p++ = EEPROM.read(addr++);
    Serial.print("\nOut NodeMemory::setup");
}

//void NodeMemory::reset(NodeID* nid, Event* cE, uint8_t nC, uint16_t clearBytes) {
void NodeMemory::reset(NodeID* nid, Event* cE, const uint16_t* eventidOffset, uint8_t nC, uint16_t clearBytes) {
    Serial.print("\nNodeMemory::reset1");
    // Do the in-memory update. Does not reset
    // the total count to zero, this is not an "initial config" for factory use.

    // clear EEPROM
    for (uint16_t i = 4; i<clearBytes; i++) writeByte(i, 0);
    //store(nid, cE, nC, eOff);
    store(nid, cE, eventidOffset, nC);
    //Serial.print("\nnC="); Serial.print(nC);
    //while(0==0){}
    for (int e=0; e<nC; e++)
      //setToNewEventID(nid, cE[e].offset);
      setToNewEventID(nid, eventidOffset[e]);
}

//void NodeMemory::store(NodeID* nid, Event* cE, uint8_t nC, uint16_t* eOff) {
void NodeMemory::store(NodeID* nid, Event* cE, const uint16_t* eventidOffset, uint8_t nC) {
    
    int addr = startAddress;
    // write tag
    writeByte(addr++, 0xEE);
    writeByte(addr++, 0x55);
    writeByte(addr++, 0x5E);
    writeByte(addr++, 0xE5);

    // write count
    writeByte(addr++, (count>>8)&0xFF);
    writeByte(addr++, (count)&0xFF);
    
    // write NodeID
    uint8_t* p;
    p = (uint8_t*)nid;
    for (uint8_t i=0; i<sizeof(NodeID); i++) 
        writeByte(addr++, *p++);

    // write events
    // don't need to rewrite the as they do not change --dph
    /*
    p = (uint8_t*)cE;
    for (int k=0; k<nC; k++) {
        for (uint8_t i=0; i<sizeof(EventID); i++) 
            writeByte(addr++, *p++);
        for (uint8_t i=sizeof(EventID); i<sizeof(Event); i++) {
            // skip over the flags
            writeByte(addr++, 0);
            p++;
         }
    }
     */

    // clear some memory
    //for (int k = 0; k < 64; k++) {
    //    writeByte(addr++, 0);
    //}
}

void NodeMemory::store(NodeID* nid, Event* cE, const uint16_t* eventidOffset, uint8_t nC, uint8_t* data, int extraBytes) {
    store(nid, cE, eventidOffset, nC);
    // write extra data
    uint8_t* p = data;
    int addr = KEYSIZE+COUNTSIZE+sizeof(NodeID)+nC*(sizeof(Event));
    for (int k=0; k<extraBytes; k++)
        writeByte(addr++, *p++);
}
void NodeMemory::storeToEEPROM(uint8_t *m, int n) {
    for (int i=0; i<n; i++)
      //  EEPROM.write(i,m[i]);
        writeByte(i, m[i]);
}

void NodeMemory::setToNewEventID(NodeID* nid, uint16_t offset) {
    Serial.print("\nIn setToNewEventID1");
    Serial.print(" offset=");
    Serial.print(offset,HEX);
    // All write to eeprom, may need to do a restore to RAM --dph
    uint8_t p = offset;  // this event's offset
    uint8_t *n = (uint8_t*)nid;
    for (uint8_t k=0; k<sizeof(*nid); k++) {
      writeByte(p++, *n++);
      //Serial.print("\nwriteByte(*p++, *n++)=");
      //Serial.print(p++); Serial.print(",");
      //Serial.print(*n++);
    }
    writeByte(p++, (count++>>8)&0xFF);
      //Serial.print("\ncount=");
      //Serial.print(p++); Serial.print(",");
      //Serial.print((count++>>8)&0xFF);
    writeByte(p++, count&0xFF);
      //Serial.print(", ");
      //Serial.print(p++); Serial.print(",");
      //Serial.print(count&0xFF);  }
}

bool NodeMemory::checkAllOK() {
    Serial.print("\ncheckAllOK");
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x5E ) return false;
    if (EEPROM.read(startAddress+3) != 0xE5 ) return false;
    return true;
}
bool NodeMemory::checkNidOK() {
    Serial.print("\ncheckNIDOK");
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x33 ) return false;
    if (EEPROM.read(startAddress+3) != 0xCC ) return false;
    return true;
}

// write only if needed
void NodeMemory::writeByte(int addr, uint8_t b) {
    if (EEPROM.read(addr) != b ) {
        EEPROM.write(addr, b);
    }
}