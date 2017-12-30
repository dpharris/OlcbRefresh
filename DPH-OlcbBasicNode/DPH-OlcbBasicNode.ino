//==============================================================
// OlcbBasicNode
//   A prototype of a basic 4-channel OpenLCB board
// 
//   setup() determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

#include "OlcbCommonVersion.h"

// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 4

// total number of events, two per channel
#define NUM_EVENT 2*NUM_CHANNEL

// Description of EEPROM memory structure, and the mirrored mem if in MEM_LARGE
#include "MemStruct.h"

NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 

// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 18

// GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
#define GOLD 19

// next lines get "warning: only initialized variables can be placed into program memory area" due to GCC bug
extern "C" {
  
 // CDI (Configuration Description Information) in xml, must match MemStruct
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
 #include "cdi.h"
 
 // SNIP Short node description for use by the Simple Node Information Protocol 
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
 const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

}; // end extern "C"

// Establish location of node Name and Node Decsription in memory
#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
      // PIP, Datagram, MemConfig, P/C, ident, teach/learn, 
      // ACDI, SNIP, CDI

      /* whole set: 
       *  Simple, Datagram, Stream, MemConfig, Reservation, Events, Ident, Teach
       *  Remote, ACDI, Display, SNIP, CDI, Traction, Function, DCC
       *  SimpleTrain, FuncConfig, FirmwareUpgrade, FirwareUpdateActive,
       *  ... additional ones may be added
       */

#include "OlcbArduinoCAN.h"
#include "OlcbInc1.h"

//Event events[NUM_EVENT] = { Event() };   // repeated for all eight events.  

Nodal_t nodal = { &nodeid, events, eventsIndex, eventidOffset, NUM_EVENT };
//Nodal_t nodal = { &NodeID(5,1,1,1,3,255), events, eventsIndex, eventidOffset, NUM_EVENT };  // alternate form.

// input/output pin drivers
// 14, 15, 16, 17 for LEDuino with standard shield
// 16, 17, 18, 19 for IOduino to clear built-in blue and gold
// Io 0-7 are outputs & LEDs, 8-15 are inputs
ButtonLed pA(0, LOW); 
ButtonLed pB(1, LOW);
ButtonLed pC(8, LOW);
ButtonLed pD(9, LOW);

#define ShortBlinkOn   0x00010001L
#define ShortBlinkOff  0xFFFEFFFEL

uint32_t patterns[] = { // two per cchannel, one per event
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn,
  ShortBlinkOff,ShortBlinkOn
};
ButtonLed* buttons[] = {  // One for each event; each channel is a pair
                        &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD
                       };

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

void pceCallback(uint16_t index){
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events.  
  // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
  // The pattern is mostly one way, blinking the other, hence inverse.
  //
  //Serial.print(F("\npceCallback()")); Serial.print(index);
  buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
}

NodeMemory nm(0);  // allocate from start of EEPROM
void store() { nm.store(&nodeid, events, eventidOffset, NUM_EVENT); }

//PCE pce(events, NUM_EVENT, &txBuffer, &nodeid, pceCallback, store, &link);
//PCE pce(events, NUM_EVENT, eventsIndex, &txBuffer, &nodeid, pceCallback, restore, &link);
PCE pce(&nodal, &txBuffer, pceCallback, restore, &link);

// Set up Blue/Gold configuration
BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);

bool states[] = {false, false, false, false}; // current input states; report when changed

// On the assumption that the producers (inputs) and consumers (outputs) are consecutive, 
// these are used later to label the individual channels as producer or consumer
#define FIRST_PRODUCER_CHANNEL_INDEX    0
#define LAST_PRODUCER_CHANNEL_INDEX     NUM_CHANNEL/2-1
#define FIRST_CONSUMER_CHANNEL_INDEX    NUM_CHANNEL/2
#define LAST_CONSUMER_CHANNEL_INDEX     NUM_CHANNEL-1

void produceFromInputs() {
  // called from loop(), this looks at changes in input pins and 
  // and decides which events to fire
  // with pce.produce(i);
  // The first event of each pair is sent on button down,
  // and second on button up.
  // 
  // To reduce latency, only MAX_INPUT_SCAN inputs are scanned on each loop
  //    (Should not exceed the total number of inputs, nor about 4)
  #define MAX_INPUT_SCAN 4
  static int scanIndex = 0;
  //
  for (int i = 0; i<(MAX_INPUT_SCAN); i++) { // simply a counter of how many to scan
    if (scanIndex < (LAST_PRODUCER_CHANNEL_INDEX)) scanIndex = (FIRST_PRODUCER_CHANNEL_INDEX);
    if (states[scanIndex] != buttons[scanIndex*2]->state) {
      states[scanIndex] = buttons[scanIndex*2]->state;
      if (states[scanIndex]) {
        pce.produce(scanIndex*2);
      } else {
        pce.produce(scanIndex*2+1);
      }
    }
  }
}

//
// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 
void userConfigWrite(unsigned int address, unsigned int length) {
  //Serial.print("\nuserConfigWrite "); Serial.print(address,HEX);
  //Serial.print(":"); Serial.print(length,HEX);
  // resets the board:
  //if( address==0 && length==0xFFFF ) setup();
  // example: if a servo's position changed, then update it immediately
  // uint8_t posn;
  // for(unsigned i=0; i<NCHANNEL; i++) {
  //    unsigned int pposn = &pmem->channel[i].posn; 
  //    if( (address<=pposn) && (pposn<(address+length) ) posn = EEPROM.read(pposn);
  //    servo[i].set(i,posn);
  // }
}

void userInitEventTypes() {
  // Set event types, now that IDs have been loaded from configuration
  // newEvent arguments are (event index, producer?, consumer?)
    for (int i=2*(FIRST_PRODUCER_CHANNEL_INDEX); i<2*(LAST_PRODUCER_CHANNEL_INDEX+1); i++) {
      pce.newEvent(i,true,false); // producer
    }
    for (int i=2*(FIRST_CONSUMER_CHANNEL_INDEX); i<2*(LAST_CONSUMER_CHANNEL_INDEX+1); i++) {
      pce.newEvent(i,false,true); // consumer
    }
}
// Unit testing
//  Uncomment a test
//#define test testEquals()
//#define test testFindIndexInArray
//#define test testIndex_FindIndex
//#define test testBsearch
//#define test testPCEEventReport
//#define test testSpeed
//#define test testPCEHandleLearnEvent
//#define test testIdentifyConsumers
//#define test testIdentifyProducers
//#define test testIdentifyEvents
//#define test testCan
#ifdef test 
  #include "unittesting.h"
#endif

//#include "streaming.h"

/**
 * Setup does initial configuration
 */
void setup()
{
  // set up serial comm; may not be space for this!
  delay(250);Serial.begin(115200);Serial.print(F("\nOlcbBasicNode\n"));
  nm.setup(&nodal, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM); 
  userInitEventTypes();
  Serial.print(F("\nP/C flags done"));
  printEventsIndex();
  printEvents();
  
  Olcb_setup();

  //Serial.print("\n\n===============================\n");
  // Unit testing
  //#define testEquals testEquals
  //#define testFindIndexInArray testEquals
  //#define testIndex_FindIndex testIndex_FindIndex
  //#define testBsearch testBsearch
  //#define testPCEEventReportf testPCEEventReport

  //#define testPCEHandleLearnEvent testPCEHandleLearnEvent
  //#define testIdentifyConsumers testIdentifyConsumers
  //#define testIdentifyProducers testIdentifyProducers
  //#define testIdentifyEvents testIdentifyEvents
  #ifdef test
    test();
  #endif

}

void loop() {
    bool activity = Olcb_loop();
    if (activity) {
        // blink blue to show that the frame was received
        Serial.print("\nrcv");
        blue.blink(0x1);
    }
    if (OpenLcb_can_active) { // set when a frame sent
        gold.blink(0x1);
        Serial.print("\nsnd");
        OpenLcb_can_active = false;
    }
    // handle the status lights  
    blue.process();
    gold.process();
   
}


