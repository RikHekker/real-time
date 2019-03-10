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
 * - RTI interrupt is set to expire with frequency of 2 kHz. Hence, 
 *   a task period of 2000 ticks corresponds to 1s.
 */
  
#define TaskStartPrio  2
#define Task1Prio      4
#define MutexPrio      5
#define Task2Prio      6
#define Task3Prio      8

#define Task2Period    4000
#define Task3Period    4000

#define Task2Phasing   1000
#define Task3Phasing   0

/* Global variables storing the sensed values */

int empty;
int light;

/* Global variables for implementing the precedence constraints */

int emptyReady = 0;
int lightReady = 0;

	
/*
 * Task1 first toggles the LED on port D28. It then waits for the sensed 
 * values on the empty and light ports. If the sensed light value
 * is larger than the LightThreshold, and empty value smaller than the 
 * EmptyThreshold, then it turns on the LED on port D22. 
 * Otherwise it turns D22 off. 
 */

void Task1(void* p_arg) {
    
  while (OS_TRUE) {
    /* Toggle a led, to see that this task has indeed started. */
    ToggleLed(LED_D28);
    
    /* Wait until the empty value is available. */
  	while (!emptyReady) {}
  	emptyReady = 0;
  	
    /* Wait until the light value is available. */
  	while (!lightReady) {}
  	lightReady = 0;
  	
  	/* Perform some work, depending on the sensed light and empty value. */
  	SetLed(LED_D22, light > LightThreshold && empty < EmptyThreshold);
  }
}

/*
 * Task2 first toggles the LED on port D27. It then senses the empty 
 * port PAD13, stores the value in a global variable, and indicates
 * that the reading of the empty port is ready (possibly trigerring
 * Task1).
 */
    
void Task2(void) {
  /* Toggle a led, to see that this task has indeed started. */
	ToggleLed(LED_D27);

  /* Read the empty pin connected to the ATD converter on port PAD13. */
	empty = ATDReadChannel(PAD13);

  /* Indicate that the reading is ready. */		
  emptyReady = 1;
}

/*
 * Task3 first toggles the LED on port D26. It then senses the light 
 * sensor on port PAD14, stores the value in a global variable, and 
 * indicates that the reading of the light is ready (possibly trigerring
 * Task1).
 */
    
void Task3(void) {
  /* Toggle a led, to see that this task has indeed started. */
	ToggleLed(LED_D26);

  /* Readthe light sensor connected to the ATD converter on port PAD14. */
	light = ATDReadChannel(PAD14);

  /* Indicate that the reading is ready. */		
	lightReady = 1;
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
  INT8U err;
  OS_EVENT* m;
  
  /* Initialize uC/OS-II and create the startup task
     It should not be neccesary to modify this function.
   */
  OSInit();
  
  /* Here you should create any synchronization primitives. */
  m = OSMutexCreate(MutexPrio, &err);
  if (err == OS_ERR_NONE) {
    ATDInit(m);
  }

  /* Here you can register your tasks */
  OSTaskCreate(Task1, 0, &Task1Stk[TASK_STK_SIZE-1], Task1Prio);
  OSTaskCreatePeriodic(Task2, Task2Period, Task2Phasing, &Task2Stk[TASK_STK_SIZE-1], Task2Prio);
  OSTaskCreatePeriodic(Task3, Task3Period, Task3Phasing, &Task3Stk[TASK_STK_SIZE-1], Task3Prio);
    
  /* Do not remove this code. */
  OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE-1], TaskStartPrio);
  OSStart();
}
