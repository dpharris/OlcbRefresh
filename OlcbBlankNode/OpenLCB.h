#include <EEPROM.h>

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
#define CDIfooter R"(\
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

typedef struct {
  uint32_t magic;         // used to check eeprom status
  uint16_t nextEID;       // the next available eventID for use from this node's set
  uint8_t  nid[6];        // the nodeID
  char     nodeName[20];  // optional node-name, used by ACDI
  char     nodeDesc[24];  // optional node-description, used by ACDI
}  NodeVar;

typedef struct {
  uint16_t offset;
  uint8_t flags;
} EIDTab;

#define CEID(x) ((unsigned int)&pmem->x,Event::CAN_CONSUME_FLAG)
#define PEID(x) ((unsigned int)&pmem->x,Event::CAN_PRODUCE_FLAG)
#define PCEID(x) ((unsigned int)&pmem->x,Event::CAN_CONSUME_FLAG|Event::CAN_PRODUCE_FLAG)

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
class NodeID {
  public:
    uint8_t val[6];
    NodeID(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f){}
};
class EventID {
  public:
    uint8_t val[6];
    EventID(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f,uint8_t g, uint8_t h){}
    EventID(){}
};
#define PRODUCE_FLAG 0x02
class Event {
  public:
    uint16_t flags;
    void send() { flags |= PRODUCE_FLAG; }
    static const int CAN_CONSUME_FLAG = 0x20;
    static const int CAN_PRODUCE_FLAG = 0x40;
};
class ButtonLed {
  public:
    ButtonLed(int a, int b){}
    void process(){}
    void on(uint32_t p){}
    void blink(uint32_t p){}
};
//Event event[NUM_EVENT] = { Event() };
uint16_t eventsIndex[NUM_EVENT];  // Sorted index to eventids
NodeVar* pnv = 0;
#define NV(x) ((unsigned int)&pnv->x)

class OpenLCB {
  public:
    NodeID* nid;
    uint16_t nextEID = 0;
    uint16_t nevent = NUM_EVENT;
    EventID eventid[NUM_EVENT];
    Event event[NUM_EVENT];
    uint16_t index[NUM_EVENT];
    EIDTab* eidOffset;
    uint16_t sMemStruct;
    bool can_active;
    void (*pceCb)(unsigned int index);
    void (*configW)(unsigned int adress, unsigned int length);
    
    OpenLCB( NodeID* _nodeid, const EIDTab* eventidOffset, uint16_t _sMemStruct,
             void (*_pceCb)(unsigned int i), 
             void (*_configW)(unsigned int a, unsigned int l) ) {
        nid = _nodeid;
        eidOffset = eventidOffset;
        sMemStruct = _sMemStruct;
        pceCb = _pceCb;
        configW = _configW;
    }
    void setup(){}
    void loop(){}
    void newSetEventID(uint16_t nextEID) {
      for(int e=0;e<nevent;e++) {
        EEPROM.put(eidOffset[e].offset,    nid);
        EEPROM.write(eidOffset[e].offset,  nextEID>>8);
        EEPROM.write(eidOffset[e].offset+1,nextEID);
        nextEID++;
      }
    }
    const uint8_t magicOK[4] = { 0xEE, 0x55, 0x5E, 0xE5 };
    const uint8_t magicNAK[4] = { 0xF5, 0x5A, 0xA5, 0x5A };
    void reset(uint16_t nextEID){           // reset system variables and write EIDs
      EEPROM.put(0,magicOK);
      EEPROM.put(NV(nid),nid);
      newSetEventID(nextEID);
      EEPROM.write(NV(nextEID),nextEID>>8);
      EEPROM.write(NV(nextEID)+1,nextEID);
    }
    void factoryReset(){     // reset to factory
      for(int i=0;i<sMemStruct;i++) EEPROM.write(i,0);  // clear EEPROM
      reset(nextEID);      
    }
    void forceInitEIDs() {
      //LDEBUG("\nforceInitEvents");
      EEPROM.update(2,0x33);
      EEPROM.update(3,0xCC);
    }

    
    static int findCompare(const void* a, const void* b){
       uint16_t ia = (uint16_t) a;
       uint16_t ib = (uint16_t) b;
       //for(int i=0; i<8; i++) {
       //  if(eventid[ia].val[i]>eventid[ib].val[i]) return 1;
       //  if(eventid[ia].val[i]<eventid[ib].val[i]) return -1;
       //}
       return 0; // they are equal
    }
    static int sortCompare(const void* a, const void* b){
       uint16_t ia = (uint16_t) a;
       uint16_t ib = (uint16_t) b;
       //for(int i=0; i<8; i++) {
       //  if(eventid[ia].val[i]>eventid[ib].val[i]) return 1;
       //  if(eventid[ia].val[i]<eventid[ib].val[i]) return -1;
       //}
       return 0; // they are equal
    }
    initTables(){        // initialize tables
      EEPROM.get(NV(nid),nid);
      nextEID = EEPROM.read(NV(nextEID)) << 8;
      nextEID += EEPROM.read(NV(nextEID+1));
      for(int e=0; e<nevent; e++) {
        index[e] = e;
        EEPROM.get(eidOffset[e].offset, eventid[e]);
        event[e].flags = eidOffset[e].flags;
      }
      for(int e=0; e<nevent; e++) 
      qsort( index, NUM_EVENT, sizeof(index[0]), sortCompare);
    }
};

// Establish location of node Name and Node Decsription in memory
#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc

