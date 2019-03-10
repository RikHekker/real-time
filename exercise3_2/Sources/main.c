#include <ucos_ii.h>
#include "cpu.h"
#include "ATD_driver.h"
#include "Led_driver.h"

#define TaskStartPrio  0
#define Task1Prio     1
#define Task2Prio     2

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task1Stk[TASK_STK_SIZE];
OS_STK Task2Stk[TASK_STK_SIZE];

#define EmptyThreshold 5
#define LightThreshold 20

/*
 * Task1 first toggles the LED on port D28. It then senses the empty 
 * port PAD8, and turns on the LED on port D23 if the sensed value 
 * is larger than 0. Otherwise D23 is turned off.
 *
 * Note that reading from an empty port should always return 0.
 */

void Task1(void) {
  ToggleLed(LED_D28);
  SetLed(LED_D23, ATDReadChannel(PAD8) > EmptyThreshold);
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
  /* Do not remove this code. The timer must be started after OSStart().
   * Since OSStart() never returns, the timer must be started from 
   * another task, e.g. TaskStart. 
   */
  CPUStartTimer(); 
  	             
	/* Do not remove this code. */
  OSTaskDel(TaskStartPrio);
}

void main(void) {
	OSInit();

  /* Here you can register your tasks */
	OSTaskCreatePeriodic(Task1, ?, ?,  &Task1Stk[TASK_STK_SIZE-1], ?);
	OSTaskCreatePeriodic(Task2, ?, ?, &Task2Stk[TASK_STK_SIZE-1], ?);

	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE-1], TaskStartPrio);
	OSStart();
}
