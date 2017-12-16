#define LARGE

#include <EEPROM.h>
#include "read.h"


void setup() {
  uint8_t v8, r8;
  uint16_t v16, r16;
  uint32_t v32, r32;
  uint8_t v[8], r[8];
  struct {
    uint32_t a;
    uint16_t b;
    char c[8];
  } s;
  //read(0, v8);
  r8 = read(0);
  r16 = read(0);
  r32 = read(0);
  read(0, r);
  read(0, &s);

}

void loop() {
  // put your main code here, to run repeatedly:

}
