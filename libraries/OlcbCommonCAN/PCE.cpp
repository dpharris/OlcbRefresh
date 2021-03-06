#include <string.h>

#include "PCE.h"

#include "NodeID.h"
#include "EventID.h"
#include "Event.h"
#include "LinkControl.h"
#include "OpenLcbCanBuffer.h"

#include "lib_debug_print_common.h"

// Mark as waiting to have Identify sent
#define IDENT_FLAG 0x01
// Mark produced event for send
#define PRODUCE_FLAG 0x02
// Mark entry as really empty, ignore
#define EMPTY_FLAG 0x04
// Mark entry to written from next learn message
#define LEARN_FLAG 0x08
// Mark entry to send a learn message
#define TEACH_FLAG 0x10

//extern int *eventOffsets;

//PCE::PCE(Event* evts, int nEvt, Index* eIndex, OpenLcbCanBuffer* b, NodeID* node, void (*cb)(uint16_t i), void (*rest)(), LinkControl* li) {
PCE::PCE(Nodal_t* nodal, OpenLcbCanBuffer* b, void (*cb)(uint16_t i), void (*rest)(), LinkControl* li) {
      //events = evts;
    events = nodal->events;
      //eventsIndex = eIndex;
    eventsIndex = nodal->eventsIndex;
      //nEvents = nEvt;
    nEvents = nodal->nevent;
      buffer = b;
      //nid = node;
    nid = nodal->nid;
      callback = cb;
      restore = rest;
      link = li;
       
      // mark as needing transmit of IDs, otherwise not interesting
      // ToDo: Is this needed if requiring newEvent?
      for (int i = 0; i < nEvents; i++) {
         if (events[i].flags & ( Event::CAN_PRODUCE_FLAG | Event::CAN_CONSUME_FLAG ))
            events[i].flags = IDENT_FLAG;
      }
      sendEvent = 0;
  }
  
  void PCE::produce(int i) {
    // ignore if not producer
    //if ((events[i].flags & Event::CAN_PRODUCE_FLAG) == 0) return;
    if ((events[i].flags & Event::CAN_PRODUCE_FLAG) == 0) return;
    // mark for production
    events[i].flags |= PRODUCE_FLAG;
    sendEvent = sendEvent<i ? sendEvent : i;
  }

  extern EventID getEID(unsigned i);
  //void getEID(EventID *eid, uint16_t p);
  void PCE::check() {
     // see in any replies are waiting to send
     while (sendEvent < nEvents) {
         // OK to send, see if marked for some cause
         // ToDo: This only sends _either_ producer ID'd or consumer ID'd, not both
         //EventID ev = events[sendEvent].getEID();
         EventID ev = getEID(sendEvent);
         
         //LDEBUG("\nPCE::check "); LDEBUG(ev.val[7]);
         //LDEBUG(" I:"); LDEBUG(0!=(events[sendEvent].flags & IDENT_FLAG));
         //LDEBUG(" cP:"); LDEBUG(0!=(events[sendEvent].flags & Event::CAN_PRODUCE_FLAG));
         //LDEBUG(" cC:"); LDEBUG(0!=(events[sendEvent].flags & Event::CAN_CONSUME_FLAG));
         //LDEBUG(" P:"); LDEBUG(0!=(events[sendEvent].flags & PRODUCE_FLAG));
         //LDEBUG(" E:"); LDEBUG(0!=(events[sendEvent].flags & EMPTY_FLAG));

         if ( (events[sendEvent].flags & (IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) == (IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) {
           events[sendEvent].flags &= ~IDENT_FLAG;    // reset flag
           //buffer->setProducerIdentified(&events[sendEvent]);
             buffer->setProducerIdentified(&ev);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if ( (events[sendEvent].flags & (IDENT_FLAG | Event::CAN_CONSUME_FLAG)) == (IDENT_FLAG | Event::CAN_CONSUME_FLAG)) {
           events[sendEvent].flags &= ~IDENT_FLAG;    // reset flag
           buffer->setConsumerIdentified(&ev);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (events[sendEvent].flags & PRODUCE_FLAG) {
           events[sendEvent].flags &= ~PRODUCE_FLAG;    // reset flag
           buffer->setPCEventReport(&ev);
           handlePCEventReport(buffer);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (events[sendEvent].flags & TEACH_FLAG) {
           events[sendEvent].flags &= ~TEACH_FLAG;    // reset flag
           buffer->setLearnEvent(&ev);
           handleLearnEvent(buffer);
           OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else {
           // just skip
           sendEvent++;
         }
     }
  }
  
  void PCE::newEvent(int index, bool p, bool c) {
      //LDEBUG("\nnewEvent i=");
    events[index].flags |= IDENT_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
    if (p) events[index].flags |= Event::CAN_PRODUCE_FLAG;
    if (c) events[index].flags |= Event::CAN_CONSUME_FLAG;
      //LDEBUG(index); LDEBUG(" ");LDEBUG2(events[index].flags,HEX);
  }
  
  void PCE::markToLearn(int index, bool mark) {
    if (mark)
        events[index].flags |= LEARN_FLAG;
    else
        events[index].flags &= ~LEARN_FLAG;
  }


bool PCE::isMarkedToLearn(int index) {
	return events[index].flags==LEARN_FLAG;
}

void PCE::sendTeach(EventID e) {   /// DPH added for Clock
	buffer->setLearnEvent(&e);
	handleLearnEvent(buffer);
	OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
}	

  
  void PCE::sendTeach(int index) {
    events[index].flags |= TEACH_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
  }
  
  bool PCE::receivedFrame(OpenLcbCanBuffer* rcv) {
    //LDEBUG("\nIn receivedFrame");
    EventID eventid;
    if (rcv->isIdentifyConsumers()) {
        // see if we consume the listed event
        //Event event;
        rcv->getEventID(&eventid);
        int index = 0;
        // find consumers of event
          while (-1 != (index = eventid.findIndexInArray(eventsIndex, nEvents, index))) {
           // yes, we have to reply with ConsumerIdentified
           if (events[index].flags & Event::CAN_CONSUME_FLAG) {
               events[index].flags |= IDENT_FLAG;
               sendEvent = sendEvent < index ? sendEvent : index;
           }
           index++;
           if (index>=nEvents) break;
        }
    } else if (rcv->isIdentifyProducers()) {
        // see if we produce the listed event
        EventID eventid;
        rcv->getEventID(&eventid);
        int index = 0;
        // find producers of event
        while (-1 != (index = eventid.findIndexInArray(eventsIndex, nEvents, index))) {
           // yes, we have to reply with ProducerIdentified
           if (events[index].flags & Event::CAN_PRODUCE_FLAG) {
               events[index].flags |= IDENT_FLAG;
               sendEvent = sendEvent < index ? sendEvent : index;
           }
           index++;
           if (index>=nEvents) break;
        }
        // ToDo: add identify flags so that events that are both produced and consumed
        // have only one form sent in response to a specific request.
    } else if (rcv->isIdentifyEvents()) {
        // if so, send _all_ ProducerIdentified, ConsumerIdentified
        // via the "check" periodic call
        for (int i = 0; i < nEvents; i++) {
          events[i].flags |= IDENT_FLAG;
        }
        sendEvent = 0;  
    } else if (rcv->isPCEventReport()) {
        // found a PC Event Report, see if we consume it
        //LDEBUG("\nrcv->isPCEventReport!");
        handlePCEventReport(rcv);
    } else if (rcv->isLearnEvent()) {
        // found a teaching frame, apply to selected
        handleLearnEvent(rcv);
    } else return false;
    return true;
  }

  void PCE::handlePCEventReport(OpenLcbCanBuffer* rcv) {
                LDEBUG("\nIn handlePCEventReport");
      EventID eventid;
      rcv->getEventID(&eventid);
                eventid.print();
      // find matching eventID
      int ind = 0;
      while ( -1 != (ind = eventid.findIndexInArray(eventsIndex, nEvents, ind))) {
          uint16_t index = eventsIndex[ind].index;
            //LDEBUG("\nhandlePCRep ind: "); LDEBUG(ind);
            //LDEBUG("\nhandlePCRep Index: "); LDEBUG(index);
            //LDEBUG("\nevents[index].flags: "); LDEBUG2(events[index].flags,HEX);
          if (events[index].flags & Event::CAN_CONSUME_FLAG)
          {
          	//LDEBUG(F("Found Consumer: Index: ")); LDEBUG(index);
           	(*callback)(index);
          }
          ind++;
          if(ind>=nEvents) break;
      }
  }
  
  void PCE::handleLearnEvent(OpenLcbCanBuffer* rcv) {
             //LDEBUG("\nIn PCE::handleLearnEvent");
             //LDEBUG("\n rcv=");
      //for(int i=0;i<14;i++) { LDEBUG2( ((uint8_t*)rcv)[i],HEX ); LDEBUG(" "); }
        bool save = false;
        EventID eid;
        rcv->getEventID(&eid);
             //LDEBUG("\neid:"); eid.print();
             //LDEBUG("\nLEARN_FLAG:"); LDEBUG2(LEARN_FLAG,HEX);
        for (int i=0; i<nEvents; i++) {
            //LDEBUG("\ni:"); LDEBUG(i);
            //LDEBUG("\nevents[i].flags:"); LDEBUG(events[i].flags,HEX);
            if ( (events[i].flags & LEARN_FLAG ) != 0 ) {
                //rcv->getEventID(events+i);
                //LDEBUG("\ni:"); LDEBUG(i);
                //LDEBUG("\neid.writeEID(i):");
                //LDEBUG2(i,HEX);
                eid.writeEID(i);
                events[i].flags |= IDENT_FLAG; // notify new eventID
                events[i].flags &= ~LEARN_FLAG; // enough learning
                sendEvent = sendEvent < i ? sendEvent : i;
                save = true;
            }
        }
        //if (save) (*store)();   NEED TO FIX dph
        if (save) (*restore)(); // restore eventids to memory, if necessary
  }

