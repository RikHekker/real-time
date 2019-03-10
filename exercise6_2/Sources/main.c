#include <ucos_ii.h>
#include "cpu.h"
#include "ATD_driver.h"
#include "Led_driver.h"

#define TASK_STK_SIZE 128
OS_STK TaskStartStk[TASK_STK_SIZE];

/*
 * Task parameters.
 *
 * Note that:
 * - lower number means higher priority.
 * - RTI interrupt is set to expire with frequency of 2 kHz. Hence, 
 *   a task period of 400 ticks corresponds to 200ms.
 */
  
#define TaskStartPrio  2
	
/*
 * BusyWait() performs a busy wait for a given number of ticks specified
 * by the delay parameter.
 *
 * Note that depending on when during a tick this function is called, 
 * it will busy wait for time t, where t is in the interval [delay - 1, delay].
 */
 	
void BusyWait(INT32U delay) {
  INT32U now = OSTimeGet();
  while (OSTimeGet() < now + delay) {}
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
  OSInit();
  
  /* Here you should create any synchronization primitives. */

  /* Here you can register your tasks */
    
  /* Do not remove this code. */
  OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE-1], TaskStartPrio);
  OSStart();
}
