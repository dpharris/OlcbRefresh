//==============================================================
// Olcb 16P 16C 24BOD 16 Servo 
// 
//   setup() determines which are consumers and
//   which are producers
//
//   Alex Shepherd and David Harris 2017
//==============================================================

#include "OlcbCommonVersion.h"
#include "MemStruct.h"
#include "debug.h"
#include "eeprom_utils.h"

#define DEBUG_BAUD_RATE 115200

NodeID nodeid(0x05,0x02,0x01,0x02,0x02,0x24);    // This node's default ID; must be valid 

extern "C" {
  
 // CDI (Configuration Description Information) in xml, must match MemStruct
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
 #include "cdi.h"
 
 // SNIP Short node description for use by the Simple Node Information Protocol 
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
 const char SNII_const_data[] PROGMEM = "\001OpenLCB\000AJS-Io-16P-16C-24BoD-16Servo\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

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

#define OLCB_NO_BLUE_GOLD
#include "OlcbArduinoCAN.h"
#include "OlcbInc1.h"

Nodal_t nodal = { &nodeid, events, eventsIndex, eventidOffset, NUM_EVENTS };

const uint8_t outputPinNums[] = { 0,  1,  2,  3,  4,  5,  6,  7};

const uint8_t inputPinNums[]  = { 8,  9, 10, 11, 12, 13, 14, 15};
uint8_t inputStates[]         = { 0,  0,  0,  0,  0,  0,  0,  0}; // current input states; report when changed

const uint8_t bodPinNums[]    = {16, 17, 18, 19, 20, 21, 22, 23, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};

uint8_t BoDStates[]           = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

void pceCallback(uint16_t index){
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events.  
  // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
  // The pattern is mostly one way, blinking the other, hence inverse.
  //
  DEBUG(F("\npceCallback: Index: ")); DEBUGL(index);

  uint8_t outputIndex = index / 2;
  uint8_t outputState = index % 2;
   
  if(index < FIRST_INPUT_EVENT_INDEX)
    digitalWrite(outputPinNums[outputIndex], outputState);
    
  else if (index >= FIRST_SERVO_EVENT_INDEX)
  {
    uint8_t servoPos = outputState ? EEPROM.read(EADDR(servoOutputs[outputIndex].thrownPos)) : EEPROM.read(EADDR(servoOutputs[outputIndex].closedPos)); 
    DEBUG(F("\npceCallback: Servo Pos: ")); DEBUGL(servoPos);
  }
     
}

NodeMemory nm(0);  // allocate from start of EEPROM
void store() { nm.store(&nodeid, events, eventidOffset, NUM_EVENTS); }

PCE pce(&nodal, &txBuffer, pceCallback, restore, &link);

void produceFromInputs() {
  // called from loop(), this looks at changes in input pins and 
  // and decides which events to fire
  // with pce.produce(i);
  // The first event of each pair is sent on button down,
  // and second on button up.
  // 
  // To reduce latency, only MAX_INPUT_SCAN inputs are scanned on each loop
  //    (Should not exceed the total number of inputs, nor about 4)

  static uint8_t inputsScanIndex = 0;
  static uint8_t bodScanIndex = 0;
  
  #define MAX_INPUT_SCAN 4
  for (int i = 0; i<(MAX_INPUT_SCAN); i++)
  {

//    DEBUG("produceFromInputs: "); DEBUG(inputsScanIndex); DEBUG(" - "); DEBUGL(bodScanIndex);
    
    if(inputsScanIndex < NUM_INPUTS)
    {
      uint8_t inputVal = digitalRead( inputPinNums[inputsScanIndex]);
      if(inputStates[inputsScanIndex] != inputVal)
      {
        inputStates[inputsScanIndex] = inputVal;
        DEBUG("produceFromInputs: Input: "); DEBUG(inputsScanIndex); DEBUG(" NewValue: "); DEBUGL(inputVal);

        if(inputVal)
          pce.produce(FIRST_INPUT_EVENT_INDEX + (inputsScanIndex * 2));
        else
          pce.produce(FIRST_INPUT_EVENT_INDEX + (inputsScanIndex * 2) + 1);
      }
      inputsScanIndex++;
    }
    else if(bodScanIndex < NUM_BOD_INPUTS)
    {
      // The UART0 is on pins 32 & 33 so while the Serial object is being used we need to skip checking these pins
#ifdef ENABLE_DEBUG_PRINT
      if(bodPinNums[bodScanIndex] == 32) bodScanIndex+=2;
#endif      
      uint8_t inputVal = digitalRead( bodPinNums[bodScanIndex]);
      //Serial.print(" NewValue: "); Serial.println(inputVal);
      if(BoDStates[bodScanIndex] != inputVal)
      {
        BoDStates[bodScanIndex] = inputVal;
        DEBUG("produceFromInputs: BODInput: "); DEBUG(bodScanIndex); DEBUG(" NewValue: "); DEBUGL(inputVal);

        if(inputVal)
          pce.produce(FIRST_BOD_EVENT_INDEX + (bodScanIndex * 2));
        else
          pce.produce(FIRST_BOD_EVENT_INDEX + (bodScanIndex * 2) + 1);
      }
      bodScanIndex++;
    }
    else
    {
      inputsScanIndex = 0;
      bodScanIndex = 0;
    }
  }
}

// initialize eeprom on factory reset
void userInit(void) 
{
  DEBUGL(F("userInit: Begin"));
  char strBuffer[16];
  memset(strBuffer, 0, sizeof(strBuffer));
  for(int i=0;i<NUM_OUTPUTS;i++) 
  {
    sprintf(strBuffer, "Output %d", i);
    DEBUGL(strBuffer);
    eeprom_update_string(EADDR(digitalOutputs[i].desc), strBuffer); 
  }

  memset(strBuffer, 0, sizeof(strBuffer));
  for(int i=0;i<NUM_INPUTS;i++)
  {
    sprintf(strBuffer, "Input %d", i);
    DEBUGL(strBuffer);
    eeprom_update_string(EADDR(digitalInputs[i].desc), strBuffer); 
  }

  memset(strBuffer, 0, sizeof(strBuffer));
  for(int i=0;i<NUM_BOD_INPUTS;i++)
  {
    sprintf(strBuffer, "Block %d", i);
    DEBUGL(strBuffer);
    eeprom_update_string(EADDR(bodInputs[i].desc), strBuffer); 
  }

  memset(strBuffer, 0, sizeof(strBuffer));
  for(int i=0;i<NUM_SERVOS;i++)
  {
    sprintf(strBuffer, "Servo %d", i);
    eeprom_update_string(EADDR(servoOutputs[i].desc), strBuffer); 
    DEBUGL(strBuffer);

    EEPROM.update(EADDR(servoOutputs[i].thrownPos),90);
    EEPROM.update(EADDR(servoOutputs[i].closedPos),90);
  }
  DEBUGL(F("userInit: Begin"));
}

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 
void userConfigWrite(unsigned int address, unsigned int length){
  // example: if a servo's position changed, then update it immediately
  // uint8_t posn;
  // for(unsigned i=0; i<NCHANNEL; i++) {
  //    unsigned int pposn = &pmem->channel[i].posn; 
  //    if( (address<=pposn) && (pposn<(address+length) ) posn = EEPROM.read(pposn);
  //    servo[i].set(i,posn);
  // }
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

/**
 * Setup does initial configuration
 */
void setup()
{

#ifdef DEBUG_BAUD_RATE
  Serial.begin(DEBUG_BAUD_RATE);DEBUGL(F("\nAJS OlcbIo 16P 16C 24BoD 16Servo"));
#endif  

//  nm.forceInitAll(); userInit(); // uncomment if need to go back to initial EEPROM state
  nm.setup(&nodal, (uint8_t*) 0, (uint16_t)0, (uint16_t)LAST_EEPROM); 

    // Setup Output Pins
  for(uint8_t i = 0; i < NUM_OUTPUTS; i++)
    pinMode(outputPinNums[i], OUTPUT);

    // Setup Input Pins
  for(uint8_t i = 0; i < NUM_INPUTS; i++)
    pinMode(inputPinNums[i], INPUT_PULLUP);

    // Setup BoD Input Pins
  for(uint8_t i = 0; i < NUM_BOD_INPUTS; i++)
    pinMode(bodPinNums[i], INPUT_PULLUP);

  // set event types, now that IDs have been loaded from configuration
  // newEvent arguments are (event index, producer?, consumer?)
  for (int i = FIRST_OUTPUT_EVENT_INDEX; i < FIRST_INPUT_EVENT_INDEX; i++)
      pce.newEvent(i,false,true); // consumer

  for (int i = FIRST_INPUT_EVENT_INDEX; i < FIRST_BOD_EVENT_INDEX; i++)
      pce.newEvent(i,true,false); // producer
  
  for (int i = FIRST_BOD_EVENT_INDEX; i < FIRST_SERVO_EVENT_INDEX; i++)
      pce.newEvent(i,true,false); // producer
  
  for (int i = FIRST_SERVO_EVENT_INDEX; i < FIRST_SERVO_EVENT_INDEX + (NUM_SERVOS * 2); i++)
      pce.newEvent(i,false,true); // consumer
  
#ifdef DEBUG_BAUD_RATE
  DEBUG(F("\nP/C flags done"));
//  printEventsIndex();
//  printEvents();
#endif  

  Olcb_setup();

  //DEBUG("\n\n===============================\n");
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
//#ifdef ENABLE_DEBUG_PRINT
//  static unsigned long nxtdot = millis();
//  if(millis()>nxtdot) {
//    nxtdot+=1000;
//    DEBUG(".");
//  }
//#endif  
    
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
  blue.process();
  gold.process();
}


