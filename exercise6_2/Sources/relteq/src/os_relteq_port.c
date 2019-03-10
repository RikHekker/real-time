
#include "os_cpu.h"
#include "os_cfg.h"
#include "os_relteq.h"
#include "ucos_ii.h"

//#include <includes.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <stdio.h>

/*
 * Make sure that OS_TASK_NAME_SIZE is defined
 */
 
#ifndef OS_TASK_NAME_SIZE
#define OS_TASK_NAME_SIZE 32
#endif

/*
 * RELTEQ timer management
 */

void RelteqPortHandleDelay(RelteqTask* task) {
  if ((task->OSTCBStat & OS_STAT_PEND_ANY) != OS_STAT_RDY) {
    task->OSTCBStat  &= ~(INT8U)OS_STAT_PEND_ANY;          /* Yes, Clear status flag   */
    task->OSTCBStatPend = OS_STAT_PEND_TO;                 /* Indicate PEND timeout    */
  } else {
    task->OSTCBStatPend = OS_STAT_PEND_OK;
  }
  if ((task->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY) {  /* Is task suspended?       */
    OSRdyGrp               |= task->OSTCBBitY;             /* No,  Make ready          */
    OSRdyTbl[task->OSTCBY] |= task->OSTCBBitX;
  }
}

void RelteqPortTaskPeriodArrived(RelteqTask* task) {
  OSSemPost((task)->OSTCBPeriodEvent);
#if OS_LOG_EN > 0
  logTaskArrival(task);
#endif
}

/*
 * RelteqPort_Init();
 */

void RelteqPort_Init(void) {
}

/* THIS IS FOR UCOS VERSION 2.86
void RelteqPortStringCopy(char* target, char* source, INT8U* perr) {
    INT8U      len;
#if OS_CRITICAL_METHOD == 3                          
    OS_CPU_SR  cpu_sr = 0;
#endif
    if (OSIntNesting > 0) {
        *perr = OS_ERR_NAME_SET_ISR;
        return;
    }

    *perr = OS_ERR_NONE;

    OS_ENTER_CRITICAL();
        
    len = OS_StrLen((INT8U*)source);                          
    if (len > (OS_TASK_NAME_SIZE - 1)) {             
        OS_EXIT_CRITICAL();
        *perr = OS_ERR_TASK_NAME_TOO_LONG;
        return;
    }

    (void)OS_StrCopy((INT8U*)target, (INT8U*)source);    
    OS_EXIT_CRITICAL();
}
*/

/* THIS IS FOR UCOS VERSION 2.88 */
void RelteqPortStringCopy(char* target, char* source, INT8U* perr) {
#if OS_CRITICAL_METHOD == 3                          
    OS_CPU_SR  cpu_sr = 0;
#endif
    *perr = OS_ERR_NONE;

    OS_ENTER_CRITICAL();
    target = source;        
    OS_EXIT_CRITICAL();
}


/*
 * Returns 1 if task is ready, 0 otherwise.
 */

#define RelteqPortTaskIsReady(task) ((OSRdyTbl[(task)->OSTCBY] & (task)->OSTCBBitX) != 0)
 
/*
 * Called within os_time.c. Inserts dummy events to represent long delays.
 */
 
void  OSLongTimeDly (INT32U ticks)
{
    INT8U      y;
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    RelteqEvent* event;
    
    if (OSIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        return;
    }
    if (ticks > 0) {                             /* 0 means no delay!                                  */
		    INT8U err;

        OS_ENTER_CRITICAL();
        y            =  OSTCBCur->OSTCBY;        /* Delay current task                                 */
        OSRdyTbl[y] &= ~OSTCBCur->OSTCBBitX;
#if OS_RELTEQ_HSF_EN > 0
        RelteqServerMakeTaskNotReady(OSTCBCur);
#endif
        if (OSRdyTbl[y] == 0) {
            OSRdyGrp &= ~OSTCBCur->OSTCBBitY;
        }
        event = RelteqEventCreate(kRelteqEventDelay, OSTCBCur);
			  RelteqQueueInsertEvent(RelteqGlobalQueue, event, ticks, &err);
        RelteqQueueUpdateEarliestTime(RelteqGlobalQueue);
        
        OS_EXIT_CRITICAL();
        OS_Sched();                              /* Find next task to run!                             */
    }
}

void RelteqPortPrint(char* s, ...) {}
void RelteqPortStringPrint(char* a, ...) {}

void ASSERT(int a, ...) {}
void ASSERT_OK(int a, ...) {}
void WARNING(char* a, ...) {}

#if OS_RELTEQ_PERIODIC_TASK_EN > 0
#include "os_periodic_task.c"
#endif

