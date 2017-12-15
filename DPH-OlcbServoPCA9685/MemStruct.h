
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
    char desc[16];        // description of this servo
    EventID pos[3];       // eventIDs to set to position 
    uint16_t drate;       // down rate
    uint16_t urate;       // up rate
    uint16_t bounce;      // amoutn of bounce
  } servo[8];             // 8 servos
} MemStruct;              // type definition
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// zero-based pointer into the MemStruct structure
MemStruct * pmem = 0;  

// define eventsOffset array in flash      (Balaz's idea) Note: this negates the need for userInitEventIDOffsets()
#define ADDR_EID(x) ((unsigned int)&pmem->x)
#define REG_SERVO(s) ADDR_EID(servo[s].pos[0]), ADDR_EID(servo[s].pos[1]), ADDR_EID(servo[s].pos[2]) 
const PROGMEM uint16_t eventidOffset[] = {
   REG_SERVO(0),
   REG_SERVO(1),
   REG_SERVO(2),
   REG_SERVO(3),
   REG_SERVO(4),
   REG_SERVO(5),
   REG_SERVO(6),
   REG_SERVO(7)
};

// Sorted index to eventids
Index eventsIndex[NUM_EVENT];  // Sorted index to eventids

#endif // MemStruct_h
