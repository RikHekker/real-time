#include <ucos_ii.h>
#include "cpu.h"
#include "ATD_driver.h"
#include "Led_driver.h"

#define TASK_START_PRIO  0
#define TASK_1_PRIO     1
#define TASK_2_PRIO     2

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task1Stk[TASK_STK_SIZE];
OS_STK Task2Stk[TASK_STK_SIZE];

#define EmptyThreshold 0
#define LightThreshold 5

/*
 * Task1 first toggles the LED on port D28. It then senses the empty 
 * port PAD13, and turns on the LED on port D23 if the sensed value 
 * is larger than 0. Otherwise D23 is turned off.
 *
 * Note that reading from an empty port should always return 0.
 */

void Task1(void) {
  ToggleLed(LED_D28);
  SetLed(LED_D23, ATDReadChannel(PAD13) > EmptyThreshold);
}

/*
 * Task2 first toggles the LED on port D27. It then senses the light 
 * sensor on port PAD14, and turns on the LED on port D22 if the sensed 
 * value is larger than the LightThreshold. Otherwise D22 is turned off.
 */
    
void Task2(void) {
  ToggleLed(LED_D27);
  SetLed(LED_D22, ATDReadChannel(PAD14) > LightThreshold);
}

void TaskStart(void* p_args) {
  /* Do not remove this code. The timer must be started after OSStart() has
     started the start task. */
  CPUStartTimer(); 
  	             
	/* Do not remove this code. */
  OSTaskDel(TASK_START_PRIO);
}

void main(void) {
  /* Do not modify this code. uC/OS-II is initialized and 
     the startup task created. */
	OSInit();

  /* Here you can register your tasks */
	OSTaskCreatePeriodic(Task1, 500, 1, &Task1Stk[TASK_STK_SIZE-1], TASK_1_PRIO);
	OSTaskCreatePeriodic(Task2, 500, 0, &Task2Stk[TASK_STK_SIZE-1], TASK_2_PRIO);

	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE-1], TASK_START_PRIO);
	OSStart();
}
