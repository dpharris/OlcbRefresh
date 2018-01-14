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

#define DEBUG
#ifdef DEBUG
    #define dP(x) Serial.print(x)
    #define dPH(x) Serial.print(x,HEX)
    #define dPL Serial.println()
    #define dPN(x) Serial.print("\n" #x ":")
#else
    #define dP(x) 
    #define dPH(x) 
    #define dPL 
    #define dPN(x) 
#endif

// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 4
// total number of events, two per channel
#define NUM_EVENT 2*NUM_CHANNEL

#include "OlcbCommonVersion.h"
//#include "debug.h"
//#include "mockCan.h"
#include "OpenLCBHeader.h"
#include "AT90can.h"
Can can;
//OlcbCanClass can;
// Description of EEPROM memory structure, and the mirrored mem if in MEM_LARGE

NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 

// ===== CDI =====
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
    extern "C" { 
        const char configDefInfo[] PROGMEM = 
            CDIheader R"(
          // ===== Enter User definitions below =====
            <group>
              <name>I/O Events</name>
              <description>Define events associated with input and output pins</description>
              <group replication='2'>
                <name>Inputs</name>
                <repname>Input</repname>
                <string size='16'><name>Description</name></string>
                <eventid><name>Activation Event</name></eventid>
                <eventid><name>Inactivation Event</name></eventid>
              </group>
              <group replication='2'>
                <name>Outputs</name>
                <repname>Output</repname>
                <string size='16'><name>Description</name></string>
                <eventid><name>Set Event</name></eventid>
                <eventid><name>Reset Event</name></eventid>
              </group>
            </group>
          // ===== Enter User definitions above =====
            )" CDIfooter;
    }

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
    typedef struct { 
      NodeVar nodeVar;         // must remain
      // ===== Enter User definitions below =====
            struct {
              char desc[16];        // description of this input-pin
              EventID activation;   // eventID which is Produced on activation of this input-pin 
              EventID inactivation; // eventID which is Produced on inactivation of this input-pin
            } inputs[2];            // 2 inputs
            struct {
              char desc[16];        // decription of this output
              EventID setEvent;     // Consumed eventID which sets this output-pin
              EventID resetEvent;   // Consumed eventID which resets this output-pin
            } outputs[2];           // 2 outputs
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition
    MemStruct *pmem = 0; 

extern "C" {
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
       const EIDTab eidtab[NUM_EVENT]  = {
        PEID(inputs[0].activation), PEID(inputs[0].inactivation),
        PEID(inputs[1].activation), PEID(inputs[1].inactivation),
        CEID(outputs[0].setEvent),  CEID(outputs[0].resetEvent),
        CEID(outputs[1].setEvent),  CEID(outputs[1].resetEvent),
      };
 
 // SNIP Short node description for use by the Simple Node Information Protocol 
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

}; // end extern "C"
    
// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 18
#define GOLD 19  // GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)


// Establish location of node Name and Node Decsription in memory
//#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
//#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {   //0xD7,0x58,0x00,0,0,0};
        pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent    | pTeach     | !pStream   | !pReservation, // 1st byte
        pACDI   | pSNIP     | pCDI       | !pRemote  | !pDisplay  | !pTraction | !pFunction | !pDCC        , // 2nd byte
        0, 0, 0, 0                                                                                           // remaining 4 bytes
};
      // PIP, Datagram, MemConfig, P/C, ident, teach/learn, 
      // ACDI, SNIP, CDI

// ===== Blue/Gold =====
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
ButtonLed* buttons[] = {  
  &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD // One for each event; each channel is a pair
};
ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

// ===== Process Consumer-eventIDs =====
void pceCallback(unsigned int index){
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events.  
  // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
  // The pattern is mostly one way, blinking the other, hence inverse.
  //
  //Serial.print(F("\npceCallback()")); Serial.print(index);
  Serial.print("\n In pceCallback inndex="); Serial.print(index);
  buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
}

//NodeMemory nm(0);  // allocate from start of EEPROM
//void store() { nm.store(&nodeid, events, eventidOffset, NUM_EVENT); }

//PCE pce(&nodal, &txBuffer, pceCallback, restore, &link);

// Set up Blue/Gold configuration
//BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);

bool states[] = {false, false, false, false}; // current input states; report when changed


// ===== Process changes to Configuration EEPROM =====
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

// ===== Callback from a Configuration write =====
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 
//void userConfigWrite(unsigned int address, unsigned int length) {
void configWritten(unsigned int address, unsigned int length) {
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

//void userInitEventTypes() {
//  // Set event types, now that IDs have been loaded from configuration
//  // newEvent arguments are (event index, producer?, consumer?)
//    for (int i=2*(FIRST_PRODUCER_CHANNEL_INDEX); i<2*(LAST_PRODUCER_CHANNEL_INDEX+1); i++) {
//      pce.newEvent(i,true,false); // producer
//    }
//    for (int i=2*(FIRST_CONSUMER_CHANNEL_INDEX); i<2*(LAST_CONSUMER_CHANNEL_INDEX+1); i++) {
//      pce.newEvent(i,false,true); // consumer
//    }
//}
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

#include "OpenLCBMid.h"
#include "extras.h"
/*
// Instansiate a OpenLCB object:
OpenLCB olcb( &nodeid,  // NodeID
            NUM_EVENT,  // number of events
               eidtab,  // array of offsets to eventids in MemStryct, and initial P/C flags
          //eventOffset,  // array of event offsets in MemStruct
           eventIndex,  // sorted array of indexes to events
             eventids,  // copy in ram of the array of eventids in EEPROM  
    sizeof(MemStruct),  // number of EEPROM entries to zero on reset
                 &can,  // CAN bus connection
                &link,  // Link level
                  &dg,  // Datagram
                 &str,  // Stream
                 &cfg,  // Configuration
                  &bg,  // BlueGold
            &txBuffer,  // CAN transmit buffer
            &rxBuffer,  // CAN receive buffer
          pceCallback,  // callback for received P/C Event messages
        configWritten   // callback for EEPROM writes from Tools
);
*/
/**
 * Setup does initial configuration
 */
void setup()
{
  // set up serial comm; may not be space for this!
  while(!Serial){}
  delay(250);Serial.begin(115200);Serial.print(F("\nOlcbBasicNode\n"));
//  nm.setup(&nodal, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM); 
  Olcb_init();
  /*
  printEeprom();
  printEventIndexes();
  printSortedEvents();
  OpenLcbCanBuffer buf;
  buf.setPCEventReport(&eee);
  pce.receivedFrame(&buf);
  */
// test void OpenLcbCanBuffer::setConsumerIdentified(EventID* eid) --- returns 194C4 instead of 19474
  OpenLcbCanBuffer buffer;
  EventID eee = { 11,22,333,44,55,66,77,88 };
  buffer.setConsumerIdentified(&eee);
  Serial.print("\n setConsumerIdentified()=");
  Serial.print(buffer.id,HEX);
  //for(int i=0;i<13;i++) {
  //  Serial.print(buffer[i]); Serial.print(".");
  //}
  while(1==1){}
}

void loop() {
    //bool activity = Olcb_loop();
    bool activity = Olcb_process();
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



