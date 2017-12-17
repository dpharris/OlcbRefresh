# OlcbRefresh
This is a refresh of the Arduino base libs, ie OlcbStarLibraries.  

It is meant to simplify and extend the Arduino code.

## Changes: 
1. Added support for multiple processors: Arduino, Teensy, Tiva
     Each set of libraries are in a separate directory.
2. Simplified the writing of CDI/xml for the node
    by making a struct{} that parallels the xml structure.   
3. To speed up eventID processing, 
    uses Index[]'s to allow eventID's to be sorted and searched.  

CDI/Memory:
```
    <cdi>
        <group replication='8'>
        <name>Channels</name>
            <eventid><name>evento</name></eventid>
            <eventid><name>event1</name></eventid>
        </group>
    </cdi>
```
becomes:    
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
2. Medium: eventIDs are copied to RAM as eventids[] (default);
3. Large:  The whole of EEPROM is mirrored to RAM as mem[].

### In RAM:
The offset in EEPROM of each eventID is stored into eventidOffset[].
The node's eventIDs are hashed, and storied along with a sequential index into eventidIndex[],
  and this is sorted on the hash value. 
In all models: 
        eventidIndex[]--->eventidOffset[]-->mem[] or EEPROM[]
In the Medium model, eventIndex also indexes the eventIDs:
        eventidIndex[]--->eventid[]



    