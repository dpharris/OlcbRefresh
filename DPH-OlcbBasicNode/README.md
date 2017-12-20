# OlcbBasicNode
*This example is temporarily named DPH-OldBasicNode.*

This node is designed to demonstrate using Consumer and Producer eventIDs.

It implements two inputs and two outputs, and is designed to have hardware attached.  

Each inpit and output pin has a LED and push-button attached to it, such that driving the pin low will illuminate the LED, and letting the pin float as an input allows the button state, pushed or released, be sensed.  <br>
    [[I will include a diagram]]

### CDI/xml
The CDI/xml describes the variables and eventIDs that are used by a UI-Tool, so that it can display them in a useful way.  See: (XML Wikipedia)[https://en.wikipedia.org/wiki/XML]

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
This is pretty basic and is not descriptive.  Therefore, additional descriptive text can be added to name and describe the variables. In addition, for each input and output, we can add a node variable that contains a description of the input/output.  This looks like:
```
    <group>
        <name>I/O Events</name>                                     -- header name
        <description>Define events associated with input and output pins</description>
        <group replication='2'>                                     -- two inputs
        <name>Inputs</name>                                         -- Group name
        <repname>Input</repname>                                    -- Name labelling each input, with a # added
        <string size='16'><name>Description</name></string>         -- Node-variable description
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



