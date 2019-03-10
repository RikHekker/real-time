/*
 * Led driver
 *
 * MH, 15-08-2011
 */
 
#ifndef __Led_h
#define __Led_h

#include "cpu.h"

/* 
 * Identifiers for the different leds. These should be used as parameters
 * to the functions operating on LEDs.
 */

#define LED_D22 PT1AD_PT1AD0                    
#define LED_D23 PT1AD_PT1AD1                    
#define LED_D24 PT1AD_PT1AD2                    
#define LED_D25 PT1AD_PT1AD3
#define LED_D26 PTT_PTT0                     
#define LED_D27 PTT_PTT1                     
#define LED_D28 PTT_PTT2                      
#define LED_D29 PTT_PTT3     

/* Constants defining the on and off states. */

#define ON  (uint8_t) 1
#define OFF (uint8_t) 0

/*
 * Initialize the LEDs. It resets the LEDs and should be called
 * during system initialization.
 */

void InitLeds   (void);

/* 
 * Turn a LED on or off. E.g. SetLed(LED_D28, ON) turns on the led
 * on port D28.
 */
 
#define SetLed(led, on) ((led) = !(on))

/* 
 * Togle a LED. E.g. Toggle(LED_D28) toggles the led on port D28.
 */

#define ToggleLed(led) ((led) ^= 1) 

/*
 * Swap two leds.
 */
              
#define SwapLeds(x,y) { (x)^=1; (y)^=1; };

/*
 * Use the LEDs to visualize an unsigned 8-bit integer value in binary format.
 */ 
 
void PrintShortToLeds(uint8_t x);

/*
 * Use the LEDs to visualize an unsigned 16-bit integer value in binary format.
 */ 

void PrintIntToLeds(uint16_t x);

#endif
