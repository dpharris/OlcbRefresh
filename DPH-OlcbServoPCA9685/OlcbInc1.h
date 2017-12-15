/* *************************************************
 * EEPROM memory layout
 *     See NodeMemory.h for background info
 *
 * Internal data, not to be reset by user:
 *     0 - 3        Memory state flag
 *     4 - 5        Cycle count
 *     6 - 11       Node ID
 *
 * User configuration data:
 *     12 - 12+8*sizeof(Event)  EventID storage
 * 
 *     94 - 113     Node name (zero-term string, 20 bytes total)
 *     114 - 136     User comment (zero-term string, 24 bytes total)
 *
 *     12+150 - 12+150+8*sizeof(Event)  Event description strings
 *
 *************************************************** */
//#include <EEPROM.h>

//#define LAST_EEPROM 12+150+NUM_EVENT*sizeof(Event)
//#define LAST_EEPROM sixeof(NodeBase) + sizeof(MemStruct)
#define LAST_EEPROM sizeof(MemStruct)

extern "C" {
  uint32_t spaceUpperAddr(uint8_t space) {  // return last valid address
    switch (space) {
      case 255: return sizeof(configDefInfo) - 1; // CDI (data starts at zero)
      case 254: return RAMEND; // RAM from Arduino definition
      case 253: return LAST_EEPROM; // Configuration
    }
    return (uint32_t)3;
  }

  const uint8_t getRead(uint32_t address, int space) {
    if (space == 0xFF) { // 255
      // Configuration definition information
      return pgm_read_byte(configDefInfo+address);
    } else if (space == 0xFE) { //254
      // All memory reads from RAM starting at first location in this program
      return *(((uint8_t*)&rxBuffer)+address);
    } else if (space == 0xFD) { //253
      // Configuration space is entire EEPROM
      return EEPROM.read(address);
    } else if (space == 0xFC) { // 252
      // used by ADCDI/SNII for constant data
      return pgm_read_byte(SNII_const_data+address);
    } else if (space == 0xFB) { // 251
      // used by ADCDI/SNII for variable data
      return EEPROM.read((int)SNII_var_data+address);
    } else {
      // unknown space
      return 0; 
    }
  }
  
  void getWrite(uint32_t address, int space, uint8_t val) {
    Serial.print("\nolcbinc getWrite");
    Serial.print(" space: "); Serial.print(space,HEX);
    Serial.print(":"); Serial.print(address,HEX);
    Serial.print("="); Serial.print(val,HEX);
    if (space == 0xFE) {
      // All memory
      *(((uint8_t*)&rxBuffer)+address) = val;
    } else if (space == 0xFD) {
      // Configuration space
      EEPROM.write(address, val);
    } 
    // all other spaces not written
  }

} // end of extern

//#include "EventTable.h"
//EventTableRow eventTable[NUM_EVENT];


/*
#ifdef EE
 void getEID(EventID *eid, uint16_t p) {
  for(int i=0; i<8; i++) eid[i] = EEPROM.read(p+i);
 }
#else
 MemStruct mem;
 void getEID(EventID *eid, uint16_t e) {
  uint8_t *p = (uint8_t*)eid;
  for(int i=0; i<8; i++) *p++ = EEPROM.read(e+i);
 }
 EventID getEID(uint16_t e) {
  EventID p;
  for(int i=0; i<8; i++) p.val[i] = EEPROM.read(e+i);
  return p;
 }
 EventID eventids[NUM_EVENT];
 void fillEventIDs(int n) {
  for (int i=0; i<n; i++) {
    eventids[i] = getEID(i);
  }
 }
#endif
*/

