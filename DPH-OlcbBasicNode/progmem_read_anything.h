#include <Arduino.h>  // for type definitions
template <typename T> void PROGMEM_readAnything (const T * sce, T& dest) {
  memcpy_P (&dest, sce, sizeof (T));
}

template <typename T> T PROGMEM_getAnything (const T * sce)  {
  static T temp;
  memcpy_P (&temp, sce, sizeof (T));
  return temp;
}

// flashGet
template <typename T> T flashGet(const T* sce)
  static T r;
  memcpy_P (&r, sce, sizeof (T));
  return r;
}
  

#if 1==0
//example ===========================
  #include <PROGMEM_readAnything.h>
  const int NUMBER_OF_ELEMENTS = 10;
  const float table[NUMBER_OF_ELEMENTS] PROGMEM = { 1.0, 34.234, 324.234, 23.1, 52.0, 3.6, 5.6, 42.42, 1908, 23.456 } ;
  void setup() {
    Serial.begin(115200);
    for (size_t i = 0; i < NUMBER_OF_ELEMENTS; i++)   {
      float thisOne;
      PROGMEM_readAnything (&table[i], thisOne);
      Serial.println(thisOne);
    }  // end of for loop
  }  // end of setup
  void loop() {}
#endif

#if 1==0

  #include <PROGMEM_readAnything.h>
  const int NUMBER_OF_ELEMENTS = 10;
  const float  table[NUMBER_OF_ELEMENTS] PROGMEM = { 1.0, 34.234, 324.234, 23.1, 52.0, 3.6, 5.6, 42.42, 1908, 23.456 } ;
  void setup() {
    Serial.begin(115200);
    for (size_t i = 0; i < NUMBER_OF_ELEMENTS; i++) 
      Serial.println(PROGMEM_getAnything (&table[i]));
  }  // end of setup
  void loop() {}
