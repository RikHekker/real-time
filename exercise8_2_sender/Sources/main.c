#include <ucos_ii.h>
#include "cpu.h"
#include "Led_driver.h"
#include "ATD_driver.h"         
#include "Drivers/CAN_driver.h"
#include "Drivers/CAN/buffer.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task1Stk[TASK_STK_SIZE];

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

/*
 * Message id which Task1 is going to send.
 */
 
#define LightId 0xA8

/*
 * Task1 is triggered periodically. It first toggles the LED on port D29. 
 * It then reads the light sensor connected to the ATD converter on port
 * PAD14, and sends a message with id equal to LightId via the CAN with
 * the payload containing an array of two INT8U values, with the first value
 * containing the light sensor value, and the second value containing
 * the light sensor value multiplied by two.
 */ 

void Task1(void) {
  INT8U data[2];
  INT8U value;
  ToggleLed(LED_D29);
  value=ATDReadChannel(PAD14);
  data[0]=value;
  data[1]=2*value;
  CANSendFrame(CANId11(LightId),2,data);
 }
 
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

  /* Initialise the CAN driver. */
  CANInit();
  CANConfigureBaudrate(125000,0);
  CANStart();	

  /* Start the application. */
	OSStart();
}
