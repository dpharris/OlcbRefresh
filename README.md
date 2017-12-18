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
```
    <cdi>
        <group replication='8'>
        <name>Channels</name>
            <eventid><name>evento</name></eventid>
            <eventid><name>event1</name></eventid>
        </group>
    </cdi>
```
parallels this structure:    
```
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

It consists of: 
 - System/Housekeeping
   - Link - establishes and maintains the node's link to the network
     - On CAN maintains alias assignment and maintenance;
     - Announcement of *Intialization Complete*
     - Announcement of *Consumed-* and *Produced-eventIDs*
     - NodeID reporting on request.
     - EventID reporting on request.
   - CDI
     - Reporting of the CDI/xml on request.
   - Memory Configuration
     - Reading and writing to teh node's memory spaces, including Configuration, RAM and EEPROM
 - Application Messaging
   - Events
     - Unaddressed globally unique eventIDs (64-bit)
   - Datagrams
     - Addressed messages containing up to 70-bytes of data
   - Streams
     - Addressed unlimited data
 - Additional Protocols
   - Teaching
   - Traction Control
   - Button

    