# OlcbRefresh
#### **WARNING** This code is in development.  AT90CAN appears to work.  Others not tested.  
This is a refresh of the Arduino base libs, ie OlcbStarLibraries.  

It is meant to simplify and extend the Arduino code.

## Changes: 
1. Added support for multiple processors: Arduino, Teensy, Tiva. 
   - Each set of files specific to a CAN-processor is kept in its own directory.   
   - The processor is automatically selected in the processor.h file. 
2. A sorted index[] into eventids[] is used to speed eventID processing, using qsort() and bsearch().  
3. Simplified the definition of CDI/xml for the node by matching a struct{} to the xml structure, see the example below.   

e.g.: This CDI/xml, which self-describes the node to the system:
```xml
    <cdi>
        ...
        <group replication='8'>
        <name>Channels</name>
            <eventid><name>event0</name></eventid>
            <eventid><name>event1</name></eventid>
            <int size='1'><name>Rate</name></int>
        </group>
        ...
    </cdi>
```
parallels this program structure:    
```c++
    typedef struct {
        ...
        struct {
            EventID event0;
            EventID event1;
            uint8_t rate;
        } channels[8];
        ...
    } MemStruct;
    Memstruct *pmem;
```
The system constant parts of these two items are provided, and only the user-app parts are needed in the sketch.  'pmem' is used to retrieve od=ffsets within Memstruct, and the macro EEADDR(x) returns this offset, which is useful in code to refer to and manipulate Memstruct fields.  
For example, **EEADDR(channels[0].rate)** will return the offset to that item in Memstruct, and it can be used to retrieve its value: **int r = EEPROM.get(EEADDR(channels[0].rate))**.  


## Memory Model:
Only eventIDs are copied to RAM into the eventids[] array.  <br>
#### In Flash:<br>
**EIDTab[]** holds the offset of each eventID into the EEPROM and RAM struct{}, and the associated flags indicating P/C.<br>
    It is initialized at compile-time using three macros: CEID(), PEID(), and PCEID().
    Fore example, **CEID(channels.event0)** provides the memory offset to teh event, and indicates it is a consumer-event.  
#### In RAM:
**eventidIndex[]** contains sorted index into **eventidOffset[]**.<br>
    It is sorted on the hash values.  
**event[]** contains run-time flags, used to indicate P/C, learning, and eventids to be transmitted to the net.  

In the medium model *only*, **eventids[]** contains a copy of the node's eventIDs, and is indexed by eventidIndex[].

This restated as a diagram:
```
                       +--> (index) -->eventid[].flags
        eventIndex[] --+--> (index) -->event[].flags
                       +--> (index) -->EIDTab[] --(offset)--> mem[] or EEPROM[]
```

## More about OpenLCB/LCC - what is it?

OpenLCB/LCC is a set of heirarchical protocols to **let nodes talk to each other**. <br>
For more information on OpenLCB, see: [OpenLCB.org](http://openlcb.org)<br>
For protocol descriptions, see: [Adopted LCC Documents](http://openlcb.org/openlcb-and-lcc-documents/layout-command-control-lcc/)

These protocols consist of: 
 - **Systems/Application Messaging**<br>
    These are the 'workhorse' messages on which most applications will be built, they are useful for sytems-messaging as well, and for building further systems-protocols.  
   - PCE - Produver/Consumer Event Messages
     - These are *unaddressed* EventID messages.
     - They implement *Producer/Consumer Events* (64-bit)
     - EventIDs are globally unique 64-bit numbers.
     - These are one-to-many messages.
   - Datagram Messages
     - These are *addressed* messages containing up to 70-bytes of data in a single message.
     - These are one-to-one messages.
   - Stream Messages
     - These are *addressed* messages carrying unlimited data in multiple messages.
     - These are one-to-one messages.
 - **Systems/Housekeeping**<br>
    These are the 'behind-the-scenes' protocol that enables and ensures the system's integrity. 
   - Link - establishes and maintains the node's link to the network
     - Announces state of Node
     - Announcement of *Intialization Complete*
     - Announcement of *Consumed-* and *Produced-eventIDs*
     - *NodeID reporting* on request.
     - *EventID reporting* on request.
     - On the CAN-implementation, this maintains *alias assignment and maintenance*;
   - SNIP - Simple Node Information Protocol
     - Provides a brief description of the node for *UI Tools* to use.
   - PIP - Protocol Identification Protocol
     - Defines which protocols the node uses, and it is reported as a bit-map.  
   - CDI - Configuration Description Information
     - *Reports the node's CDI/xml* on request.
   - Memory Configuration
     - Reading and writing to the node's memory spaces, including Configuration, RAM and EEPROM spaces.
 - **Additional Protocols**<br>
    These protocols extend the base-system.
   - Teaching -- teaching an eventID from one node to one or more others.  
   - Traction Control -- train control.
 - **Additional Utility-Libraries**<br>
    These libraries implement useful functionality.  
   - BG - Blue/Green -- node health indicators and system buttons.
   - ButtonLed -- implements controlling a button and LED from a single processor pin.

## How the Above Translates to the Codebase
The 'codebase' is a set of libraries and functions that implement the basic protocols of OpenLCB/LCC. <br> 
Each protocol has corresponding **code**, usually in the form of a **class**, and implemented as a pair of *.h and *.cpp  files. <br> 
The codebase tries to hide some of the complexity in #include files.  <br>

However, each protocol needs to have: 
 - **initialization**, and
 - **processing**
    
For example there are some selected lines of code from the OlcbBasicNode example used for **initialization**: 
```c++
  NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
  const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; 
  uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
  ButtonLed* buttons[] = { &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD };
  PCE pce(&nodal, &txBuffer, pceCallback, restore, &link);
  BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);
```
Most of the **processing** is hidden as functions in the #include files.  

## How Does the Application Interact with the Codebase?
The programmer of the Application must: 
 - Decide what and how the new Application works, ie how eventids and other node variables are used to build the Application.  
 - Choose the **NodeID** - this must be from **a range controlled** by the manufacturer - **ie you**.  
 - Write the **CDI/xml** describing the node and its node-variables, including its eventIDs. 
 - Write a **MemStruct{}** that matches the xml description.  
 - Define **EIDTab** to flag each eventID as a **Producer, Consumer, or Both** by using **CEID**, **PEID**, and **PCEID**.  
 - Write **pceCallback()**, which processes received eventIDs, ie eventIDs to be consumed, and causing whatever action is required, eg a LED being lit or extinguished.  
 - Write **produceFromInputs()** which scans the node's inputs and, if appropriate, flags an evenItD to be sent.  
 - Write **userConfigWrite()** which is called whenever a UI Tool writes to the node's memory.  This code can then compare the memory address range of the change to the node's variables, and take whatever action is appropriate, e.g. update a servo position.
 - Write additional support and glue code for the Application.  

## Example Applications
The provided examples will give some ideas of how to accomplish sample projects.  They can form the basis of, or  be adapted to, a new Application, or just used for inspiration.  
 - **OlcbBasicNode**<br>
    Implements a simple node which exercises most of the protocols.  It has **two inputs** and **two outputs**.  Each input has two Producer-eventIDs and each output has two Consumer-eventIDs, so 8 eventIDs in total.  This Application makes use of the ButtonLed library to control **two buttons** and **two LEDs**.  In addition, it implements the BG (Blue-Gold) protocol to allow the **teaching** of eventIDs between this node and others.  
- **OlcbServoPCA8695**<br>
    Implements driving a number of servos from a PCA8695 PWM chip.  It shows how to write a different **pceCallback()**.  It also uses **userConfigWrite()** to allow real-time updating of a servo positions from a **UI Tool**, such as **JMRI** or **Model Railroad System**.  
