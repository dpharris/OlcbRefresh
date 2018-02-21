//==============================================================
// TCH Tech Consumer Board
//   32 channels of outputs, 2 eventIDs per channel
// 
//
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//   Tim Hatch
//   David Harris
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
#define NUM_CHANNEL 32
// total number of events, two per channel
#define NUM_EVENT 2*NUM_CHANNEL
//#define LAST_EEPROM 12+1316+8*sizeof(Event)

//#include "debug.h"
#include "OpenLCBHeader.h"
//#include "mockCan.h"
#include "AT90can.h"

//NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
//NodeID nodeid(0x02,0x00,0x36,0x01,0x12,0x02);    // Easy to find in eeprom listings 
NodeID nodeid(0x66,0x55,0x44,0x33,0x22,0x11);    // Easy to find in eeprom listings 

// ===== CDI =====
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
    extern "C" { 
        const char configDefInfo[] PROGMEM = 
        //const char configDefInfo[]  = 
          // ===== Enter User definitions below CDIheader line =====
          CDIheader R"(
            <group>
              <name>Input Events</name>
              <description>Define events associated with input pins</description>
              <group replication='32'>
                <name>Outputs</name>
                <repname>Output</repname>
                <string size='16'><name>Description</name></string>
                <eventid><name>Activation Event</name></eventid>
                <eventid><name>Inactivation Event</name></eventid>
              </group>
            </group>
          )" CDIfooter;
          // ===== Enter User definitions above CDIfooter line =====
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
            } inputs[32];            // 2 inputs
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
       const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        CEID(inputs[0].activation), CEID(inputs[0].inactivation),
        CEID(inputs[1].activation), CEID(inputs[1].inactivation),
        CEID(inputs[2].activation), CEID(inputs[2].inactivation),
        CEID(inputs[3].activation), CEID(inputs[3].inactivation),
        CEID(inputs[4].activation), CEID(inputs[4].inactivation),
        CEID(inputs[5].activation), CEID(inputs[5].inactivation),
        CEID(inputs[6].activation), CEID(inputs[6].inactivation),
        CEID(inputs[7].activation), CEID(inputs[7].inactivation),
        CEID(inputs[8].activation), CEID(inputs[8].inactivation),
        CEID(inputs[9].activation), CEID(inputs[9].inactivation),
        CEID(inputs[10].activation), CEID(inputs[10].inactivation),
        CEID(inputs[11].activation), CEID(inputs[11].inactivation),
        CEID(inputs[12].activation), CEID(inputs[12].inactivation),
        CEID(inputs[13].activation), CEID(inputs[13].inactivation),
        CEID(inputs[14].activation), CEID(inputs[14].inactivation),
        CEID(inputs[15].activation), CEID(inputs[15].inactivation),
        CEID(inputs[16].activation), CEID(inputs[16].inactivation),
        CEID(inputs[17].activation), CEID(inputs[17].inactivation),
        CEID(inputs[18].activation), CEID(inputs[18].inactivation),
        CEID(inputs[19].activation), CEID(inputs[19].inactivation),
        CEID(inputs[20].activation), CEID(inputs[20].inactivation),
        CEID(inputs[21].activation), CEID(inputs[21].inactivation),
        CEID(inputs[22].activation), CEID(inputs[22].inactivation),
        CEID(inputs[23].activation), CEID(inputs[23].inactivation),
        CEID(inputs[24].activation), CEID(inputs[24].inactivation),
        CEID(inputs[25].activation), CEID(inputs[25].inactivation),
        CEID(inputs[26].activation), CEID(inputs[26].inactivation),
        CEID(inputs[27].activation), CEID(inputs[27].inactivation),
        CEID(inputs[28].activation), CEID(inputs[28].inactivation),
        CEID(inputs[29].activation), CEID(inputs[29].inactivation),
        CEID(inputs[30].activation), CEID(inputs[30].inactivation),
        CEID(inputs[31].activation), CEID(inputs[31].inactivation),
      };
 
 // SNIP Short node description for use by the Simple Node Information Protocol 
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote
//const char SNII_const_data[] PROGMEM = "\001OpenLCB\000OlcbBasicNode\0001.0\000" OlcbCommonVersion ;
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

//Blink patterns
  #define ShortBlinkOn   0x00010001L
  #define ShortBlinkOff  0xFFFEFFFEL
  unsigned long patterns[NUM_EVENT] = {
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,//8
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,//16
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,//24
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,
    ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn, ShortBlinkOff,ShortBlinkOn,//32
  };

  // output drivers
  ButtonLed p2(2, HIGH); ButtonLed p3(3, HIGH); ButtonLed p4(4, HIGH); ButtonLed p5(5, HIGH);
  ButtonLed p6(6, HIGH); ButtonLed p7(7, HIGH); ButtonLed p8(8, HIGH); ButtonLed p9(9, HIGH);         //8
  ButtonLed p10(10, HIGH); ButtonLed p11(11, HIGH); ButtonLed p12(12, HIGH); ButtonLed p13(13, HIGH);
  ButtonLed p14(14, HIGH); ButtonLed p17(17, HIGH); ButtonLed p18(18, HIGH); ButtonLed p19(19, HIGH); //16
  ButtonLed p22(22, HIGH); ButtonLed p23(23, HIGH); ButtonLed p24(24, HIGH); ButtonLed p25(25, HIGH);
  ButtonLed p26(26, HIGH); ButtonLed p27(27, HIGH); ButtonLed p28(28, HIGH); ButtonLed p29(29, HIGH); //24
  ButtonLed p30(30, HIGH); ButtonLed p31(31, HIGH); ButtonLed p32(32, HIGH); ButtonLed p33(33, HIGH);
  ButtonLed p34(34, HIGH); ButtonLed p35(35, HIGH); ButtonLed p36(36, HIGH); ButtonLed p37(37, HIGH); //32

  ButtonLed* buttons[NUM_EVENT] = {
    &p2,&p2,&p3,&p3,&p4,&p4,&p5,&p5,&p6,&p6,&p7,&p7,&p8,&p8,&p9,&p9,
    &p10,&p10,&p11,&p11,&p12,&p12,&p13,&p13,&p14,&p14,&p17,&p17,&p18,&p18,&p19,&p19,
    &p22,&p22,&p23,&p23,&p24,&p24,&p25,&p25,&p26,&p26,&p27,&p27,&p28,&p28,&p29,&p29,
    &p30,&p30,&p31,&p31,&p32,&p32,&p33,&p33,&p34,&p34,&p35,&p35,&p36,&p36,&p37,&p37,
    };
// ===== Blue/Gold =====
  ButtonLed blue(42, LOW);
  ButtonLed gold(43, LOW);

// ===== Process Consumer-eventIDs =====
    void pceCallback(unsigned int index) {
      // Invoked when an event is consumed; drive pins as needed
      // from index of all events.  
      // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
      // The pattern is mostly one way, blinking the other, hence inverse.
      //
      //Serial.print(F("\npceCallback()")); Serial.print(index);
      Serial.print("\n In pceCallback index="); Serial.print(index);
      buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
    }

bool states[] = {false}; // current input states; report when changed


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
  //for (int i = 0; i<(MAX_INPUT_SCAN); i++) { // simply a counter of how many to scan
  //  if (scanIndex < (LAST_PRODUCER_CHANNEL_INDEX)) scanIndex = (FIRST_PRODUCER_CHANNEL_INDEX);
  //  if (states[scanIndex] != buttons[scanIndex*2]->state) {
  //    states[scanIndex] = buttons[scanIndex*2]->state;
  //    if (states[scanIndex]) {
  //      pce.produce(scanIndex*2);
  //    } else {
  //      pce.produce(scanIndex*2+1);
  //    }
  //  }
  //}
}

// ===== Callback from a Configuration write =====
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 
//void userConfigWrite(unsigned int address, unsigned int length) {
void configWritten(unsigned int address, unsigned int length) {
  Serial.print("\nuserConfigWrite "); Serial.print(address,HEX);
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

#include "OpenLCBMid.h"
#include "extras.h"

// ==== Setup does initial configuration =============================
void setup() {
  // set up serial comm; may not be space for this!
  while(!Serial){}
  delay(250);Serial.begin(115200);Serial.print(F("\nTCH Consumer-32\n"));

  #define FORCEALLINIT
  #ifdef FORCEALLINIT
      Serial.print("\nForced Initialization");
      nm.forceInitAll();
      // initialize descriptions
      EEPROM.put(EEADDR(nodeVar.nodeName),"TCH Consumer32");
      EEPROM.put(EEADDR(nodeVar.nodeDesc),"Testing");
      //  char s[16];
      //for(int i=0; i<32; i++) {
      //  printf(s, "output%i",i);
        //EEPROM.put(EEADDR(inputs[0].desc),s);
      //}
  #endif
  
  Olcb_init();
  printEeprom();
  //while(1){}
}

// ==== Loop ===========================================
void loop() {
    bool activity = Olcb_process();
    static long T = millis()+5000;
    if(millis()>T) {
      T+=5000;
      Serial.print("\n.");
    }
    if (activity) {
        // blink blue to show that the frame was received
        //Serial.print("\nrcv");
        blue.blink(0x1);
    }
    //if (OpenLcb_can_active) { // set when a frame sent
    if (olcbcanTx.active) { // set when a frame sent
        gold.blink(0x1);
        //Serial.print("\nsnd");
        olcbcanTx.active = false;
    }
    // handle the status lights  
    blue.process();
    gold.process();
   
}



