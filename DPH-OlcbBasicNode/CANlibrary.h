/*
 *  CANlibrary.h
 *  Selects appropriate CAN library for specific processor present
 */

/*
// Tinys
#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  #include <CAN.h>

// **8 ... 168 and 328 Arduinos
#elif defined(__AVR_ATmega8__)  || defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__) || \
   defined(__AVR_ATmega168__) ||defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328P__) \

#include <can.h>

// Mega 16, 32
#elif defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) 
  #include <CAN.h>
  
// Mega 128, 1280 & 2560
#elif defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #include <CAN.h>
  
// Sanguino
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
#include <can.h>

// AT90CAN
#elif defined(__AVR_AT90CAN32__) || defined(__AVR_AT90CAN64__) || defined(__AVR_AT90CAN128__)
  #include <CAN.h>

// Teensies
#elif defined(__AVR_ATmega32U4__)
  // Teensy 2.0
  #ifdef CORE_TEENSY 
  #include <CAN.h>
  // Teensy
  #else
  #define CHIPSET ATmega_32U4_B 
  #endif
// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
  #include <CAN.h>
// Teensy ARM
#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
  #include "CANTeesyARM.h"

// Teensy 3.5 & 3.6

// Tiva Lauchpads
#elif defined(__LM4F120H5QR__) || defined (__TM4C123GH6PM__)
  #include "CANTiva.h"

#endif
*/
