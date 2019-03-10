/*
 * 2IN60, Exercises 2
 */
 
#include "cpu.h"
#include "ATD_driver.h"
#include "Led_driver.h"

#define LightThreshold 20
 
/*
 * Task1 toggles the LED on port D23.
 */
    
void Task1(void)  {
	/* Implement the task here */
}


/*
 * Task2 senses the light sensor on the port PAD14, and toggles LEDs on ports
 * D22, D24, and D26 if the sensed value is larger than the LightThreshold.
 * Otherwise, it does nothing.
 */
    
void Task2(void)  {
	/* Implement the task here */
}


int main(void) { 
  /* Initialize the system */
  CPUInitSystem();
  InitLeds ();
  _EINT(); /* Enable interrupts */
  CPUStartTimer(); 
  
  /* Do the work */

  return (0);
}
