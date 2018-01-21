#define P(x)    { Serial.print(x); delay(1);}
#define PS(x)   { P(F(x)); }
#define PH(x)   { if(x<0x10)P(0); Serial.print(x,HEX); }
#define PH16(x) { if(x<0x1000)P(0); if(x<0x100)P(0); PH(x); }
#define PH32(x) { if(x<0x10000000)P(0);if(x<0x1000000)P(0);if(x<0x100000)P(0);if(x<0x10000)P(0);PH16(x); }
#define PL      { P("\n"); }
#if (0x0102>>8==2)
  #define endian16(num) num
  #define endian32(num) num
  #define endian64(num) num
#else
  #define endian16(num)  ( ( (num>>8) &0x00ff ) | ( (num<<8) &0xff00 ) )   // byte 0 to byte 3
  #define endian32(num)  ( ( (num>>24) &0xff ) | ( (num<<8) &0xff0000 ) | ( (num>>8) &0xff00 ) | ( (num<<24) &0xff000000 ) )   // byte 0 to byte 3
  #define endian48(num)  ( ((num>>40)&0xff) | ((num>>24)&0xff00) | ((num>>8)&0xff0000) | \
                           ((num&0xff)<<40) | ((num&0xff00)<<24) | ((num&0xff0000)<<8) )
  #define endian64(num)  ( ((num>>56)&0xff) | ((num>>40)&0xff00) | ((num>>24)&0xff0000) | ((num>>8)&0xff000000) | \
                           ((num&0xFF)<<56) | ((num&0xFF00)<<40) | ((num&0xFF0000)<<24) | ((num&0xFF000000)<<8) )
#endif

// ==== NodeID ====
class NodeID {
  public: 
    union {
      uint64_t nid;      // node ID
      uint8_t  val[8]; 
    };
    uint16_t alias;      // node Alias
    NodeID(uint64_t _nid, uint16_t _alias) { nid=_nid; alias=_alias; }
    NodeID(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) {
      val[0]=a; val[1]=b; val[2]=c; val[3]=d; val[4]=e; val[5]=f; 
    }
    equal(NodeID n) { return (this->nid==n.nid); }
};
// ==== EventID ====
class EventID {
  public:
    union {
      uint64_t eid;
      uint8_t val[8];
    };
    EventID(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f, uint8_t g, uint8_t h) {
      val[0]=a; val[1]=b; val[2]=c; val[3]=d; val[4]=e; val[5]=f; val[6]=g; val[7]=h; 
    }
    EventID(uint64_t a) {
      eid = endian64(a); 
    }
};
// ==== Event ====
class Event {
  uint16_t flags;
  void setFlag(uint16_t m) { flags |=  m; }
  void resetFlag(uint16_t m) { flags &=  ~m; }
  void process(){}
};

// ==== OlcbNet =============================================================
class OlcbNet {
  public:
    virtual bool init();                      // initialize interface
    virtual bool txIdle();                       // is interface idle?
    virtual bool txAvail();                     // are buffers available?
    virtual bool write(long timeout);    // if timeout==0, write immediately or fail, else activate timeout
    bool write() { write( (long)0 ); }
    virtual bool rxAvail();                      // is rx buffer available?
    virtual bool read();                    // read
    bool active;                       // status of tx  
};

// ==== CAN =========================================
class OlcbCan : public OlcbNet {
  public: 
    uint32_t id;
    uint8_t len;
    uint8_t data[8];
    OlcbCan(uint32_t _id, uint8_t _len, uint8_t* _data) {
      id=_id; len=_len; memcpy(&data, _data, len);
    }
};
//=============On Tiva=============
class Canbus {
  public:
    uint8_t msg[13];
    void send() {}
};
class TivaCan : public OlcbCan {
  public:
    bool init(){return 0;}                      // initialize interface
    bool txIdle(){return 0;}                       // is interface idle?
    bool txAvail(){return 0;}                     // are buffers available?
    bool write(long timeout){ 
      Canbus b;
      memcpy(&b, (uint8_t*)this, 13);
      b.send();
      return 0;
    }
    bool rxAvail(){return 0;}                      // is rx buffer available?
    bool read(){return 0;}                    // read
};
//=============On AT90CAN =============
class AT90Can : public OlcbCan {
  public:
    bool init(){return 0;}             
    bool txIdle(){return 0;}           
    bool txAvail(){return 0;}                    
    bool write(long timeout){return 0;}   
    bool rxAvail(){return 0;}              
    bool read(){return 0;}                    
};

// ==== Ethernet =====================================
class OlcbEth : public OlcbNet {
  public:
    uint16_t mti;
    NodeID src;
    NodeID dst;
    EventID eid;
    uint8_t len;
    uint8_t data[80];
};
class TivaEth : public OlcbEth {
  public:
    bool init(){return 0;}             
    bool txIdle(){return 0;}           
    bool txAvail(){return 0;}                    
    bool write(long timeout){ return 0; }   
    bool rxAvail(){return 0;}              
    bool read(){return 0;}                      
};

// ===== Interface ======================================================
class OlcbInterfaceClass {
  public:
    virtual bool isMti(uint16_t m);
    virtual void setMti(uint16_t m);
    virtual NodeID getSource();
    virtual void setSource(NodeID src);
    virtual NodeID getDestination();
    virtual void setDestination(NodeID dst);
    virtual bool isPcer();
    virtual void setPcer();
    virtual bool isDG();
    virtual bool isStream();
    virtual bool isAddressed();
};
// ==== On CAN ====================================
class OlcbCanInterface : public OlcbInterfaceClass, public OlcbCan {
  NodeID nid;
  public: 
    OlcbInterface(NodeID _nid) { nid=_nid; }
    OlcbCan* can;
  // CAN-MTI definitions
    static const uint16_t mti_initComplete  = 0x19100;
    static const uint16_t mti_pipReq        = 0x19828;
    static const uint16_t mti_pcer          = 0x195B4;
    static const uint16_t mti_streamData    = 0x1F;
    static const uint16_t mti_streamInitReq = 0x19CC8;
  // getters
    NodeID getSource(){ 
      NodeID nid(0,can->id&0x0FFF);
      return nid; 
    }
    void setSource(NodeID src) {
      can->id |= src.alias;
    }
    NodeID getDestination(){
      if(isDG()||isStreamData()) return NodeID(0,(can->id>>12)&0x0FFF);
      if(isAddressed()) return NodeID(0, ((uint16_t)data[0])<<8 | data[1] );
      return NodeID(0,0);       
    }
    void setDestination(NodeID dst){
      can->id |= ((uint64_t)dst.alias)<<12;
    }
    bool isMti(uint16_t m) { return (id>>12)&m; }
    void setMti(uint16_t m) { id |= (m<<12); }
    bool isPcer() { return isMti(mti_pcer); }
    void setPcer() { setMti(mti_pcer); }
    bool isDG();
    bool isStream() { return isMti(mti_streamInitReq) || isSMti(mti_streamData); }
    bool isAddressed() { return id&0x00008000; } 
  protected:
    bool isSMti(uint8_t m) { return (id>>24)&m; }
    void setSMti(uint8_t m) { id |= (m<<24); }
    bool isStreamData();
  // alias
    static const uint16_t CID[4] = { 7, 6, 5, 4 };
    static const uint16_t RID   = 0x700;
    static const uint16_t AMD   = 0x701;
    static const uint16_t AME   = 0x702;
    static const uint16_t AMR   = 0x703;
    bool obtainAlias(uint16_t* seed) {
      uint8_t state = 0;
      long timeout = millis() + 500;
      nid.alias = seed;
      while(state<5) {
        if(millis()>timeout) return false;
        if( OlcbCan( ((uint32_t)CID[state])<<24 | nid.alias, 0, 0).write(200 ) ) state++;
        else {
          nid.alias++;
          state=0;
        }
      }
      OlcbCan( RID<<12 | nid.alias, 0, 0).write();
      OlcbCan( AMD<<12 | nid.alias, 0, 0).write();
      OlcbCan( mti_initComplete<<12 | nid.alias, 0, 0).write();
    }
};
// ==== On Ethernet ====================================
class OlcbEthInterface : public OlcbInterfaceClass, public OlcbEth {
  public: 
    OlcbEth* eth;

    // MTI definitions
    static const uint16_t mti_initComplete   = 0x0100;
    static const uint16_t mti_pipReq         = 0x0828;
    static const uint16_t mti_pcer           = 0x05B4;
    static const uint16_t mti_dg             = 0x1C48;
    static const uint16_t mti_dgOk           = 0x0A28;
    static const uint16_t mti_dgRejd         = 0x0A48;
    static const uint16_t mti_streamInitReq  = 0x0CC8;
    static const uint16_t mti_streamInitOk   = 0x0CC8;
    static const uint16_t mti_streamDataSend = 0x0CC8;

     // construct and deconstruct
    bool isMti(uint16_t m) { return m==mti; }
    void setMti(uint16_t m) { mti=m; }
    bool isPcer() { return isMti(mti_pcer); }
    void setPcer() { setMti(mti_pcer); }
    bool isPipReq() { return isMti(mti_pipReq); }
    void setPipReq() { setMti(mti_pipReq); }
    bool isDG() { return isMti(mti_dg); }
    bool isStreamIntReq() { return isMti(mti_streamInitReq); }
  protected:
};


void setup() {
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
