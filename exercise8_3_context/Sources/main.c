#include <ucos_ii.h>
#include "cpu.h"
#include "Led_driver.h"
#include "ATD_driver.h"         
#include "Drivers/CAN_driver.h"
#include "Drivers/CAN/buffer.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task1Stk[TASK_STK_SIZE];
OS_STK Task2Stk[TASK_STK_SIZE];

/*
 * Task parameters.
 *
 * Note that:
 * - lower number means higher priority.
 * - RTI interrupt is set to expire with frequency of 1 kHz.
 */
  
#define TaskStartPrio  2
#define Task1Prio      6
#define Task1Period 1000
#define Task1Phasing   0
#define Task2Prio      8

/*
 * Message id which Task1 is going to send.
 */
 
#define LightId 0xA8

/*
 * List of message ids which Task2 is going to receive.
 */
 
#define LedId 0xA9
INT32U idlist2[1] = { LedId };

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
 * Task1 is triggered periodically. It first toggles the LED on port D27. 
 * It then reads the light sensor connected to the ATD converter on port
 * PAD14, and sends the value via a CAN message with id equal to LightId.
 */ 

void Task1(void) {}
 
/*
 * Task2 is triggered by an arriving message with id equal to LedId.
 * Upon message arrival it first toggles the LED on port D26, and then,
 * if there were no errors with the message, it toggles the led on
 * port D25.
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
	OSTaskCreatePeriodic(Task1, Task1Period, Task1Phasing, &Task1Stk[TASK_STK_SIZE-1], Task1Prio);
	OSTaskCreate(Task2, 0, &Task2Stk[TASK_STK_SIZE-1], Task2Prio);

  /* Initialise the CAN driver. */	

  /* Start the application. */
	OSStart();
}
