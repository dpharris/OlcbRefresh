//==============================================================
// OlcbBlnkNode
//   A prototype of a basic 4-channel OpenLCB board
// 
//   setup() determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

#define NUM_EVENT 3

#include "OpenLCB.h"

NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 



// CDI
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" { const char configDefInfo[] PROGMEM = R"(
 <cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'>
    <identification>
      <manufacturer>OpenLCB</manufacturer>
      <model>OlcbBasicNode</model>
      <hardwareVersion>1.0</hardwareVersion>
      <softwareVersion>0.4</softwareVersion>
    </identification>
    <segment origin='12' space='253'> <!-- bypasses magic, nextEID, nodeID -->
      <group>
        <name>Node ID</name>
        <description>User-provided description of the node</description>
        <string size='20'><name>Node Name</name></string>
        <string size='24'><name>Node Description</name></string>
      </group>
    <!-- ===== Enter User definitions below ===== -->
      <group replication='1'>
        <name>I/O Events</name>
        <description>Define events associated with input and output pins</description>
        <eventid><name>Consumer Event</name></eventid>
        <eventid><name>Producer Event</name></eventid>
        <eventid><name>P/C Event</name></eventid>
      </group>
    <!-- ===== Enter User definitions above ===== -->
    </segment>
    <segment origin='0' space='253'> <!-- stuff magic to trigger resets -->
      <name>Reset</name>
      <description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description>
      <int size='4'>
        <map>
          <relation><property>3998572261</property><value>(No reset)</value></relation>
          <relation><property>3998561228</property><value>User clear: New default EventIDs, blank strings</value></relation>
          <relation><property>0</property><value>Mfg clear: Reset all, including Node ID</value></relation>
        </map>
      </int>
    </segment>
  </cdi>)";
}

// ===== MemStruct =====
//  Memory structure of EEPROM, must match CDI above
typedef struct { 
    NodeVar nodeVar;         // must remain
    // ===== Enter User definitions below =====
    EventID ceid;              // example consumer eventID
    EventID peid;              // example producer eventID
    EventID pceid;             // example prodcuder/consumer eventID
    // ===== Enter User definitions above =====
  } MemStruct;                 // type definition
MemStruct * pmem = 0;   
// ===== eventidOffset
// Array of offsets of every eventID in MemStruct/EEPROM/mem, and P/C flags
const PROGMEM EIDTab eventidOffset[] = {
      CEID(ceid),    // installs offset to a consumer-eventID with its flags
      PEID(peid),    // installs offset to a producer-eventID with its flags 
      PCEID(pceid)   // installs offset to an eventID that is both a producer and a consumer, with its flags
  };

// next lines get "warning: only initialized variables can be placed into program memory area" due to GCC bug
extern "C" {
  
    // SNIP Short node description for use by the Simple Node Information Protocol 
    //  See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

    // PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
    //  See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
    const uint8_t protocolIdentValue[6] PROGMEM= {
      pSimple | pDatagram | pMemConfig | pPCEvents | pIdent | pTeach, // 1st byte
      pACDI | pSNIP | pCDI,                                           // 2nd byte
      0, 0, 0, 0                                                      // remaining bytes
    };

}; // end extern "C"

// Optional definition of Status indicators and/or buttons
#define BLUE 18    // LEDuino:18; Io:48; Ioduino:14
#define GOLD 19    //
ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

OpenLCB olcb( &nodeid, NUM_EVENT, event, eventsIndex, eventidOffset );
//Nodal_t nodal = { &NodeID(5,1,1,1,3,255), events, eventsIndex, eventidOffset, NUM_EVENT };  // alternate form.

// ===== I/O Definitions ======
// input/output pin drivers
// LEDuino with standard shield: 14, 15, 16, 17 for 
// IOduino to clear built-in blue and gold: 16, 17, 18, 19
// Io 0-7 are outputs & LEDs: 8-15 are inputs
ButtonLed pA(0, LOW);  // pin 0, active low
ButtonLed pB(1, LOW);
ButtonLed pC(8, LOW);
// flashing patterns
#define ShortBlinkOn   0x00010001L  
#define ShortBlinkOff  0xFFFEFFFEL
uint32_t patterns[] = { // two per cchannel, one per event
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn
};
ButtonLed* buttons[] = {  
  &pA,&pA,&pB,&pB,&pC,&pC  // One for each event; in this case, each channel is a pair
};

// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t index){ 
  buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
}

// ===== Process Producers-eventIDs =====
bool getState(unsigned p){}
void produceFromInputs() {  
  static bool state[] = {false, false, false}; 
  for(unsigned p; p<NUM_EVENT; p++) {
    if(event[p].flags & Event::CAN_PRODUCE_FLAG)
      if( getState(p) != state[p]) {
        state[p] ^= 1;  // flip state
        event[p].send(); 
      }
  }
}

// ===== Process changes to Configuration =====
void userConfigWrite(unsigned int address, unsigned int length) {
  if(length==0xFFFF) restore();
}


void setup() {
    Olcb_setup();  // setup system
}

void loop() {
    bool activity = Olcb_loop();    // Process system
    if (activity) blue.blink(0x1);  // blink blue when a frame was received
    if (olcb.can_active) { 
        gold.blink(0x1);            // blink gold when a frame sent
        olcb.can_active = false;
    }
    blue.process();      // handle the status lights  
    gold.process();
}


