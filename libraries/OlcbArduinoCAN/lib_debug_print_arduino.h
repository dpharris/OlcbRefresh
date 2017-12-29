//  Stuff that helps debugging
#ifndef ARD_ENABLE_DEBUG_PRINT_INC
#define ARD_ENABLE_DEBUG_PRINT_INC

// Uncomment the line below to enable Library Common Debug Printing
//#define ARD_DEBUG_PRINT_ENABLE
#ifdef ARD_DEBUG_PRINT_ENABLE
  #define ADEBUG(x) Serial.print(x)
  #define ADEBUGL(x) Serial.println(x);
  #define ADEBUG2(x,y) Serial.print(x,y);
#else
  #define ADEBUG(x)
  #define ADEBUGL(x)
  #define ADEBUG2(x,y)
#endif

#endif
