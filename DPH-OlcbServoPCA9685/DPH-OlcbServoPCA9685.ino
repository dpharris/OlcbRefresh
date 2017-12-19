

//==============================================================
// OlcbServoPCA9685
//   A prototype of a basic 4-channel OpenLCB board
// 
//   setup() determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

#define MEM_SMALL 1
#define MEM_MEDIUM 2
#define MEM_LARGE 3
#define MEM_MODEL MEM_SMALL      // 18892 1207  147 ms          // default to small
//#define MEM_MODEL MEM_MEDIUM   // 19098 1329  121 ms
//#define MEM_MODEL MEM_LARGE    // 19090 1419  130 ms


// next line for stand-alone compile
#include <Arduino.h>
#include <avr/pgmspace.h>
//#include "logging.h"
#include "OlcbCommonVersion.h"

// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 16

// total number of events, two per channel
#define NUM_EVENT 3*NUM_CHANNEL

// Description of EEPROM memory structure, and the mirrored mem if in MEM_LARGE
#include "MemStruct.h"

// init for serial communications if used
#define         BAUD_RATE       115200

NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 

// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 48
// GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
#define GOLD 49

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

// Events this node can produce or consume, used by PCE and loaded from EEPROM by NM
Event events[NUM_EVENT] = { Event() }; 

//Nodal_t nodal = { {5,1,1,1,3,255}, events, eventsIndex, eventidOffset, NUM_EVENT };
Nodal_t nodal = { &nodeid, events, eventsIndex, eventidOffset, NUM_EVENT };

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

void pceCallback(uint16_t index);
void restore();

NodeMemory nm(0);  // allocate from start of EEPROM
void store() { nm.store(&nodeid, events, eventidOffset, NUM_EVENT); }

//PCE pce(events, NUM_EVENT, &txBuffer, &nodeid, pceCallback, store, &link);
//PCE pce(events, NUM_EVENT, eventsIndex, &txBuffer, &nodeid, pceCallback, restore, &link);
PCE pce(&nodal, &txBuffer, pceCallback, restore, &link);

// Set up Blue/Gold configuration
BG bg(&pce, 0, 0, 0, &blue, &gold, &txBuffer);

bool states[] = {false, false, false, false}; // current input states; report when changed

// This is called to process actions --> producer events -- none in this application
void produceFromInputs() {}

//
// Callback from a Configuration write
// Use this to detect changes in the node's configuration
// This may be useful to take immediate action on a change.
// 
void userConfigWrite(unsigned int address, unsigned int length){
   uint8_t pl;  // pulse length
   for(unsigned s=0; s<NUM_CHANNEL; s++) {
     for(unsigned p=0; p<3; p++) {
       unsigned int pp = &pmem->servo[s].pulse[p]; 
       if( (address<=pp) && (pp<(address+length)) ) pl = EEPROM.read(pp);
       setServoPulse(s, ((double)pl)/1000);
     }
   }
}

#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 60;   // 60 Hz
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000;
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}

void pceCallback(uint16_t index) {
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events. 
  int p = index%3;  // position 0-2
  int s = index/3;  // servo 0-7
  uint8_t b;
  EEPROM.get((int)&pmem->servo[s].pos[p]+1,b);
  int pl = EEPROM.get((int)&pmem->servo[s].pos[p],b);
  pl = (pl<<8) + b;
  setServoPulse(s, ((double)pl)/1000);
}

/**
 * Setup does initial configuration
 */
void setup()
{
  // set up serial comm; may not be space for this!
  delay(250);Serial.begin(BAUD_RATE);Serial.print(F("\nOlcbServoPCA9685\n"));
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  nm.setup(&nodal, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM); 
  
  // set event types, now that IDs have been loaded from configuration
  // newEvent arguments are (event index, producer?, consumer?)
  for (int i=0; i<8; i++) {
      pce.newEvent(i, false, true); // producer
  }
  
  Olcb_setup();
    
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  yield();
  
}

void loop() {    
    bool activity = Olcb_loop();
    if (activity) {
        // blink blue to show that the frame was received
        blue.blink(0x1);
    }
    if (OpenLcb_can_active) { // set when a frame sent
        gold.blink(0x1);
        OpenLcb_can_active = false;
    }
    // handle the status lights  
//    blue.process();
//    gold.process();

}


