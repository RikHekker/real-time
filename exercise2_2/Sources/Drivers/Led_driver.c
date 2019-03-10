#ifndef __Led_c
#define __Led_c

/*
 * Led driver
 *
 * MH, 15-08-2011
 */

#include "Led_driver.h"


void InitLeds () {
  PrintShortToLeds(0);
}

void PrintShortToLeds(uint8_t x) {
  SetLed(LED_D22, x & (1 << 0));
  SetLed(LED_D23, x & (1 << 1));
  SetLed(LED_D24, x & (1 << 2));
  SetLed(LED_D25, x & (1 << 3));
  SetLed(LED_D26, x & (1 << 4));
  SetLed(LED_D27, x & (1 << 5));
  SetLed(LED_D28, x & (1 << 6));
  SetLed(LED_D29, x & (1 << 7));    
}

void PrintIntToLeds(uint16_t x) {
  if (x > 255) {
    PrintShortToLeds(255);
  } else {
    PrintShortToLeds((uint8_t)x); 
  }
}


#endif
