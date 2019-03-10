#include <ucos_ii.h>
#include "cpu.h"
#include "Led_driver.h"         
#include "Drivers/CAN_driver.h"
#include "Drivers/CAN/buffer.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task2Stk[TASK_STK_SIZE];

/*
 * Task parameters.
 *
 * Note that:
 * - lower number means higher priority.
 * - RTI interrupt is set to expire with frequency of 1 kHz.
 */
  
#define TaskStartPrio  2
#define Task2Prio      8

#define LightThreshold 5

/*
 * List of message ids which Task2 is going to receive.
 */
 
#define LightId 0xA8
INT32U idlist2[1] = { LightId };

/*
 * The queue, where the CAN driver is going to write pointers
 * to messages destined for Task2.
 */

OS_EVENT* queue2;

/* 
 * Memory space where queue2 is going to store the pointers
 * to messages in the internal buffer of the CAN driver.
 */
 
void* queue2buf[10];

/*
 * Task2 is triggered by an arriving message with id equal to LightId.
 * Upon message arrival it first toggles the LED on port D22. It then
 * extracts the data from the incoming message, assuming it is an array
 * containing two INT8U values, and sets:
 *   LED on port D23 if and only if the first value exceeds the LightThreshold,
 *   LED on port D24 if and only if the second value exceeds the LightThreshold.
 */
 
void Task2(void* pArg) {}

void TaskStart(void *_unused) {
  /* Do not remove this code. The timer must be started after OSStart() has
     started the start task. */
  CPUStartTimer(); 
  
	/* Do not remove this code. */
  OSTaskDel(TaskStartPrio);
}

void main(void) {
  /* Initialize uC/OS-II and create the startup task. */
	OSInit();
	OSTaskCreate(TaskStart, 0, &TaskStartStk[TASK_STK_SIZE-1], TaskStartPrio);

  /* Here you can register your tasks */
	OSTaskCreate(Task2, 0, &Task2Stk[TASK_STK_SIZE-1], Task2Prio);

  /* Initialise the CAN driver. */	

  /* Start the application. */
	OSStart();
}
