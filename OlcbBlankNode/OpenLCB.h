
typedef struct {
  uint32_t magic;         // used to check eeprom status
  uint16_t nextEID;       // the next available eventID for use from this node's set
  uint8_t  nid[6];        // the nodeID
  char     nodeName[20];  // optional node-name, used by ACDI
  char     nodeDesc[24];  // optional node-description, used by ACDI
}  NodeVar;

typedef struct {
  uint16_t offset;
  uint8_t flags;
} EIDTab;

#define CEID(x) ((unsigned int)&pmem->x,Event::CAN_CONSUME_FLAG)
#define PEID(x) ((unsigned int)&pmem->x,Event::CAN_PRODUCE_FLAG)
#define PCEID(x) ((unsigned int)&pmem->x,Event::CAN_CONSUME_FLAG|Event::CAN_PRODUCE_FLAG)

#define pSimple       0x80
#define pDatagram     0x40
#define pStream       0x20
#define pMemConfig    0x10
#define pReservation  0x08
#define pPCEvents     0x04
#define pIdent        0x02
#define pTeach        0x01

#define pRemote       0x80
#define pACDI         0x40
#define pDisplay      0x20
#define pSNIP         0x10
#define pCDI          0x08
#define pTraction     0x04
#define pFunction     0x02
#define pDCC          0x01

#define pSimpleTrain  0x80
#define pFuncConfig   0x40
#define pFirmwareUpgrade     0x20
#define pFirwareUpdateActive 0x10

#define OlcbCommonVersion "0.1"
class NodeID {
  public:
    uint8_t val[6];
    NodeID(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f){}
};
class EventID {
  public:
    uint8_t val[6];
    EventID(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f,uint8_t g, uint8_t h){}
};
#define PRODUCE_FLAG 0x02
class Event {
  public:
    uint16_t flags;
    void send() { flags |= PRODUCE_FLAG; }
  // Mark entry as consumer
  static const int CAN_CONSUME_FLAG = 0x20;
  // Mark entry as producer
  static const int CAN_PRODUCE_FLAG = 0x40;
};
class ButtonLed {
  public:
    ButtonLed(int a, int b){}
    void process(){}
    void on(uint32_t p){}
    void blink(uint32_t p){}
};
class OpenLCB {
  public:
    bool can_active;
    OpenLCB( NodeID* nodeid, uint16_t nevent, Event* event, uint16_t* eventIndex, const EIDTab* eventidOffset ){}
};

Event event[NUM_EVENT] = { Event() };
uint16_t eventsIndex[NUM_EVENT];  // Sorted index to eventids

void Olcb_setup(){}  // setup system
bool Olcb_loop(){}   // process system
restore(){};        // restore tables

// ===== User routines =====
extern void pceCallback(uint16_t index);
void userConfigWrite(unsigned int address, unsigned int length);

