/*
 * MemStruct.h  defines the memory structure of EEPROM and RAM-mirror, if Medium-Model
 */

#ifndef MemStruct_h
#define MemStruct_h
//#pragma message("!!! compiling MemStruct.h")

#include <Arduino.h>

#include "EventID.h"
#include "Event.h"
#include "NodeMemory.h"
#include "NodeID.h"
#include "Index.h"
#include <EEPROM.h>

// Memory Model -- Choose one only.
//#define MEM_MODEL_SMALL    // small but slow, works out of EEPROM
#define MEM_MODEL_MEDIUM   // faster, eventIDs are copied to RAM
//#define MEM_MODEL_LARGE    // fastest but large, EEPROM is mirrored to RAM 

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

// vvvvvvvv User defined EEPROM layout vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
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
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// zero-based pointer into the MemStruct structure
MemStruct * pmem = 0;   

// Sorted index to eventids
Index eventsIndex[NUM_EVENT];  // Sorted index to eventids

// define eventsOffset array in flash      (Balaz's idea) Note: this negates the need for userInitEventIDOffsets()
#define ADDR_EID(x) ((unsigned int)&pmem->x)
const PROGMEM uint16_t eventidOffset[] = {
   ADDR_EID( inputs[0].activation   ),
   ADDR_EID( inputs[0].inactivation ),
   ADDR_EID( inputs[1].activation   ),
   ADDR_EID( inputs[1].inactivation ),
   ADDR_EID( outputs[0].setEvent    ),
   ADDR_EID( outputs[0].resetEvent  ),
   ADDR_EID( outputs[1].setEvent    ),
   ADDR_EID( outputs[1].resetEvent  )
};

#ifdef MEM_MODEL_MEDIUM
   EventID eventids[NUM_EVENT];    // copy of eventids in RAM
#endif

extern void userFillEventOffsets();
extern  void writeEEPROM(int addr, uint8_t b);
extern  void initTables();
extern  void restore();
extern  void printRawEEPROM();
extern  void initTables();
extern  void printEventsIndex();
extern  void printEvents();

#endif // MemStruct_h
