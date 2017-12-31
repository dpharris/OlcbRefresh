// Mock Can class
#ifndef Can_h
#define Can_h

typedef struct {
  union {
    uint32_t id;
    uint8_t  idb[4];
  };
  uint8_t length;
  uint8_t data[8];
  bool isForHere(uint16_t a){}
  bool isFrameTypeOpenLcb(){}
  bool isAddressedMessage(){}
} Can_t;
class Can {
 public:
  void init(){}
  uint8_t avail(){}
  uint8_t read(Can_t *m){}
  uint8_t bufFree(){}
  uint8_t write(Can_t *m){}
};

#endif Can_h

