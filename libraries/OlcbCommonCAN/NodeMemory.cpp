
#include "NodeMemory.h"

#include "EventID.h"
#include "Event.h"
#include "NodeID.h"
#include "processor.h"

#include "lib_debug_print_common.h"

extern "C" {
    extern uint16_t getOffset(uint16_t index);
    //extern void EventID::writeEID(int index);
    extern void writeEID(int index);
}

#define KEYSIZE 4
#define NEXTEIDSIZE 2

// ToDo: NodeID* not kept in object member to save RAM space, may be false economy

NodeMemory::NodeMemory(int start) {
    startAddress = start;
    count = 0;
}

void NodeMemory::forceInitAll() {
    //LDEBUG("\nforceInitAll");
    EEPROM.update(0,0xFF);
    EEPROM.update(1,0xFF);
}

void NodeMemory::forceInitEvents() {
    //LDEBUG("\nforceInitEvents");
    EEPROM.update(2,0x33);
    EEPROM.update(3,0xCC);
}

extern void printEvents();
extern void initTables();
extern void userInitEventIDOffsets();

void NodeMemory::setup(NodeID* nid, Event* _cE, uint8_t _nC, uint8_t* data, uint16_t extraBytes, uint16_t clearBytes) {
    //LDEBUG("\nIn NodeMemory::setup");
    Event* cE  = _cE;
    uint16_t nC = nodal->_nC;

    //LDEBUG("\nnC(nevents)=");LDEBUG(nC);
    //for(int i=0;i<nC;i++) {
    //    LDEBUG("\n");LDEBUG(i);
    //    LDEBUG(":");LDEBUG2(eventidOffset[i],HEX);
    //}
                            //Serial.print("\nIn NodeMemory::setup");

    if (checkNidOK()) {
        // leave NodeID and count, reset rest
        //LDEBUG("\ncheckNidOK'd");

        // read NodeID from non-volative memory
        uint8_t* p;
        int addr = startAddress+KEYSIZE+NEXTEIDSIZE; // skip check word and count
        p = (uint8_t*)nid;
        for (unsigned int i=0; i<sizeof(NodeID); i++) 
           *p++ = EEPROM.read(addr++);

        // load count
        uint8_t part1 = EEPROM.read(startAddress+KEYSIZE);
        uint8_t part2 = EEPROM.read(startAddress+KEYSIZE+1);
        count = (part1<<8)+part2;

        // handle the rest
        reset(nid, cE, nC);

    } else if (!checkAllOK()) {
        //LDEBUG("\n!checkAllOK");
        // fires a factory reset
        count = 0;
        // handle the rest
        reset(nid, cE, nC);
    }
    
    // read NodeID from non-volative memory
    uint8_t* p;
    int addr = startAddress+KEYSIZE+NEXTEIDSIZE; // skip check word and count
    p = (uint8_t*)nid;
    for (uint8_t i=0; i<sizeof(NodeID); i++) 
        *p++ = EEPROM.read(addr++);
 
    //LDEBUG("\nOut NodeMemory::setup");
}

void NodeMemory::reset(NodeID* nid, Event* cE, uint8_t nC) {
    //LDEBUG("\nNodeMemory::reset1");
    for (uint16_t e=0; e<nC; e++) {
        //setToNewEventID(nid, getOffset(e));
        //uint16_t off = eidtab[e].offset;
        uint16_t off = getOffset(e);
        setToNewEventID(nid, off);
        nextEID++;
    }
}

//void NodeMemory::store(NodeID* nid, Event* cE, uint8_t nC, uint16_t* eOff) {
void NodeMemory::store(NodeID* nid, Event* cE, const uint16_t* eventidOffset, uint8_t nC) {
    
    int addr = startAddress;
    // write tag
    EEPROM.update(addr++, 0xEE);
    EEPROM.update(addr++, 0x55);
    EEPROM.update(addr++, 0x5E);
    EEPROM.update(addr++, 0xE5);

    // write count
    EEPROM.update(addr++, (count>>8)&0xFF);
    EEPROM.update(addr++, (count)&0xFF);
    
    // write NodeID
    uint8_t* p;
    p = (uint8_t*)nid;
    for (uint8_t i=0; i<sizeof(NodeID); i++) 
        EEPROM.update(addr++, *p++);
}

void NodeMemory::setToNewEventID(NodeID* nid, uint16_t offset) {
    //LDEBUG("\nIn setToNewEventID1");
    //LDEBUG(" offset="); LDEBUG2(offset,HEX);
    //Serial.print("\nIn NodeMemory::setToNewEventID offset=");Serial.print(offset);
    EventID eid = *((EventID*)nid);
    //Serial.print("\n nid="); for(int i=0;i<6;++i){Serial.print(nid->val[i],HEX);Serial.print(".");}
    eid.val[6] = (nextEID>>8)&0xFF;
    eid.val[7] = nextEID&0xFF;
    //eid.print();
    EEPROM.put(offset, eid);
}

bool NodeMemory::checkAllOK() {
    //LDEBUG("\ncheckAllOK");
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x5E ) return false;
    if (EEPROM.read(startAddress+3) != 0xE5 ) return false;
    return true;
}
bool NodeMemory::checkNidOK() {
    //LDEBUG("\ncheckNIDOK");
    if (EEPROM.read(startAddress  ) != 0xEE ) return false;
    if (EEPROM.read(startAddress+1) != 0x55 ) return false;
    if (EEPROM.read(startAddress+2) != 0x33 ) return false;
    if (EEPROM.read(startAddress+3) != 0xCC ) return false;
    return true;
}
