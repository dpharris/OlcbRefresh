#ifndef Event_h
#define Event_h

#include "EventID.h"

class EventID;
class Event {
 public:
  Event();
  Event(bool produce, bool consume);
  Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
  
  //bool equals(Event* n);
  void print();
  //EventID getEID(unsigned i);
  static int evCompare(void* a, void* b);
  static int hashCompare(const void* a, const void* b);

  /**
   * Check to see if this object is equal
   * to any in an array of Events
   */
  Event* findEventInArray(Event* array, int len);
  
  int findIndexInArray(Event* array, int len, int start);
    
  // hash to try to speed up searches
  //int hash;
  // index of the eventID before sorting
  //int index;
  // offset into MemStruct --- depreciated, instead construct offsets in flash at compile-time
  //uint16_t offset;
  // bit mask local flags
  uint16_t flags;
  
  // Mark entry as consumer
  static const int CAN_CONSUME_FLAG = 0x20;
  // Mark entry as producer
  static const int CAN_PRODUCE_FLAG = 0x40;
};

#endif
