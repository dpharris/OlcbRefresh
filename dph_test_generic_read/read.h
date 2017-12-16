


// OpenLCB/LCC is BIG-endian.  
// AVR, Tiva-ARM, Teensy-ARM are all LITTLE-endian.  

//#define LARGE
uint8_t mem[200];

#define _size 1000
#ifdef LARGE
  uint8_t read(uint16_t address) {
    uint8_t* m = mem; 
    return m[address];
  }
  void write(uint16_t address, uint8_t d) {
    uint8_t* m = mem; 
    m[address] = d; 
  }
#else
  uint8_t read(uint16_t address) {
    return EEPROM.read(address);
  }
  void write(uint16_t address, uint8_t d) {
    EEPROM.write(address, d); 
  }
#endif

  uint16_t read16(unsigned address) {
    union { uint32_t u16; uint8_t u0, u1; } r;
    r.u1 = read(address);
    r.u0 = read(address+1);
  }
  uint32_t read32(unsigned address) {
    union { uint32_t u32; uint8_t u0, u1, u2, u3; } r;
    r.u3 = read(address);
    r.u2 = read(address+1);
    r.u1 = read(address+2);
    r.u0 = read(address+3);
  }
  
  template<typename T>
  T &read(unsigned int address) {
    T r;
    for(unsigned i=0; i<sizeof(T); i++) r[i] = read(address+i);
    return r;
  }
  template<> uint16_t &read<uint16_t>(unsigned int address) {
    union { uint16_t u; uint8_t u0, u1; } r;
    r.u0 = read(address+1);
    r.u1 = read(address);
    return r.u;
  }
  template<> uint32_t &read<uint32_t>(unsigned int address) {
    union { uint32_t u; uint8_t u0, u1, u2, u3; } r;
    r.u0 = read(address+3);
    r.u1 = read(address+2);
    r.u2 = read(address+1);
    r.u3 = read(address);
    return r.u;
  }
  template<typename T>
  void* &read(unsigned int address, T *r) {
    uint8_t p = r;
    for(int i=sizeof(T); i; i--, p++) p = read(address+i);
  }
  
  uint16_t get16(unsigned address) {
    union { uint32_t u16; uint8_t u0, u1; } r;
    r.u1 = EEPROM.read(address);
    r.u0 = EEPROM.read(address+1);
  }
  uint32_t get32(unsigned address) {
    union { uint32_t u32; uint8_t u0, u1, u2, u3; } r;
    r.u3 = EEPROM.read(address);
    r.u2 = EEPROM.read(address+1);
    r.u1 = EEPROM.read(address+2);
    r.u0 = EEPROM.read(address+3);
  }


/*  
template<typename T>
class Get {
  T &get(unsigned int address, T &t) {
    T r;
    EEPROM.get(address, r);
    return r;
  }
};
template <>
class Get<uint16_t> {
uint16_t &get(unsigned int address, uint16_t &t) {
    uint16_t r;
    r = EEPROM.read(address+1);
    r = (r<<8) + EEPROM.read(address);
    return r;
  }
};
/*  
  template<typename T> 
  T &get(int address, T &t) {
    if (address < 0 || address + sizeof(T) > _size)
      return t;
    if(sizeof(T)==2) {
      T v = EEPROM.read(address+1);
      t = (v<<8) + EEPROM.read(address);
    } else if(sizeof(T)==4) {
      T v = EEPROM.read(address+3);
      t = (v<<8) + EEPROM.read(address+2);
      t = (v<<8) + EEPROM.read(address+1);
      t = (v<<8) + EEPROM.read(address);
    } else memcpy((uint8_t*) &t, _data + address, sizeof(T));
    return t;
  }

  template<typename T> 
  const T &put(int address, const T &t) {
    if (address < 0 || address + sizeof(T) > _size)
      return t;

    memcpy(_data + address, (const uint8_t*) &t, sizeof(T));
    _dirty = true;
    return t;
  }
*/

