# OlcbRefresh
This is a refresh of the Arduino base libs, ie OlcbStarLibraries.  

It is meant to simplify and extend the Arduino code.

## Changes: 
1. Added support for multiple processors: Arduino, Teensy, Tiva. 
   - Each set of files specific to a CAN-processor is kept in its own directory.   
   - The processor is automatically selected in the processor.h file. 
2. A sorted Index[] is used to speed eventID processing, using a hash of the eventid.  
3. Simplified the definition of CDI/xml for the node by matching a struct{} to the xml structure, see the example below.   

e.g.: CDI/xml:
```xml
    <cdi>
        <group replication='8'>
        <name>Channels</name>
            <eventid><name>evento</name></eventid>
            <eventid><name>event1</name></eventid>
        </group>
    </cdi>
```
parallels this structure:    
```c++
    typedef struct {
        struct {
            EventID event0;
            EventID event1;
        } channels[8];
    } MemStruct;
```

## Memory Models:
1. Small: All operations are from EEPROM;
2. Medium: eventIDs are copied to RAM as eventids[];
3. Large:  The whole of EEPROM is mirrored to RAM as mem[].
#### In RAM:
- eventidOffset[] stores the offset of each eventID into the EEPROM or RAM struct{}.
- eventidIndex[] contains a hash of each eventID, and an associated sequential index into eventidIndex[], it is sorted on the hash values.  
- in the medium model, eventids[] contains a copy of the eventIDs, and is indexed by eventidIndex[].
- In all models: 
```
        eventidIndex[]--(index)-->eventidOffset[]--(offset)-->mem[] or EEPROM[]
        eventidIndex[]--(index)-->Events[].flags
```
- In the Medium model, eventIndex also indexes the eventIDs array:
```
        **eventidIndex[]--(index)-->eventids[]
```

## More about OpenLCB/LCC

OpenLCB/LCC is a set of heirarchical protocols to let nodes talk to each other.  

These consist of: 
 - Systems/Application Messaging<br>
    These are the 'workhorse' messages on which most applications will be built, they are useful for sytems-messaging as well, and for building further systems-protocols.  
   - PCE - Event Messages
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
 - Systems/Housekeeping<br>
    These are the 'behind-the-scenes' protocol that enables and ensures the system's construction. 
   - Link - establishes and maintains the node's link to the network
     - Announces state of Node
     - Announcement of *Intialization Complete*
     - Announcement of *Consumed-* and *Produced-eventIDs*
     - *NodeID reporting* on request.
     - *EventID reporting* on request.
     - On the CAN-implementation, this maintains *alias assignment and maintenance*;
   - SNII
     - Simple Node Information -- provides a brief description of the node for *UI Tools* to use.
   - PIP
     - Protocol Identification -- Defines which protocols the node uses, and it is reported as a bit-map.  
   - CDI
     - *Reporting of the node's CDI/xml* on request.
   - Memory Configuration
     - Reading and writing to the node's memory spaces, including Configuration, RAM and EEPROM spaces.
 - Additional Protocols<br>
    These protocols extend the base-system.
   - Teaching -- teaching an eventID from one node to one or more others.  
   - Traction Control -- train control.
 - Additional Utility-Libraries<br>
    These libraries implement useful functionality.  
   - BG - Blue/Green -- node health indicators and system buttons.
   - ButtonLed -- implements controlling a button and LED from a single processor pin.

## How the Above Translates to the Codebase
The 'codebase' is a set of libraries and functions that implement the basic protocols of OpenLCB/LCC. <br> 
Each protocol has corresponding code, usually in the form of a class, and implenented as a pair of *.h and *.cpp  files. <br> 
The codebase tries to hide some of the complexity in #include files.  <br>

However, each protocol needs to have: 
 - **initialization**, and
 - **processing**
    
For example there are lines of code from the OlcbBasicNode example for **initialization**: 
```c++
  NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
  const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; 
  uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
  ButtonLed* buttons[] = { &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD };
  PCE pce(&nodal, &txBuffer, pceCallback, restore, &link);
  BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);
```
Most of the **processing** is hidden in the #include files.  

## How Does the Application Interact with the Codebase?
The programmer of the Application must: 
 - Decide what and how the new Application works, ie how eventids and other node variables are used to build the Aplication.  
 - Choose the NodeID - this must be from a range **controlled** by the manufacturer - **ie you**.  
 - Write the **CDI/xml** describing the node and its node-variables, including its eventIDs. 
 - Write a **MemStruct{}** that matches the xml description.  
 - Choose the Memory Model, one of: **SMALL. MEDIUM, or LARGE**.  A good first choice is **MEDIUM**.  
 - Write code to flag each eventID as a **Producer, Consumer, or Both**.  
 - Write **pceCallback()**, which processes received eventIDs, ie eventIDs to be consumed, and causing whatever action is required, eg a LED being lit or extinguished.  
 - Write **produceFromInputs()** which scans the node's inputs and, if appropriate, flags an evenItD to be sent.  
 - Write **userConfigWrite()** which is called whenever a UI Tool writes to the node's memory.  This code can then compare the memory address range of the change to the node's variables, and take whatever action is appropriate, e.g. update a servo position.
 - Write additional support and glue code for the Application.  

## Example Applications
The provided examples will give some ideas of how to accomplish sample projects.  They can form the basis of and be adapted for a new Application, or used for inspiration.  
 - **OlcbBasicNode** implements a simple node which exercises most of the protocols.  It has two inputs and two outputs.  Each input has two Producer-eventIDs and each output has two Consumer-eventIDs, so 8 eventIDs in total.  This Application makes use of the ButtonLed library to control two buttons and two LEDs.  In addition, it implements the BG (Blue-Gold) protocol to allow the **teaching** of eventIDs between this node and others.  
- **OlcbServoPCA8695** implements driving a number of servos from a PCA8695 PWM chip.  It shows how to write a different **pceCallback()**.  It also uses **userConfigWrite()** to allow real-time updating of a servo positions from a **UI Tool**, such as **JMRI** or **Model Railroad System**.  
