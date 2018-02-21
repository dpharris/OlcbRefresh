//==============================================================
// OlcbFncNode
//   A prototype of a Function OpenLCB board
//
//   This node has: 
//     NUM_CHANNEL channels, and each has: 
//        eventid, and a 
//        actionFnc( pin, firstParameter, secondParameter)
//
//   The actions include: 
//     None
//     blink(pin, period, dutyCycle)
//     dstrobe(pin, period, dutyCycle)
//     fade( pin, targetLevel, rate)
//     servo( servo, targetPosition, rate)
//     input( pin, sense, secondEidindex)
//     sample( pin, sense, secondEidindex)        
// 
//   setup() determines which are consumers and
//   which are producers
//
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

/*
            <group replication='4'>
              <name>EventIDs</name>
              <repname>EventID</repname>
              <string size='16'><name>Description</name></string>
              <int size='1'>
                <name>Function</name>
                <default>0</default>
                <map>
                  <relation><property>0</property><value>Off</value></relation>
                  <relation><property>1</property><value>Blink(period,duty)</value></relation>
                  <relation><property>2</property><value>Dstrobe(period,duty)</value></relation>
                  <relation><property>3</property><value>Fade(lumen,rate)</value></relation>
                  <relation><property>4</property><value>Servo(target,rate)</value></relation>
                  <relation><property>5</property><value>Input(sense,debounce)</value></relation>
                  <relation><property>6</property><value>Sample(sense,debounce)</value></relation>
                </map>
              </int>
              <int size='1'>
                <name>Pin#</name>
                <default>0</default>
                <map>
                  <relation><property>0</property><value>None</value></relation>
                  <relation><property>1</property><value>pin12</value></relation>
                  <relation><property>2</property><value>pin13</value></relation>
                  <relation><property>3</property><value>pin14</value></relation>
                  <relation><property>4</property><value>pin21</value></relation>
                  <relation><property>5</property><value>pin22</value></relation>
                  <relation><property>6</property><value>pin30</value></relation>                   
                </map>
              </int>
              <int size='1'><name>Parameter1(0-255)</name>
                   <description>period,target,sense</description>
              </int>
              <int size='1'><name>Parameter2(0-255)</name></int>
              <eventid><name>EventID</name></eventid>
            </group>

*/




//#include "debug.h"
#include "OlcbCommonVersion.h"

// Description of EEPROM memory structure, and the mirrored mem if in MEM_LARGE
#include "MemStruct.h"

// The following lines are needed because the Arduino environment 
// won't search a library directory unless the library is included 
// from the top level file (this file) [[ this may have changed with more recent IDEs ]]
//#include <EEPROM.h>

//#include <can.h>
//#include <ButtonLed.h>

//#include "NodeID.h"

// init for serial communications if used
#define         BAUD_RATE       115200

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

////// ACTIONS //////////////////////////////////////////////////
uint8_t pinMap[] = { 3,4,5,6 };
void setPin(uint16_t pin, uint16_t level){
  Serial.print("\nsetPin: ");
  Serial.print(level,HEX);
}
void blink(uint16_t pin, uint16_t period, uint16_t duty) {
  static long next = 0;
  static bool off = true;
  static uint16_t offPeriod;
  static uint16_t onPeriod;
  if(millis()<next) return;
  if(next==0) {
    setPin(pinMap[pin], 0);
    onPeriod = (period * duty)/100;
    offPeriod = period - onPeriod;
    next += offPeriod;
    return;
  }
  if(off) { 
    next += offPeriod;
    setPin(pin, 0);
  } else {
    next += onPeriod;
    setPin(pinMap[pin], 0xFFFF);
  }
}
void dstrobe(uint16_t pin, uint16_t period, uint16_t duty) {
  static long next = 0;
  static uint8_t state;
  static uint16_t offPeriod;
  static uint16_t offPeriod2;
  static uint16_t onPeriod;
  if(millis()<next) return;
  if(next==0) {
    setPin(pin, 0);
    onPeriod = (period * duty)/100;
    offPeriod = period - onPeriod*10;
    offPeriod = onPeriod * 8;
    state = 0;
    next += offPeriod;
    return;
  }
  switch (state) { 
  case 0: // just finished off
    next += onPeriod;
    setPin(pinMap[pin], 0xFFFF);
    state++;
    break;
  case 1: // finished first strobe
    next += offPeriod2;
    setPin(pinMap[pin], 0);
    state++;
    break;
  case 2: // finished mid strobes
    next += onPeriod;
    setPin(pinMap[pin], 0xFFFF);
    state++;
    break;
  case 4: // finished second strobe
    next += offPeriod;
    setPin(pinMap[pin], 0);
    state = 0;
    break;
  }
}

void fade(uint16_t pin, uint16_t target, uint16_t rate) {
  static long next = 0;
  static float level = 0;
  #define fudge 0.001
  if(millis()<next) return;
  level = (target-level) * rate / fudge;
  uint16_t r = level+0.01;
  setPin(pinMap[pin],r);
}
void setServo(uint16_t servo, uint16_t posn) {
  Serial.print("\nsetServo: ");
  Serial.print(servo,HEX);
  Serial.print(" posn: ");
  Serial.print(posn,HEX);
}
void servo(uint16_t serv, uint16_t target, uint16_t rate) {
  static long next = 0;
  static float posn = 0;
  static bool off = true;
  #define fudge 100.0
  if(millis()<next) return;
  if(target>1800) target = 1800;
  posn = (target-posn) * rate / fudge;
  uint16_t r = posn+0.01;
  setServo(serv,r);  
}
void sendEvent(uint16_t index) {
  Serial.print("\nsendEvent: ");
  Serial.print(index,HEX);
}
void input(uint16_t pin, uint16_t sense, uint16_t secondEventID) {
  #define PERIOD 200
  static long next = 0;
  bool pinState = false;
  if(millis()<next) return;
  if(next==0) {
    pinMode(pinMap[pin], INPUT);
  }
  bool r = digitalRead(pin);
  if(r!=pinState) {
    if(r) sendEvent(sense?pin:secondEventID);
    else  sendEvent(sense?secondEventID:pin);
    pinState = r;
  }
  next += PERIOD;
}
void sample(uint16_t pin, uint16_t sense, uint16_t secondEventID) {
  #define PERIOD 200
  static long next = 0;
  bool pinState = false;
  if(millis()<next) return;
  pinMode(pinMap[pin], INPUT);
  delay(1); // settling time
  bool r = digitalRead(pin);
  if(r!=pinState) {
    if(r) sendEvent(sense?pin:secondEventID);
    else  sendEvent(sense?secondEventID:pin);
    pinState = r;
  }
  next += PERIOD;
}

void randm(uint16_t pin, uint16_t max, uint16_t period) {
  static long next = 0;
  setPin(pin, random(max));
  next += period;
}

Channel channel[NUM_EVENT];
void fncProcess() {
  static uint8_t i = 0;
  Channel c = channel[i];
  if(c.action!=0) action[c.action](c.pin, c.parm0, c.parm1);
  i++;
  if(i>NUM_CHANNEL) i=0;        
}

uint16_t channelState[NUM_CHANNEL];
void f_set(uint16_t pin, uint16_t index, uint16_t value) {
  channelState[index] = value;
}
void f_and(uint16_t pin, uint16_t index, uint16_t value) {
  channelState[index] &= value;
}
void f_or(uint16_t pin, uint16_t index, uint16_t value) {
  channelState[index] |= value;
}
void f_if(uint16_t pin, uint16_t index, uint16_t eidIndex) {
  if(channelState[index]) sendEvent(eidIndex);
}

//  1 eid0 set 1 1
//  2 eid1 and 1 1
//  3 eid2 if  1 4
//  4 eid3 none
//

// when pin-event consumed, delay period^2 and then send another eventID
void f_delay(uint16_t pin, uint16_t period, uint16_t eidi) {
  static long next = 0;
  if(next==0) {
    //next += period;
    next += period * (unsigned long)period;
    //next += scaledResult = fscale( 0, 66000.0, 0, 1E9, period, 0);
  } else if(millis>next) {
    sendEvent(eidi);
    next = 0;
  }
}

// is it possible to chain the fncs?

/////////////////////////////////////////////////////////////////



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
  delay(250);Serial.begin(BAUD_RATE);Serial.print(F("\nOlcbBasicNode\n"));
  Serial.print(F("\nMemModel=")); Serial.print(MEM_MODEL);
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state
  //nm.setup(&nodeid, events, eventidOffset, NUM_EVENT, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM); 
  nm.setup(&nodal, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM); 
//  Serial << F("Hi there") << _HEX(1234) << "\n";

#ifdef DUMMY    
    char s0[10] = "Test Node";
    for(int j=0; j<sizeof(s0); j++) writeByte((uint16_t)&pmem->nodeName+j, s0[j]);
    char s1[20] = "Just the facts Mam";
    for(int j=0; j<sizeof(s1); j++) writeByte((uint16_t)&pmem->nodeDesc+j, s1[j]);
    char s3[8] = "input";
    char s4[8] = "output";
    
    for(int i=0;i<2;i++) {
      for(int j=0; j<sizeof(s3); j++) writeByte((uint16_t)&pmem->inputs[i].desc+j, s3[j]);
      for(int j=0; j<sizeof(s4); j++) writeByte((uint16_t)&pmem->outputs[i].desc+j, s4[j]);
    }
#endif   

  EEPROM.get(ADDR_EID(channel), channel);
  
  // set event types, now that IDs have been loaded from configuration
  // newEvent arguments are (event index, producer?, consumer?)
  for (int i=0; i<NUM_CHANNEL; i++) {
      if(consumerFnc[channel[i].action]) pce.newEvent(i,false,true); // consumer
      else pce.newEvent(i,true,false); // producer
  }

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

    fncProcess();
   
}


