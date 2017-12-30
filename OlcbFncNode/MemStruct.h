
#ifndef MemStruct_h
#define MemStruct_h
//#pragma message("!!! compiling MemStruct.h")

#include <Arduino.h>

#include "EventID.h"
#include "Event.h"
//#include "EventsIndex.h"
#include "NodeMemory.h"
void userFillEventOffsets();
#include "NodeID.h"
#include "Index.h"
#include <EEPROM.h>

#define MEM_SMALL 1
#define MEM_MEDIUM 2
#define MEM_LARGE 3
#define MEM_MODEL MEM_SMALL    // small but slow, works out of EEPROM
//#define MEM_MODEL MEM_MEDIUM   // faster, eventIDs are copied to RAM
//#define MEM_MODEL MEM_LARGE    // fastest but large, EEPROM is mirrored to RAM 

// Number of channels implemented. Each corresonds 
// to an input or output pin.
#define NUM_CHANNEL 4

// total number of events, two per channel
#define NUM_EVENT 1*NUM_CHANNEL

/*
 *  Definition of the memory structure of the node's non-volatile memory, usually EEPROM.   
 *  It defines the memory layout of the node's variables.  
 *  It has three parts: 
 *    1. Non-optional node identification: NodeID, the next available eventID, and a magic 
 *         variable that documents the state of this memory.  
 *    2. Optional node identification: node-name and node-description.  These are optional, 
 *         but are useful for making the node's identification human-readable.  They are part 
 *         of ACDI, teh abreviated CDI.  
 *    3. Application data.  THe layout is up to the node designer.  The resulting structure
 *         has to be mirrired in the CDI xml description.  The CDI describes the nature and 
 *         size of the various node-variables/fields.  
 *         
 *  The example below demonstates two inouts, each with a description, and a pair of 
 *     producer-eventIDs, and two outputs, each with a descriptin and a pair of 
 *     consumer-eventIDs.  
 *     
 *  The struct is collected into a typedef MemStruct.  This is used to declare: 
 *    1. pmem -- a zero-based pointer used to access eepro amd ram, as necessary;
 *    2. mem  -- an in-memory structure used to mirrow the eeprom in the large-memory model.  
 */

typedef struct {
    EventID eid;      // associated eventID
    uint16_t action;  // action function
    uint16_t pin;     // associtiated pin
    uint16_t parm0;   // first parameter
    uint16_t parm1;   // second parameter
} Channel;

void blink(uint16_t pin, uint16_t period, uint16_t duty);
void dstrobe(uint16_t pin, uint16_t period, uint16_t duty);
void fade(uint16_t pin, uint16_t targetIntensity, uint16_t rate);
void servo(uint16_t pin, uint16_t targetPosition, uint16_t speed);
void input(uint16_t pin, uint16_t sense, uint16_t debounce);
void sample(uint16_t pin, uint16_t sense, uint16_t duty);

typedef void(*Action_t)(uint16_t, uint16_t, uint16_t);
Action_t action[] {
    blink, dstrobe, fade, servo, input, sample
};
bool consumerFnc[] = { true, true, true, true, false, false };

typedef struct //__attribute__ ((packed)) 
{ 
  uint32_t magic;         // used to check eeprom status
  uint16_t nextEID;       // the next available eventID for use from this node's set
  uint8_t  nid[6];        // the nodeID
  char     nodeName[20];  // optional node-name, used by ACDI
  char     nodeDesc[24];  // optional node-description, used by ACDI
  Channel  channel[NUM_CHANNEL];    // Channel defn
} MemStruct;              // type definition
// zero-based pointer into the MemStruct structure
MemStruct * pmem = 0;   
#define ADDR_EID(x) ((unsigned int)&pmem->x)

// define eventsOffset array in flash      (Balaz's idea) Note: this negates the need for userInitEventIDOffsets()
const PROGMEM uint16_t eventidOffset[] = {
   ADDR_EID( channel[0].eid ),
   ADDR_EID( channel[1].eid ),
   ADDR_EID( channel[2].eid ),
   ADDR_EID( channel[3].eid ),
};

// Sorted index to eventids
Index eventsIndex[NUM_EVENT];  // Sorted index to eventids

//Event events[NUM_EVENT];      // array of offsets of the node's eventIDs in mem/EEPROM -- this is never sorted
                              // this allows the Olcb code to process these eventiDs in natural order



#if MEM_MODEL == MEM_MEDIUM
   EventID eventids[NUM_EVENT];    // copy of eventids in RAM
#endif

/*typedef struct {
  NodeID* nid;
  Event* events;
  Index* eventsIndex;
  const uint16_t* eventidOffset;
  uint16_t nevent;
} Nodal_t;
*/
  void writeEEPROM(int addr, uint8_t b);
  void initTables();
  void restore();
  void printRawEEPROM();
  void initTables();
  void printEventsIndex();
  void printEvents();
#endif
