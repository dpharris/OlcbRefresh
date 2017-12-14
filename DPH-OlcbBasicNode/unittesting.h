// Unit Testing
#include "OpenLcbCan.h"

#if test==testEquals
void testEquals() {
  Serial.print(F("\ntestEquals()"));
  EventID a(5,1,1,1,3,255,0,3);
  EventID b(5,1,1,1,3,255,1,3);
  EventID* c = &b;
  if(a.equals(&a)) Serial.print(F("\n  a==a pass"));
  else  Serial.print(F("\n  a==a FAIL"));
  if(a.equals(&b)) Serial.print(F("\n  a==&b FAIL"));
  else  Serial.print(F("\n  a==&b pass"));
  if(a.equals(c)) Serial.print(F("\n  a==c FAIL"));
  else  Serial.print(F("\n  a==c pass"));
  if(c->equals(&b)) Serial.print(F("\n  c==&b pass"));
  else  Serial.print(F("\n  c==&b FAIL"));
  if(b.equals(c)) Serial.print(F("\n  b==c pass"));
  else  Serial.print(F("\n  b==c FAIL"));
  if(c->equals(c)) Serial.print(F("\n  c==c pass"));
  else  Serial.print(F("\n  c==c FAIL"));
}
#endif
#if test==testFindIndexInArray
void testFindIndexInArray() {
  Serial.print("\n\nTest findIndexInArray");
  EventID eid(5,1,1,1,3,0xFF,0,1);
  Serial.print(F("\neid.hash="));Serial.print(eid.hash(),HEX);
  int v;
  v = eid.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==0) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  v = eid.findIndexInArray(eventsIndex, NUM_EVENT, v+1);  // is there a second match? 
  if(v==-1) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid1(5,1,1,1,3,0xFF,0,2);
  v = eid1.findIndexInArray(eventsIndex, NUM_EVENT, 1); // should find it
  if(v==1) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  v = eid1.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==1) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid3(5,1,1,1,3,0xFF,0,3);
  v = eid3.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==2) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid4(5,1,1,1,3,0xFF,0,4);
  v = eid4.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==3) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid5(5,1,1,1,3,0xFF,0,5);
  v = eid5.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==4) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid6(5,1,1,1,3,0xFF,0,6);
  v = eid6.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==5) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid7(5,1,1,1,3,0xFF,0,7);
  v = eid7.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==6) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid8(5,1,1,1,3,0xFF,0,8);
  v = eid8.findIndexInArray(eventsIndex, NUM_EVENT, 0); 
  if(v==7) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
}
#endif
#if test==testIndex_FindIndex
void testIndex_FindIndex() {
  Serial.print("\n\nTestIndex_findIndex");
  EventID eid(5,1,1,1,3,0xFF,0,2);
  Serial.print(F("\neid.hash="));Serial.print(eid.hash(),HEX);
  Index* ind;
  ind = eventsIndex->findIndex(&eid,8,NUM_EVENT,0);
  if(ind->index==1) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  ind = eventsIndex->findIndex(&eid,8,NUM_EVENT,ind+1); //is there a second match?
  if(ind==nullptr) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  ind = eventsIndex->findIndex(&eid,8,NUM_EVENT,ind); // should succeed
  if(ind->index==1) Serial.print(F(" -- pass"));
  else Serial.print(" -- FAIL"); 
  EventID eid1(5,1,1,1,3,0xFF,0,1);
  ind = eventsIndex->findIndex(&eid1,8,NUM_EVENT,0); // should succeed
  if(ind->index==0) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
  EventID eid2(5,1,1,1,3,0xFF,0,8);
  ind = eventsIndex->findIndex(&eid2,8,NUM_EVENT,0); // should succeed
  if(ind->index==7) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL")); 
}
#endif
#if test==testBsearch
void testBsearch() {
  Serial.print("\nTest Bsearch()");
  EventID eid0 = eventsIndex[3].getEID();
           Serial.print(F("\n  eid0=")); eid0.print();
  Index hh;
  hh.hash = eid0.hash();
           Serial.print(F("\n  hh=")); Serial.print(hh.hash,HEX);
  Index* ei;
  ei = (Index*)bsearch( &hh, eventsIndex, NUM_EVENT, sizeof(Index), Index::findCompare);
  EventID eid1 = events[ei->index].getEID();
     eid1.print();
  if(0==memcmp(&eid1,&eid0,8)) Serial.print(F(" --  pass"));
  else Serial.print(F(" ==FAIL"));
}
#endif
#if test==testPCEEventReport
void testPCEEventReport() {
  Serial.print("\nTest PCE::EventReport()\n");
  EventID eid0 = EventID(5,1,1,1,3,0xFF,0,6);
  eid0.print();
  uint8_t bf[12] = {0};
  OpenLcbCanBuffer* buf = (OpenLcbCanBuffer*)bf;
  buf->setPCEventReport(&eid0);
  bool b = pce.receivedFrame(buf);
  if(b) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- FAIL"));
  eid0 = EventID(5,1,1,1,3,0xFF,0,66);
  buf->setPCEventReport(&eid0);
  b = pce.receivedFrame(buf);
  if(b) Serial.print(F(" -- pass"));
  else Serial.print(F(" -- fail"));
}
#endif
#if test==testSpeed
void testSpeed() {
  Serial.print("\nTest Speed od Event Processing\n");
  EventID eid0 = EventID(5,1,1,1,3,0xFF,0,6);
  eid0.print();
  uint8_t bf[12] = {0};
  OpenLcbCanBuffer* buf = (OpenLcbCanBuffer*)bf;
  buf->setPCEventReport(&eid0);
  long ttt = millis();
  for(int i=0;i<1000;i++) {
    pce.receivedFrame(buf);
  }
  Serial.print(F("\nelapsed ms:")); Serial.print(millis()-ttt);
}
#endif
#if test==testPCEHandleLearnEvent
void testPCEHandleLearnEvent() {
  Serial.print("\nTest PCE::HandleLearnEvent()");
  #define LEARN_FLAG 0x08
  EventID old = events[2].getEID();
  int oldFlags = events[2].flags;
  EventID eid0 = EventID(1,2,3,4,5,6,7,0x0B);
  events[2].flags |= LEARN_FLAG;
  //Serial.print("\nevents[2].flags:"); Serial.print(events[2].flags,HEX);
  uint8_t bf[12] = {0};
  OpenLcbCanBuffer* buf = (OpenLcbCanBuffer*)bf;
  buf->setLearnEvent(&eid0);
  bool b = pce.receivedFrame(buf);
  if(b) Serial.print(" -- pass1");
  else Serial.print(" -- FAIL1");
  printRawEEPROM();
  //printRawMem();
  Serial.print("\nold:"); old.print();
  Serial.print("\neid0:"); eid0.print();
  EventID eid1 = events[2].getEID();
  Serial.print("\neid1:"); eid1.print();
  if(eid1.equals(&eid0)) Serial.print(" -- pass2");
  else Serial.print(" -- FAIL2");
  if(oldFlags==events[2].flags) Serial.print(" -- pass3");
  else Serial.print(" -- FAIL3");
}
#endif
#if test==testIdentifyConsumers
void testIdentifyConsumers() {
  Serial.print("\nTest IdentifyConsumers");
  //#define MTI_IDENTIFY_CONSUMERS 0x8F4
  Event e0 = events[3];
  uint8_t bf[12] = {0};
  OpenLcbCanBuffer* buf = (OpenLcbCanBuffer*)bf;
  buf->setOpenLcbMTI(MTI_IDENTIFY_CONSUMERS);
  bool b = pce.receivedFrame(buf);
  Serial.print(events[2].flags,HEX);
  if(events[3].flags & Event::CAN_CONSUME_FLAG) Serial.print(" -- pass");
  else Serial.print(" -- FAIL");
}
#endif
#if test==testIdentifyProducers
void testIdentifyProducers() {
  Serial.print("\nTest testIdentifyProducers");
  Event e0 = events[3];
  uint8_t bf[12] = {0};
  OpenLcbCanBuffer* buf = (OpenLcbCanBuffer*)bf;
  buf->setOpenLcbMTI(MTI_IDENTIFY_PRODUCERS);
  bool b = pce.receivedFrame(buf);
  if(events[3].flags & Event::CAN_PRODUCE_FLAG) Serial.print(" -- pass");
  else Serial.print(" -- FAIL");
}
#endif
#if test==testIdentifyEvents
#define IDENT_FLAG 0x01
void testIdentifyEvents() {
  Serial.print("\nTest testIdentifyEvents");
  Event e0 = events[3];
  uint8_t bf[12] = {0};
  OpenLcbCanBuffer* buf = (OpenLcbCanBuffer*)bf;
  buf->setOpenLcbMTI(MTI_IDENTIFY_EVENTS_GLOBAL);
  bool b = pce.receivedFrame(buf);
  //if(events[3].flags & Event::IDENT_FLAG) Serial.print(" -- pass");
  if(events[3].flags & IDENT_FLAG) Serial.print(" -- pass");
  else Serial.print(" -- FAIL");
}
#endif
#if test==testCan
//#include "processor.h"
void testCan() {
  Serial.print("\nTest testCan");
  tCAN buff;
  buff.id = 45;
  buff.flags.extended = 1;
  buff.length = 4;
  buff.data[0] = 0x11;
  buff.data[1] = 0x22;
  buff.data[2] = 0x33;
  buff.data[3] = 0x44;
  can_send_message(&buff);
  can_get_message(&buff);
  if(buff.id==45 \
  &&(buff.length==4) \
  &&(buff.data[0]=0x11) \
  &&(buff.data[1]=0x22) \
  &&(buff.data[2]=0x33) \
  &&(buff.data[3]=0x44) \
  ) Serial.print("\nYay");
  else Serial.print("\nNay");
}
#endif


