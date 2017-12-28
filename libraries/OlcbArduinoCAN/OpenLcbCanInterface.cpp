
/**
 * Specific implementation for CAN MP2515
 */
//#include <can.h>

//#include "OpenLcbCanInterface.h"
#include "OpenLcbCanBuffer.h"
#include <Arduino.h>

void OpenLcb_can_init() {
    uint8_t r = can_init(125000);
    Serial.print("\nOpenLcb_can_init return=");
    Serial.print(r);
    //logstr("OpenLcb_can_init");
}

// Can a (the) CAN buffer be used?  
// Generally, indicates the buffer can be immediately
// queued for transmit, so it make sense to prepare it now
bool OpenLcb_can_xmt_ready(OpenLcbCanBuffer* b) {
  // use only MP2515 buffer 0 and 1 to ensure that
  // tranmissions take place in order
                                //Serial.print("\nOpenLcb_can_xmt_ready");
                                //Serial.print(SUPPORT_AT90CAN);
///#if defined(SUPPORT_MCP2515) && (SUPPORT_MCP2515 == 1)
//  uint8_t status = can_buffers_status();
//  // Check to see if Tx Buffer 0 or 1 is free
//  if ((status & (ST_TX0REQ|ST_TX1REQ)) == (ST_TX0REQ|ST_TX1REQ))
//    return false;  //  Both at full
//  else
//    return true;   // at least one has space
//#elif defined(SUPPORT_AT90CAN) && (SUPPORT_AT90CAN == 1)
//                                //Serial.print("\ncan_check_free_buffer1: ");
//    bool r = can_check_free_buffer();  //  Both at full
//    Serial.print(r);
//    return r;
//    //return can_check_free_buffer();  //  Both at full
//#else
    return OpenLcb_can_xmt_idle();
//#endif
}

// Queue a CAN frame for sending, if possible
// Returns true if queued, false if not currently possible
bool OpenLcb_can_queue_xmt_immediate(OpenLcbCanBuffer* b) {
                            //Serial.print("\nOpenLcb_can_queue_xmt_immediate id=");
                            //Serial.print(b->id,HEX);
    if (!OpenLcb_can_xmt_ready(b)) return false;
    //Serial.print("\nOpenLcb_can_queue_xmt_immediate 2");
  // buffer available, queue for send
    OpenLcb_can_active = true;
                            //Serial.print(" .. can_send_message");
    can_send_message(b);
    return true;
}

// Queue a CAN frame for sending; spins until it can queue
void OpenLcb_can_queue_xmt_wait(OpenLcbCanBuffer* b) {
                            //Serial.print("\nIn OpenLcb_can_queue_xmt_wait");
  while (!OpenLcb_can_queue_xmt_immediate(b)) {};
}

// Send a CAN frame, waiting until it has been sent
void OpenLcb_can_send_xmt(OpenLcbCanBuffer* b) {
                            //Serial.print("\nOpenLcb_can_send_xmt  ");
  OpenLcb_can_queue_xmt_wait(b);
  // wait for sent
  while (!OpenLcb_can_xmt_idle()) {}
}

// Check whether all frames have been sent,
// a proxy for the link having gone idle
bool OpenLcb_can_xmt_idle() {
                            //Serial.print("\nOpenLcb_can_send_xmt  ");
//#if defined(SUPPORT_MCP2515) && (SUPPORT_MCP2515 == 1)
//    uint8_t status = can_buffers_status();
//    //  Check to see if Tx Buffer 0,1 and 2 are all free
//    if ((status & (ST_TX0REQ|ST_TX1REQ|ST_TX2REQ)) == 0)
//        return true;   // All empty, nothing to send
//    else
//        return false;  // Any full
//#elif defined(SUPPORT_AT90CAN) && (SUPPORT_AT90CAN == 1)
//                            //Serial.print("\ncan_check_free_buffer2  ");
//    return can_check_free_buffer();  //  Both at full
//#else
    return can_check_free_buffer();
//#endif
}

// Make the oldest received CAN frame available,
// in the process removing it from the CAN subsystem.
// Return false (zero) if no frame available.
bool OpenLcb_can_get_frame(OpenLcbCanBuffer* b) {
    //Serial.print("\nIn OpenLcb_can_get_frame");
    int v = can_get_message(b);
                            //if(v>0) {
                            //  Serial.print("\nOpenLcb_can_get_frame  ");
                            //  Serial.println(b->id,HEX);
                            //}
    return v!=0;
}

bool OpenLcb_can_active;

