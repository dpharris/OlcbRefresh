# OlcbBasicNode
*This example is temporarily named DPH-OldBasicNode.*

This node is designed to demonstrate how to use Consumer and Producer eventIDs.

It implements two inputs and two outputs. Each inpit and output pin has a LED and push-button attached to it, such that driving the pin low will illuminate the LED, and letting the pin float as an input allows the button state, pushed or released, be sensed.  <br>
    [[I will include a diagram]]

### CDI/xml
The CDI/xml describes the variables and eventIDs that are used by a GUI-Tool, so that it can display them in a useful way.  See: [XML Wikipedia](https://en.wikipedia.org/wiki/XML)

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
This is pretty basic but it is not descriptive.  So additional descriptive text can be added in order to give names to, and describe, the variables. In addition, for each input and output, we can add a text node-variable that saves a description of the input/output inthe EEPROM.  This looks like:
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
That is essentially the CDI/xml.  However, every node also has some system-variables stored in its EEPROM.  These are stored at a known location, so the system code can find them.  So, we need to add some preface-xml:
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

And we also need some footer-xml.  This describes the system variables, and let's the user reset the node from the GUI-Tool.  At this point, just leave it alone and do not edit it. 
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
The actual xml is coded in the sketch as follows:
```
const char configDefInfo[] PROGMEM = R"(
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
```
### Matching MemStruct
The matching C++ struct{} MemStruct consists of some fixed system variables, and node variables matching the xml.  It is coded as: 
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

### Stepping through the code

```C++
#define MEM_SMALL 1
#define MEM_MEDIUM 2
#define MEM_LARGE 3
#define MEM_MODEL MEM_SMALL    // small but slow, works out of EEPROM
//#define MEM_MODEL MEM_MEDIUM   // faster, eventIDs are copied to RAM
//#define MEM_MODEL MEM_LARGE    // fastest but large, EEPROM is mirrored to RAM 
```
These #defines let you choose the Memory Model.  Just uncomment one of the last three #defines to choose the model.  
```C++
// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 4
// total number of events, two per channel
#define NUM_EVENT 2*NUM_CHANNEL
```
These lines determine how many channels and eventIDs this node uses.  In this case, there are four channels: two inputs and two outputs.  Each channel is either 'on' or 'off' and each state-transition has an associated eventID, so there are twice as many eventIDs as there are channels.  
```C++
#include "MemStruct.h"
```
This line #includes the struct{} defintion of the format of the EEPROM, and, if in LARGE-mode, the mirrored copy of it in RAM.  
```C++
NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
```
This defines the node's ID.  It must be *changed* to one that *you control or own*.  
```C++
// Define pins
// BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define BLUE 18

// GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
#define GOLD 19
```
This example node implements two status indicators, one blue, and the other gold.  The blue LED flashes when the node receives a message, and the gold LED flashes when this noe sends a message.  This example also implements Blue and Gold buttons which can be used to Teach eventIDs to other nodes, or to reset theis node.  
```c++
// next lines get "warning: only initialized variables can be placed into program memory area" due to GCC bug
extern "C" {

// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
#include "cdi.h"

// SNIP Short node description for use by the Simple Node Information Protocol 
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; // last zero in double-quote

}; // end extern "C"
```
These lines include the CDI/xml dscription of the node's EEPROM in a form that is useful to a GUI-Tool.  The second part defines a string that has a shortened desription of the node that includes its type, and versions of its hardware and software.  It is requested by the Simple Node Information protocol, and the string is sent in reply.  This lets the GUI-Tool make a list of the available nodes online.  


