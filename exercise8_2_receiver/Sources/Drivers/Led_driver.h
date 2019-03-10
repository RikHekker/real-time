#ifndef __Led_h
#define __Led_h

/*
 * Led interface
 *
 * MH, 15-08-2011
 */

#include "cpu.h"

#define LED_D22 PT1AD_PT1AD0                    
#define LED_D23 PT1AD_PT1AD1                    
#define LED_D24 PT1AD_PT1AD2                    
#define LED_D25 PT1AD_PT1AD3
#define LED_D26 PTT_PTT0                     
#define LED_D27 PTT_PTT1                     
#define LED_D28 PTT_PTT2                      
#define LED_D29 PTT_PTT3     
              
#define LED_SWAP(x,y) { (x)^=1; (y)^=1; };


#define ON  (uint8_t) 1
#define OFF (uint8_t) 0

void InitLeds   (void);
#define SetLed(led, on) ((led) = !(on))
#define ToggleLed(led) ((led) ^= 1)  
void PrintShortToLeds(uint8_t x);
void PrintIntToLeds(uint16_t x);

#endif
