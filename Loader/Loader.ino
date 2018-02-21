#define DEBUG
#ifdef DEBUG
    #define dB(x) Serial.begin(x)
    #define dP(x) Serial.print(x)
    #define dPH(x) Serial.print(x,HEX)
    #define dPL Serial.println()
    #define dPN(x) Serial.print("\n" #x ":")
#else
    #define dP(x) 
    #define dPH(x) 
    #define dPL 
    #define dPN(x) 
#endif

#include "AT90can.h"
Can olcbcanTx;
Can olcbcanRx;
#include "OlcbCanInterface.h"
OlcbCanInterface txBuffer(&olcbcanTx);
OlcbCanInterface rxBuffer(&olcbcanRx);
#include "NodeID.h"
NodeID nodeid(1,2,3,4,5,6);
#include "LinkControl.h"
LinkControl link(&txBuffer, &nodeid);
#include "Datagram.h"
unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from);
Datagram dg(&txBuffer, datagramCallback, &link);

const unsigned int pageSize = 1024;
unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from) {
  static unsigned int l = 0;
  static unsigned int pPB = 0;
  unsigned int len = length;
  uint8_t pageBuffer[pageSize];
  dP("\nDG callback");
  if(rbuf[0]!=0x20) return 0;
  switch(rbuf[1]) {
  case 0xFD:  // write to flash
    l += len;
    if(l>pageSize) {
      len = pageSize - l;  // amount left behind
      l = pageSize;
    }
    memcpy(&pageBuffer[pPB], &rbuf[0], l);
     
  }
}

void setup() {
   dB(115200);
   dP("\nLoader trial");

}

void loop() {
    bool rcvFramePresent = rxBuffer.net->read();
    link.check();
    dg.check();
}
