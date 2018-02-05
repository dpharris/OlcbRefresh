//
//  OpenLCBMid.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OpenLCBMid_h
#define OpenLCBMid_h

#define NV(x) EEADDR(nodeVar.x)

//#include "OpenLcbCanInterface.h"
//#include "OpenLcbCanBuffer.h"
#include "NodeID.h"
#include "EventID.h"
#include "Event.h"

// specific OpenLCB implementations
#include "LinkControl.h"

#ifndef OLCB_NO_DATAGRAM
#include "Datagram.h"
#endif

#ifndef OLCB_NO_STREAM
#include "OlcbStream.h"
#endif

#ifndef OLCB_NO_MEMCONFIG
#include "Configuration.h"
#endif

#include "NodeMemory.h"
#include "PCE.h"
#include "PIP.h"
#include "SNII.h"
#include "BG.h"
#include "ButtonLed.h"



Can olcbcanRx;
Can olcbcanTx;
OlcbCanInterface     rxBuffer(&olcbcanRx);  // CAN receive buffer
OlcbCanInterface     txBuffer(&olcbcanTx);  // CAN send buffer

#define LAST_EEPROM sizeof(MemStruct)

// flashGet
//template <typename T> T flashGet(const T* sce) {
//    static T r;
//    memcpy_P (&r, sce, sizeof (T));
//    return r;
//}


// The variable parts of the SNII protocol are stored in EEPROM, as secribed by MemStruct
MemStruct *pmem = 0;
#define SNII_var_data &pmem->nodeVar.nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeVar.nodeName)  // location of nodeDesc


extern "C" {

    // EIDTab - eventID_Table is stored in Flash
    // It has two fields:
    //   offset = offset to each eventID in the MemStruct in EEPROM
    //   flags = the initial flags indicating whether the eventID showed be announced at startup
    uint16_t getOffset(unsigned index) {
        return pgm_read_word(&eidtab[index].offset);
    }
    uint16_t getFlags(unsigned index) {
        return pgm_read_word(&eidtab[index].flags);
    }

    uint32_t spaceUpperAddr(uint8_t space) {  // return last valid address
        switch (space) {
            case 255: return sizeof(configDefInfo) - 1; // CDI (data starts at zero)
            case 254: return RAMEND; // RAM from Arduino definition
            case 253: return LAST_EEPROM; // Configuration
        }
        return (uint32_t)3;
    }
    
    //const uint8_t getRead(uint32_t address, int space) {
    const uint8_t getRead(uint32_t address, int space) {
        if (space == 0xFF) { // 255
            // Configuration definition information
            return pgm_read_byte(configDefInfo+address);
        } else if (space == 0xFE) { //254
            // All memory reads from RAM starting at first location in this program
            return *(((uint8_t*)&rxBuffer)+address);
        } else if (space == 0xFD) { //253
            // Configuration space is entire EEPROM
            uint8_t r = EEPROM.read(address);
            //Serial.print("\ngetRead "); Serial.print(space,HEX);
            //Serial.print(":"); Serial.print(address,HEX);
            //Serial.print("="); Serial.print(r,HEX);
            return r;
        } else if (space == 0xFC) { // 252
            // used by ADCDI/SNII for constant data
            return pgm_read_byte(SNII_const_data+address);
        } else if (space == 0xFB) { // 251
            // used by ADCDI/SNII for variable data
            return EEPROM.read((int)SNII_var_data+address);
        } else {
            // unknown space
            return 0;
        }
    }
    
    void getWrite(uint32_t address, int space, uint8_t val) {
                        //Serial.print("\nolcbinc getWrite");
                        //Serial.print(" space: "); Serial.print(space,HEX);
                        //Serial.print(":"); Serial.print(address,HEX);
                        //Serial.print("="); Serial.print(val,HEX);
        if (space == 0xFE) {
            // All memory
            *(((uint8_t*)&rxBuffer)+address) = val;
        } else if (space == 0xFD) {
            // Configuration space
            EEPROM.write(address, val);
            //configWritten(address, val);
        }
        // all other spaces not written
    }
    
    void printeidtab() {
        Serial.print("\neidtab:\n");
        for(int i=0;i<NUM_EVENT;i++) {
            Serial.print("[");
            Serial.print(getOffset(i),HEX); Serial.print(", ");
            Serial.print(getFlags(i),HEX); Serial.print("], ");
        }
    }
    // Extras
    void printEventIndexes() {
        Serial.print(F("\nprintEventIndex\n"));
        for(int i=0;i<NUM_EVENT;i++) {
            Serial.print(eventIndex[i],HEX); Serial.print(F(", "));
        }
    }
    void printEvents() {
        Serial.print(F("\nprintEvents "));
        Serial.print(F("\n#  flags  EventID"));
        for(int i=0;i<8;i++) {
            //Serial.print(F("\n  offset: ")); Serial.print(events[i].offset,HEX);
            Serial.print("\n"); Serial.print(i);
            //Serial.print(":"); Serial.print(eidtab[i].offset,HEX);
            Serial.print(":"); Serial.print(getOffset(i),HEX);
            Serial.print(F(" : ")); Serial.print(event[i].flags,HEX);
            //Serial.print(F(" : ")); eventid[i].print();
            Serial.print(F(" : ")); event[i].eid.print();
        }
    }
    
    void printEventids() {
        Serial.print("\neventids:");
        for(int e=0;e<NUM_EVENT;e++) {
            Serial.print("\n[");
            for(int i=0;i<8;i++) {
                //Serial.print(eventid[e].val[i],HEX); Serial.print(", ");
                Serial.print(event[e].eid.val[i],HEX); Serial.print(", ");
            }
        }
    }
    void printSortedEvents() {
        Serial.print("\nSorted events");
        for(int i=0; i<NUM_EVENT; i++) {
            Serial.print("\n");
            Serial.print(i); Serial.print(": ");
            int e = eventIndex[i];
            Serial.print(e); Serial.print(": ");
            for(int j=0;j<8;j++) {
                //Serial.print(eventid[e].val[j]);
                Serial.print(event[e].eid.val[j]);
                Serial.print(".");
            }
        }
    }
} // end of extern

LinkControl link(&txBuffer, &nodeid);

#ifndef OLCB_NO_STREAM
    unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length);
    OlcbStream str(&txBuffer, streamRcvCallback, &link);
    unsigned int resultcode = 1;  // dummy temp value
    unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length){
        return resultcode;  // return pre-ordained result
    }
#else
    #define str 0
#endif

#ifndef OLCB_NO_DATAGRAM
    unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from);
    Datagram dg(&txBuffer, datagramCallback, &link);
    Configuration cfg(&dg, &str, getRead, getWrite, (void (*)())0, configWritten);
    //Configuration cfg(&dg, &str, getRead, getWrite, (void (*)())0, (void (*)(unsigned int, unsigned int))0 );

    unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from) {
        // invoked when a datagram arrives
        //logstr("consume datagram of length ");loghex(length); lognl();
        //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
        //printf("\n");
        // pass to consumers
        return cfg.receivedDatagram(rbuf, length, from);
    }
#else
    #define cfg 0
    #define dg  0
#endif

//Nodal_t nodal = { &nodeid, event, eventIndex, eidtab, NUM_EVENT };

//PCE pce(&nodeid, event, eventIndex, eidtab, NUM_EVENT, &txBuffer, pceCallback, restore, &link);
PCE pce(event, NUM_EVENT, eventIndex, &txBuffer, pceCallback, restore, &link);
//PCE::PCE(Event* evts, int nEvt, uint16_t* eIndex, OpenLcbCanBuffer* b, void (*cb)(uint16_t i), void (*rest)(), LinkControl* li)
#ifndef OLCB_NO_BLUE_GOLD
    BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);
#else
    #define bg 0
#endif

NodeMemory nm(0);

extern "C" {
    
    extern EventID getEID(unsigned i) {
        //return eventid[i];
        return event[i].eid;
    }
    
    extern void writeEID(int index) {
        // All write to EEPROM, may have to restore to RAM.
        Serial.print("\nwriteEID() "); Serial.print(index);
        EEPROM.put(getOffset(index), event[index].eid);
    }
}

void restore() {
    //initTables   !!!!!!!!
}

static int sortCompare(const void* a, const void* b){
    uint16_t ia = *(uint16_t*)a;
    uint16_t ib = *(uint16_t*)b;
    Serial.print("\nIn sortCompare!! ia=");Serial.print(ia);
    Serial.print(" ib=");Serial.print(ib);
    //EventID ea = eventid[ia];
    for(unsigned int i=0; i<8; i++) {
        if(event[ia].eid.val[i]>event[ib].eid.val[i]) return 1;
        if(event[ia].eid.val[i]<event[ib].eid.val[i]) return -1;
    }
    return 0; // they are equal
}


extern void initTables(){        // initialize tables
    
    for(unsigned int e=0; e<NUM_EVENT; e++) {
        eventIndex[e] = e;
        EEPROM.get(getOffset(e), event[e].eid);
        event[e].flags |= getFlags(e);
    }

    /*
    eventid[0].val[7]=7;
    eventid[1].val[7]=4;
    eventid[2].val[7]=5;
    eventid[3].val[7]=4;
    eventid[4].val[7]=4;
    eventid[5].val[7]=4;
    eventid[6].val[7]=1;
    eventid[7].val[7]=0;

    printEventIndexes();
    printEvents();
        qsort( eventIndex, NUM_EVENT, sizeof(eventIndex[0]), sortCompare);
    printEventIndexes();
    printEvents();
    */

}

//extern can_init();
// ===== System Interface  
void Olcb_init() {       // was setup()
            Serial.print("\nIn olcb::init");
    nm.setup(&nodeid, event, NUM_EVENT, (uint16_t)sizeof(MemStruct));
            Serial.print("\nIn olcb::init1");
    
    initTables();
    printEventIndexes();
    printEvents();
            Serial.print("\nIn olcb::init2");

    PIP_setup(&txBuffer, &link);
    //SNII_setup((uint8_t)sizeof(SNII_const_data), SNII_var_offset, &txBuffer, &link);
            Serial.print("\nIn olcb::init3");
    //SNII_setup((uint8_t)32, SNII_var_offset, &txBuffer, &link);
    SNII_setup((uint8_t)sizeof(SNII_const_data), SNII_var_offset, &txBuffer, &link);
            Serial.print("\nIn olcb::init4");
    //can.init();
    olcbcanTx.init();
            Serial.print("\nIn olcb::init5");
    link.reset();
            Serial.print("\nIn olcb::init6");
}

// Main processing loop
//
bool Olcb_process() {   // was loop()

    bool rcvFramePresent = rxBuffer.net->read();

    link.check();

    bool handled = false;  // start accumulating whether it was processed or skipped
    if (rcvFramePresent) {
        handled = link.receivedFrame(&rxBuffer);
    }

    if (link.linkInitialized()) {
        if (rcvFramePresent && rxBuffer.isForHere(link.getAlias()) ) {
        //if (rcvFramePresent && rxBuffer.isForHere(nodeid) ) {
#ifndef OLCB_NO_DATAGRAM
            handled |= dg.receivedFrame(&rxBuffer);  // has to process frame level
#endif
            if(rxBuffer.isFrameTypeOpenLcb()) {  // skip if not OpenLCB message (already for here)
                handled |= pce.receivedFrame(&rxBuffer);
#ifndef OLCB_NO_STREAM
                handled |= str.receivedFrame(&rxBuffer); // suppress stream for space
#endif
                handled |= PIP_receivedFrame(&rxBuffer);
                handled |= SNII_receivedFrame(&rxBuffer);
                if (!handled && rxBuffer.isAddressedMessage()) link.rejectMessage(&rxBuffer, 0x2000);
            }
        }
        pce.check();
#ifndef OLCB_NO_DATAGRAM
        dg.check();
#endif
#ifndef OLCB_NO_STREAM
        str.check();
#endif
#ifndef OLCB_NO_MEMCONFIG
        cfg.check();
#endif
#ifndef OLCB_NO_BLUE_GOLD
        bg.check();
#endif
        PIP_check();
        SNII_check();
        //produceFromInputs();
    }
    return rcvFramePresent;
}




#endif /* OpenLCBMid_h */
