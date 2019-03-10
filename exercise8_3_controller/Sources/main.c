#include <ucos_ii.h>
#include "cpu.h"
#include "Led_driver.h"
#include "ATD_driver.h"         
#include "Drivers/CAN_driver.h"
#include "Drivers/CAN/buffer.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task3Stk[TASK_STK_SIZE];

/*
 * Task parameters.
 *
 * Note that:
 * - lower number means higher priority.
 * - RTI interrupt is set to expire with frequency of 1 kHz.
 */
  
#define TaskStartPrio  2
#define Task3Prio      7

#define LightThreshold 5

/*
 * List of message ids which Task3 is going to receive.
 */
 
#define LightId 0xA8
INT32U idlist3[1] = { LightId };

/*
 * Message id which Task3 is going to send.
 */
 
#define LedId 0xA9

/*
 * The queue, where the CAN driver is going to write pointers
 * to messages destined for Task2.
 */

OS_EVENT* queue3;

/* 
 * Memory space where queue2 is going to store the pointers
 * to messages in the internal buffer of the CAN driver.
 */
 
void* queue3buf[10];

/*
 * Task3 is triggered by an arriving message with id equal to LightId.
 * Upon message arrival it first toggles the LED on port D22. It then
 * extracts the data from the incoming message and sets the LED on
 * port D23 if and only if the value exceeds the LightThreshold. 
 * If the threshold was exceeded, then it delays for 200 ms and sends
 * a message with id equal to LedId and with no data (i.e. a simple
 * signal).   
 */ 
 
void Task3(void* pArg) {}
 
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
	OSTaskCreate(Task3, 0, &Task3Stk[TASK_STK_SIZE-1], Task3Prio);

  /* Initialise the CAN driver. */

  /* Start the application. */
	OSStart();
}
