# OlcbBasicNode
*This example is temporarily named DPH-OldBasicNode.*

This node is designed to demonstrate how to use Consumer and Producer eventIDs.

It implements two inputs and two outputs. Each inpit and output pin has a LED and push-button attached to it, such that driving the pin low will illuminate the LED, and letting the pin float as an input allows the button state, pushed or released, be sensed.  <br>
Here is a video that shows one such node with buttons and LEDs (the node at the bottom) [OlcbBasicNode Video](https://www.youtube.com/watch?v=D95Y2Z-4gRQ). <br>
    [[I will include a diagram]]


// Hardware is: <br>
//  +5V---R1---LED>|----pin----R2----button---gnd <br>
// where R1 is 0.5-1k, R2 is >0.5k. <br>
// Values of 500 and 1k work well for cheap LEDs.  <br>
// For high intensity LEDs, R1 should perhaps be 1k. <br>

### CDI/xml
The CDI/xml describes the variables and eventIDs that are used by a GUI-Tool, so that it can display them in a useful way.  Here is a snapshot of a GUI-Tool displaying this node's CDI/xml.  For a general xml description, see: [XML Wikipedia](https://en.wikipedia.org/wiki/XML)
![Sample GUI-Tool snsp-shot.](http://jmri.sourceforge.net/help/en/package/jmri/jmrix/openlcb/swing/networktree/FilledOutConfigWindow.png)

Since each input pin has two states, we will want two producer-eventIDs, one for each state, for each input.  Also, each output pin also has two states, and so will have two consumer-eventids each.  

To describe this we need to write the following xml:
```
        <group replication='2'>                       -- two inputs
            <eventid></eventid>                       -- first eventID
            <eventid></eventid>                       -- second eventID
        </group>
        <group replication='2'>                       -- two outputs
            <eventid></eventid>                       -- first eventID
            <eventid></eventid>                       -- second eventID
        </group>
```
This xml is pretty basic, but it is not descriptive.  Fortunately, descriptive text can be added in order to make it more 'user-friendly' by giving names to and descriptions of the variables. These are visible in the GUI-Tool.  In addition, we can add a text node-variable that is saved in the node.  All these additions look  like:
```
    <group>
        <name>I/O Events</name>                                     -- header name
        <description>Define events associated with input and output pins</description>
        <group replication='2'>                                     -- two inputs
            <name>Inputs</name>                                     -- Group name
            <repname>Input</repname>                                -- Name labelling each input, with a # added
            <string size='16'><name>Description</name></string>     -- Node-variable description
            <eventid><name>Activation Event</name></eventid>        -- Named eventID
            <eventid><name>Inactivation Event</name></eventid>      -- Named eventID
        </group>
            <group replication='2'>
            <name>Outputs</name>
            <repname>Output</repname>
            <string size='16'><name>Description</name></string>
            <eventid><name>Set Event</name></eventid>
            <eventid><name>Reset Event</name></eventid>
        </group>
    </group>
```
That is the essentially the CDI/xml desription of the App.  You can compare to the GUI-Tool snapshot above.  However, every node also has some system-variables stored in its EEPROM, which are stored at a known memory location, so the system code can find them.  So, we need to add some preface-xml:
```
<cdi>
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
```
Note that you can edit the identification entries, and the description as you want.  

And we also need some footer-xml.  This describes the system variables, and let's the user reset the node from the GUI-Tool.  At this point, just add it verbatim and do not edit it. 
```
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
</cdi>
```
The actual xml is coded in the sketch in the **cdi.h** file as follows:
```
        const char configDefInfo[] PROGMEM =  
          // ===== Enter User definitions below CDIheader line =====
          CDIheader R"(
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
          )" CDIfooter;
          // ===== Enter User definitions above CDIfooter line =====
    }
```
For more information about CDI/xml, see the Configuration Description Protocol documents: [Standard](http://openlcb.org/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf) and [TechNote](http://openlcb.org/wp-content/uploads/2016/02/TN-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf)
### Matching MemStruct
The matching C++ struct{} MemStruct consists of some fixed system variables, and node variables matching the xml.  It looks like:
```
typedef struct //__attribute__ ((packed)) 
{ 
    uint32_t magic;         // used to check eeprom status
    uint16_t nextEID;       // the next available eventID for use from this node's set
    uint8_t  nid[6];        // the nodeID
    char     nodeName[20];  // optional node-name, used by ACDI
    char     nodeDesc[24];  // optional node-description, used by ACDI
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
} MemStruct;              // type definition
```
It looks much simpler because it does not contain all the descriptive text for the GUI, but only the node-variables that are stored in the node.  The first three variables are system-variables used for node integrity, so they can not be altered.  
See: Memory Config [Standard](http://openlcb.org/wp-content/uploads/2016/02/S-9.7.4.2-MemoryConfiguration-2016-02-06.pdf) and [TechNote](http://openlcb.org/wp-content/uploads/2016/02/TN-9.7.4.2-MemoryConfiguration-2016-02-06.pdf)

Coded in the sketch it uses a header, likr this:
```
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
```
Th3 variable **pmem** is useful for referring to to fields in the CDI, for example: **&pmem->inputs[1].desc** returns the offest into Memstruct for that **desc** varible.  The macro **EEADDR(X)** simplifies this: **EEADDR(inputs[1].desc)**, and can be used, for example: **EEPROM.get(inputs[1].desc, array)** can be used to retrieve that field.  

### Stepping Through the Sketch's Code
 
```C++
// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 4
// total number of events, two per channel
#define NUM_EVENT 2*NUM_CHANNEL
```
These lines determine how many channels and eventIDs this node uses.  In this case, there are four channels: two inputs and two outputs.  Each channel is either 'on' or 'off' and each state-transition has an associated eventID, so there are twice as many eventIDs as there are channels.  
 
```C++
NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
```
This defines the node's ID.  It must be *changed* to one that *you control or own*.  
```
// ===== CDI =====
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
    extern "C" { 
        const char configDefInfo[] PROGMEM = 
          // ===== Enter User definitions below CDIheader line =====
          CDIheader R"(
            <group>
              ...
            </group>
          )" CDIfooter;
          // ===== Enter User definitions above CDIfooter line =====
    }
```
Defines the CDI, as described above.  
```
// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
    typedef struct { 
      NodeVar nodeVar;         // must remain
      // ===== Enter User definitions below =====
            struct {
              ...
            } outputs[2];           // 2 outputs
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition
    MemStruct *pmem = 0; 
```
This defines the memory structure that matches the CDI, as described above.  
```C++
// SNIP Short node description for use by the Simple Node Information Protocol 
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote
}; // end extern "C"
```
These lines define a string that has a short-hand description of the node that includes its type and versions of its hardware and software.  It is requested by the GUI-Tool using the Simple Node Information protocol, and this string is sent in reply.  This allows the GUI-Tool to build a list of which nodes are on the bus.  <br>
See: SNIP [Standard](http://openlcb.org/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf) and [TechNote](http://openlcb.org/wp-content/uploads/2016/02/TN-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf)
```C++
// Establish location of node Name and Node Decsription in memory
#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc
```
These lines establish the address in EEPROM of the node's name, and the node's description.  
```C++
// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
        // PIP, Datagram, MemConfig, P/C, ident, teach/learn, 
        // ACDI, SNIP, CDI <br.
```
This line defines which protocol that this example node uses, these are listed in the commented lines below.  
See: PIP Section 3.3.6 in [Standard](http://openlcb.org/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf) and [TechNote](http://openlcb.org/wp-content/uploads/2016/02/TN-9.7.3-MessageNetwork-2016-02-06.pdf)
```C++
ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);
```
These lines instansiate two buttons associated with the, previously defined, Blue and Gold pins. 
```C++
ButtonLed pA(0, LOW); 
ButtonLed pB(1, LOW);
ButtonLed pC(8, LOW);
ButtonLed pD(9, LOW);
```
ButtoLed is a library which performs button debouncing and flashing of a LED on a single pin.  The parameters define the Arduino pin-number and the sense of the pin.  Here we are defining the two inputs on pins 0 and 1and outputs on pins 8 and 9. The buttons are "on" when the pin is low, ie when the pin is grounded.  
```C++
#define ShortBlinkOn   0x00010001L
#define ShortBlinkOff  0xFFFEFFFEL
```
The flashing of a LED is based on a 32-bit pattern, and is accomplished by stepping through the pattern.  These two patterns are defining a strobe-effect, and a reverse strobe-effect.  
```C++
uint32_t patterns[] = { // two per cchannel, one per event
ShortBlinkOff,ShortBlinkOn,
ShortBlinkOff,ShortBlinkOn,
ShortBlinkOff,ShortBlinkOn,
ShortBlinkOff,ShortBlinkOn
};
```
This array associates the flashing patterns to each of the node's eight eventIDs.  
```C++
ButtonLed* buttons[] = {  // One for each event; each channel is a pair
&pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD
};
```
This array associates a button to each of the node's eight eventIDs. 

```
This example node implements two status indicators, one blue, and the other gold.  The blue LED flashes when the node receives a message, and the gold LED flashes when this noe sends a message.  This example also implements Blue and Gold buttons which can be used to Teach eventIDs to other nodes, or to reset theis node.  

```C++
void pceCallback(uint16_t index){
// Invoked when an event is consumed; drive pins as needed
// from index of all events.  
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
// The pattern is mostly one way, blinking the other, hence inverse.
//
//Serial.print(F("\npceCallback()")); Serial.print(index);
buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );
}
```
This is the definition of the pceCallback() which is called when one of the node's eventIDs is received by the node.  The eight eventIDs are indexed, or numbered sequentially, from 0 to 7, and the received eventID's index is supplied as the paramter.  This routine has to be defined by the application developer.  In this case the code determines which of the buttons[] is involved  and applies the approprite pattern is applied, depending on the which eventID was received.  'patterns[index]&0x1' determines which of the pair of 'on' or 'off' was received.  

```C++
bool states[] = {false, false, false, false}; // current input states; report when changed
```
This line initializes the states of the four I/O pins.  
```C++
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
```
This is a user-defined routine.  Its job is to scan any triggers that would cause the node to send a Producer-eventID.  In this case it scans the input pins, but generally it might also scan its clock, calculate logic, or scan other node peripherals.  <br>
This particualr routine scans each input pin, and if any change happens, it flags the appropriate eventid to be send onto the bus.  It has addition code to limit how many inputs it scans every timme it is called, so that it does not excute for too long and block other code from running.  
```C++
//
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
```
This is another user-define routine.  It is not actually used in this sketch, but it is called whenever the EEPROM is changed by a GUI-Tool.  It is useful when you want the node to respond immediately to such a change, rather than waiting for it to take effect at the end of an editting seesion on the GUI-Tool. The commented code shows a theoretical example where a servo position can be updated in real time, by updating its position from a node variable (channel.posn) immediately on each change.   
```
#include "OpenLCBMid.h"
```
This file includes much of the 'behind the scenes' code for setting up structures.  

```C++
// ==== Setup does initial configuration =============================
void setup()
{
    // set up serial comm; may not be space for this!
    //delay(250);Serial.begin(BAUD_RATE);Serial.print(F("\nOlcbBasicNode\n"));
    Serial.print(F("\nMemModel=")); Serial.print(MEM_MODEL);
    
    //nm.forceInitAll();

    Olcb_init();
}
```
This is the node setup().  **nm.forceInitAll();** will rset the node ot factory settings.  System internals are initialized by calling Olcb_setup().  
```C+
// ==== Loop ===========================================
void loop() {
    bool activity = Olcb_process();
    if (activity) {
        // blink blue to show that the frame was received
        blue.blink(0x1);
    }
    //if (OpenLcb_can_active) { // set when a frame sent
    if (olcbcanTx.active) { // set when a frame sent
        gold.blink(0x1);
        olcbcanTx.active = false;
    }
    // handle the status lights  
    blue.process();
    gold.process();
}
```
The other mandatory routine, loop(), where the system internals are processed by calling Olcb_loop(), which returns an indication of received bus activity.  This is displayed on the Blue LED.  Any outgping activity is similarly displayed on the Gold LED.  The gold.blimk() call implements a 'keep-alive' blink on the Gold LED.  And finally, the Blue and Gold buttons are processed.  
<br>
The End.  
