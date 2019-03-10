#include <ucos_ii.h>
#include "cpu.h"
#include "ATD_driver.h"
#include "Led_driver.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task1Stk[TASK_STK_SIZE];
OS_STK Task2Stk[TASK_STK_SIZE];
OS_STK Task3Stk[TASK_STK_SIZE];

#define EmptyThreshold 5
#define LightThreshold 20

/*
 * Task parameters.
 *
 * Note that:
 * - lower number means higher priority.
 * - task periods and phasings are in ticks
 * - RTI interrupt is set to expire with frequency of 2 kHz. Hence, 
 *   a task period of 400 ticks corresponds to 200ms.
 */
  
#define TaskStartPrio  2
#define Task1Prio      4
#define Task2Prio      6
#define Task3Prio      8

#define Task1Period    400
#define Task2Period    2000
#define Task3Period    2000

#define Task1Phasing   2
#define Task2Phasing   1
#define Task3Phasing   0

OS_EVENT* mutex;

/*
 * Task1 checks whether it is executed strictly periodically. If it 
 * observes start jitter, then it toggles the LED on port D26.
 * 
 * Note that start jitter is defined as the difference between the relative start times
 * of two consecutive jobs of a task. The relative start time of a job is the time
 * difference between the activation of the job and the time that it starts executing.
 */

INT32U lastTime = 0;

void Task1(void) {
  INT32U now = OSTimeGet();
  if (lastTime > 0 && now - lastTime != Task1Period) {
    ToggleLed(LED_D26);
  }
  lastTime = now;
}
 /* port PAD13, and turns on the LED on port D23 if the sensed value 
 * is larger than 0. Otherwise D23 is turned off.
 *
 * Note that reading from an empty port should always return a value
 * close to 0 (due to static interference the value may be slightly 
 * larger than 0).
 */

void Task2(void) {
  INT8U err;
  ToggleLed(LED_D28);
  OSMutexPend(mutex,0,&err);
  SetLed(LED_D23, ATDReadChannel(PAD13) > EmptyThreshold);
  OSMutexPost(mutex);
}

/*
 * Task3 first toggles the LED on port D27. It then senses the light 
 * sensor on port PAD14, and turns on the LED on port D22 if the sensed 
 * value is larger than the LightThreshold. Otherwise D22 is turned off.
 */
    
void Task3(void) {
  INT8U err;
  ToggleLed(LED_D27);
  OSMutexPend(mutex,0,&err);
  SetLed(LED_D22, ATDReadChannel(PAD14) > LightThreshold);
  OSMutexPost(mutex);
}

void TaskStart(void* p_args) {
  /* Do not remove this code. The timer must be started after OSStart() has
     started the start task. */
  CPUStartTimer(); 

	/* Do not remove this code. This task must remain active during the entire run. */
  for (;;){
    OSTimeDly(OS_TICKS_PER_SEC); 
  }
}

void main(void) {  
  /* Initialize uC/OS-II and create the startup task
     It should not be neccesary to modify this function.
   */
   INT8U err;
  OSInit();
  
  /* Here you should create any synchronization primitives. */
  
  mutex=OSMutexCreate(Task2Prio, &err);

  /* Here you can register your tasks */
  OSTaskCreatePeriodic(Task1, Task1Period, Task1Phasing, &Task1Stk[TASK_STK_SIZE-1], Task1Prio);
  OSTaskCreatePeriodic(Task2, Task2Period, Task2Phasing, &Task2Stk[TASK_STK_SIZE-1], Task2Prio);
  OSTaskCreatePeriodic(Task3, Task3Period, Task3Phasing, &Task3Stk[TASK_STK_SIZE-1], Task3Prio);
  
  /* Do not remove this code. */
  OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE-1], TaskStartPrio);
  OSStart();
}
