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

#define DEBUG

// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 10
// total number of events, two per channel
#define NUM_EVENT NUM_CHANNEL

#include "OpenLCBHeader.h"
#include "AT90can.h"

//NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
NodeID nodeid(0x12,0x23,0x34,0x45,0x56,0x67);    // Easy to find in eeprom listings 

// ===== CDI =====
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
    extern "C" { 
        const char configDefInfo[] PROGMEM = 
        //const char configDefInfo[]  = 
          // ===== Enter User definitions below CDIheader line =====
          CDIheader R"(
            <group replication='4'>
              <name>EventIDs</name>
              <repname>EventID</repname>
              <string size='16'><name>Description</name></string>
              <int size='2'>
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
              <int size='2'>
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
              <int size='2'><name>Parameter1(0-255)</name>
                   <description>period,target,sense</description>
              </int>
              <int size='2'><name>Parameter2(0-255)</name></int>
              <eventid><name>EventID</name></eventid>
            </group>
          )" CDIfooter;
          // ===== Enter User definitions above CDIfooter line =====
    }

typedef struct {
    uint8_t   desc[16];
    uint16_t   pin;
    uint16_t   fnc;
    uint16_t   parm[3];
    EventID   evt;
} Channel;
Channel channel[NUM_CHANNEL];

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
    typedef struct { 
      NodeVar nodeVar;         // must remain
      // ===== Enter User definitions below =====
          Channel chl[NUM_CHANNEL];
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
      PCEID(chl[0].evt), PCEID(chl[1].evt), PCEID(chl[2].evt), PCEID(chl[3].evt), PCEID(chl[4].evt),
      PCEID(chl[5].evt), PCEID(chl[6].evt), PCEID(chl[7].evt), PCEID(chl[8].evt), PCEID(chl[9].evt),
    };

 // SNIP Short node description for use by the Simple Node Information Protocol 
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote
//const char SNII_const_data[] PROGMEM = "\001OpenLCB\000OlcbFncNode\0001.0\000" OlcbCommonVersion ;
}; // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {   //0xD7,0x58,0x00,0,0,0};
        pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent    | pTeach     | !pStream   | !pReservation, // 1st byte
        pACDI   | pSNIP     | pCDI       | !pRemote  | !pDisplay  | !pTraction | !pFunction | !pDCC        , // 2nd byte
        0, 0, 0, 0                                                                                           // remaining 4 bytes
};
      // PIP, Datagram, MemConfig, P/C, ident, teach/learn, 
      // ACDI, SNIP, CDI

// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 18
// GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
#define GOLD 19
   ButtonLed blue(BLUE, LOW);
   ButtonLed gold(GOLD, LOW);
   uint32_t patterns[] = {}; // two per cchannel, one per event
   ButtonLed* buttons[] = {};  

void pceCallback(uint16_t index){
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events.  
  // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
  // The pattern is mostly one way, blinking the other, hence inverse.
  //
  //Serial.print(F("\npceCallback()")); Serial.print(index);
  buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
}

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


void userInitAll(){}
void userSoftReset() {}
void userHardReset() {}
//
// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 
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

////// ACTIONS //////////////////////////////////////////////////
uint8_t pinMap[] = { 3,4,5,6 };
void setPin(uint16_t pin, uint16_t level){
  Serial.print("\nsetPin: ");
  Serial.print(level,HEX);
}
void blink(uint16_t p, uint16_t period, uint16_t duty) {
  static long next = 0;
  static bool off = true;
  static uint8_t pin = 0;
  static uint16_t offPeriod;
  static uint16_t onPeriod;
  if(millis()<next) return;
  if(next==0) {
    pin = pinMap[p];
    setPin(pin, 0);
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
    setPin(pin, 0xFFFF);
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
  static const float fudge = 0.001;
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
  static const float fudge = 100.0;
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

typedef void (Action)(uint16_t pin, uint16_t p0, uint16_t p1);
Action* action[7] = {
  0, blink, dstrobe, fade, servo, input, sample,
};

void fncProcess() {
  static uint8_t i = 0;
  Channel c = channel[i];
  if(c.fnc!=0) action[c.fnc](c.pin, c.parm[0], c.parm[1]);
  if(++i>NUM_CHANNEL) i=0;        
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
  } else if(millis()>next) {
    sendEvent(eidi);
    next = 0;
  }
}

// is it possible to chain the fncs?

/////////////////////////////////////////////////////////////////
#include "OpenLCBMid.h"

void setup()
{

  #ifdef DEBUG
    // set up serial comm; may not be space for this!
    while(!Serial){}
    delay(250);Serial.begin(115200);dP(F("\nOlcbBasicNode\n"));
  #endif
  
  //nm.forceInitAll(); // uncomment if need to go back to initial EEPROM state

  EEPROM.get(EEADDR(chl), channel);
  
  // set event types, now that IDs have been loaded from configuration
  // newEvent arguments are (event index, producer?, consumer?)
  for (int i=0; i<NUM_CHANNEL; i++) {
      if(channel[i].fnc<5) pce.newEvent(i,false,true); // consumer
      else pce.newEvent(i,true,false); // producer
  }

  Serial.print(F("\nP/C flags done"));
  //printEventsIndex();
  //printEvents();
  
  Olcb_init();
}

void loop() {
    bool activity = Olcb_process();
    if (activity) {
        // blink blue to show that the frame was received
        Serial.print("\nrcv");
        blue.blink(0x1);
    }
    if (olcbcanTx.active) { // set when a frame sent
        gold.blink(0x1);
        Serial.print("\nsnd");
        olcbcanTx.active = false;
    }
    // handle the status lights  
    blue.process();
    gold.process();

    fncProcess();
}


