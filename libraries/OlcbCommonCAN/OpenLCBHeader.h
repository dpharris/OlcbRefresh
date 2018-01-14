//
//  OpenLCBHeader.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OpenLCBHeader_h
#define OpenLCBHeader_h

#include "NodeID.h"

typedef struct NodeVar_ {
    uint32_t magic;         // used to check eeprom status
    uint16_t nextEID;       // the next available eventID for use from this node's set
    NodeID   nid;        // the nodeID
    char     nodeName[20];  // optional node-name, used by ACDI
    char     nodeDesc[24];  // optional node-description, used by ACDI
} NodeVar;
//extern NodeVar* pNodeVars = 0;
//#define NV(x) EEADDR(nodeVar.x)


// ===== eventidOffset Support =====================================
//   Note: this allows system routines to initialize event[]
//         since eventOffsets are constant in flash.
typedef struct {
    uint16_t offset;
    uint16_t flags;
} EIDTab;


//#include <EEPROM.h>
//#include "Can.h"
#include "OpenLcbCanBuffer.h"
#include "PIP.h"
#include "SNII.h"
#include "PCE.h"

// specific OpenLCB implementations
#include "LinkControl.h"

//#ifndef OLCB_NO_DATAGRAM
#include "Datagram.h"
//#endif
//#ifndef OLCB_NO_STREAM
#include "OlcbStream.h"
//#endif
//#ifndef OLCB_NO_MEMCONFIG
#include "Configuration.h"
//#endif
//#ifndef OLCB_NO_MEMCONFIG
#include "Configuration.h"
//#endif

#include "NodeMemory.h"
#include "PCE.h"
#include "PIP.h"
#include "SNII.h"
#include "BG.h"
#include "ButtonLed.h"
#include "Event.h"

#include "OlcbCanCLass.h"

// ===== CDI System Portions =======================================
#define CDIheader R"( \
    <cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'> \
    <identification> \
        <manufacturer>OpenLCB</manufacturer>\
        <model>OlcbBasicNode</model>\
        <hardwareVersion>1.0</hardwareVersion>\
        <softwareVersion>0.4</softwareVersion>\
        </identification>\
    <segment origin='12' space='253'> <!-- bypasses magic, nextEID, nodeID -->\
        <group>\
            <name>Node ID</name>\
            <description>User-provided description of the node</description>\
            <string size='20'><name>Node Name</name></string>\
            <string size='24'><name>Node Description</name></string>\
        </group>)" 
#define CDIfooter R"( \
    </segment>\
    <segment origin='0' space='253'> <!-- stuff magic to trigger resets -->\
        <name>Reset</name>\
        <description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description>\
        <int size='4'>\
            <map>\
                <relation><property>3998572261</property><value>(No reset)</value></relation>\
                <relation><property>3998561228</property><value>User clear: New default EventIDs, blank strings</value></relation>\
                <relation><property>0</property><value>Mfg clear: Reset all, including Node ID</value></relation>\
            </map>\
        </int>\
    </segment>\
    </cdi>)"


extern "C" {
    extern const EIDTab eidtab[];
}

#define EEADDR(x) ((unsigned int)&pmem->x)
#define CEID(x)    EEADDR(x), Event::CAN_CONSUME_FLAG
#define PEID(x)    EEADDR(x), Event::CAN_PRODUCE_FLAG
#define PCEID(x)   EEADDR(x), Event::CAN_CONSUME_FLAG|Event::CAN_PRODUCE_FLAG

// ===== PIP Support ===============================================
#define pSimple       0x80
#define pDatagram     0x40
#define pStream       0x20
#define pMemConfig    0x10
#define pReservation  0x08
#define pPCEvents     0x04
#define pIdent        0x02
#define pTeach        0x01

#define pRemote       0x80
#define pACDI         0x40
#define pDisplay      0x20
#define pSNIP         0x10
#define pCDI          0x08
#define pTraction     0x04
#define pFunction     0x02
#define pDCC          0x01

#define pSimpleTrain  0x80
#define pFuncConfig   0x40
#define pFirmwareUpgrade     0x20
#define pFirwareUpdateActive 0x10

#define OlcbCommonVersion "0.0.1"

//#define Can OlcbCanClass

extern void pceCallback(unsigned int i);
extern void restore();
////Nodal* nodal;
extern PCE pce;

// Establish location of node Name and Node Decsription in memory
//#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
//#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc
#define SNII_var_data 12           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset 20          // location of nodeDesc


Event event[NUM_EVENT];             // operating flags
EventID eventid[NUM_EVENT];         // copy of EIDs from EEPROM
uint16_t eventIndex[NUM_EVENT];     // Sorted index to eventids
//uint16_t eventOffset[NUM_EVENT];    // stored in flash


// Extras
void printEventIndexes() {
    Serial.print(F("\nprintEventIndex\n"));
    for(int i=0;i<NUM_EVENT;i++) {
        Serial.print(eventIndex[i],HEX); Serial.print(F(", "));
        //eventsIndex[i].print();
        //Serial.print(F("\n hash: ")); Serial.print(eventIndex[i].hash,HEX);
        //Serial.print(F("  index: ")); Serial.print(eventIndex[i].index,HEX);
    }
}
void printEvents() {
    Serial.print(F("\nprintEvents "));
    //Serial.print(MEM_MODEL);
    for(int i=0;i<8;i++) {
        //Serial.print(F("\n  offset: ")); Serial.print(events[i].offset,HEX);
        Serial.print("\n"); Serial.print(i); Serial.print(":");
        Serial.print(F(" flags: ")); Serial.print(event[i].flags,HEX);
        Serial.print(F(" eventID: ")); eventid[i].print();
    }
}

void printEventids() {
    Serial.print("\neventids:");
    for(int e=0;e<NUM_EVENT;e++) {
        Serial.print("\n[");
        for(int i=0;i<8;i++) {
            Serial.print(eventid[e].val[i],HEX); Serial.print(", ");
        }
    }
}
void printSortedEvents() {
    Serial.print("\Sorted events");
    for(int i=0; i<NUM_EVENT; i++) {
        Serial.print("\n");
        Serial.print(i); Serial.print(": ");
        int e = eventIndex[i];
        Serial.print(e); Serial.print(": ");
        for(int j=0;j<8;j++) {
            Serial.print(eventid[e].val[j]);
            Serial.print(".");
        }
    }
}






#endif /* OpenLCBHeader_h */
