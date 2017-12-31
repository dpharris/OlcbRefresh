//==============================================================
// OlcbBlnkNode
//   A prototype of a basic 4-channel OpenLCB board
//
//   David P Harris copyright 2017
//==============================================================

#define NUM_EVENT 3
//#define USE_BG

#include "OpenLCB.h"
#include "Can.h"
Can *can;


NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 

// CDI
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" { 
  const char configDefInfo[] PROGMEM = 
      CDIheader R"(
    // ===== Enter User definitions below =====
      <group replication='1'>
        <name>I/O Events</name>
        <description>Define events associated with input and output pins</description>
        <eventid><name>Consumer Event</name></eventid>
        <eventid><name>Producer Event</name></eventid>
        <eventid><name>P/C Event</name></eventid>
      </group>
    // ===== Enter User definitions above =====
      )"CDIfooter;
}

// ===== MemStruct =====
//  Memory structure of EEPROM, must match CDI above
typedef struct { 
    NodeVar nodeVar;         // must remain
    // ===== Enter User definitions below =====
      struct {
        EventID ceid;              // example consumer eventID
        EventID peid;              // example producer eventID
        EventID pceid;             // example prodcuder/consumer eventID
      } io;
    // ===== Enter User definitions above =====
} MemStruct;                 // type definition
MemStruct *pmem = 0;   

// ===== eventidOffset =====
// Array of offsets of every eventID in MemStruct/EEPROM/mem, and P/C flags
const PROGMEM EIDTab eventidOffset[] = {
      CEID(io.ceid),    // installs offset to a consumer-eventID with its flags
      PEID(io.peid),    // installs offset to a producer-eventID with its flags 
      PCEID(io.pceid)   // installs offset to an eventID that is both a producer and a consumer, with its flags
  };

extern "C" {  
    // SNIP Short node description for use by the Simple Node Information Protocol 
    //  See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    const char SNII_const_data[] PROGMEM = 
        "\001OpenLCB\000DPHOlcbBlankNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

    // PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
    //  See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
    const uint8_t protocolIdentValue[6] PROGMEM= {
        pSimple | pDatagram | pMemConfig | pPCEvents | pIdent | pTeach, // 1st byte
        pACDI | pSNIP | pCDI,                                           // 2nd byte
        0, 0, 0, 0                                                      // remaining bytes
    };
}; // end extern "C"

#ifdef USE_BG
// Optional definition of Status indicators and/or buttons
#define BLUE 18    // LEDuino:18; Io:48; Ioduino:14
#define GOLD 19    //
ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

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
BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);
#endif

// ===== Process Consumer-eventIDs =====
extern void pceCallback(uint16_t index){ 
  //EG:  buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
}

// ===== Process Producers-eventIDs =====
bool getState(unsigned p){}
void produceFromInputs() { }

// ===== Process changes to Configuration =====
void configWritten(unsigned int address, unsigned int length) { }

// Define OpenLCB instantiation
OpenLCB olcb( &nodeid, NUM_EVENT, eventidOffset, sizeof(MemStruct), can, pceCallback, configWritten);

void setup() {
    olcb.setup();  // setup system
    // olcb.reset(); // uncomment to restore factory settings
}

void loop() {
    bool activity = olcb.loop();    // Process system
    #ifdef USE_BG
        if (activity) blue.blink(0x1);  // blink blue when a frame was received
        if (olcb.can_active) { 
            gold.blink(0x1);            // blink gold when a frame sent
            olcb.can_active = false;
        }
        blue.process();      // handle the status lights  
        gold.process();
    #endif
    //produceFromInputs();
}


