/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                  MUTUAL EXCLUSION SRP SEMAPHORE MANAGEMENT
*
*
* File    : OS_MUTEXSRP.C
* By      : Martijn van den Heuvel
*					:	SAN group, Dep. of Mathematics and Computer Science, Eindhoven University of Technology
* Version : V2.86
*
*********************************************************************************************************
*/

#ifndef  OS_MASTER_FILE
#include <ucos_ii.h>
#endif

/*
*********************************************************************************************************
*                                            LOCAL CONSTANTS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            LOCAL VARS
*********************************************************************************************************
*/

#if OS_MUTEX_SRP_EN > 0
Resource  OSSRPSystemCeiling[MAX_RESOURCES];               /* SRP system ceiling stack                 */

/*$PAGE*/
/*
*********************************************************************************************************
*                          INITIALIZE DATASTRUCTURES FOR STACK RESOURCE POLICY
*
* Description: This function initializes the (stack) datastructure for the SRP protocol.
*
* Arguments  : void
* Returns    : void 
*
*********************************************************************************************************
*/
void OS_MutexSRPInit(void) 
{
  INT8U i;

  /* The first element of the system ceiling stack is used to indicate that no resource is locked  */
  OSSRPSystemCeiling[0].ceiling = MAX_INT8U; 
  OSSRPSystemCeiling[0].lockingTask = 0;
  OSSRPSystemCeiling[0].previous = (void*)0;

  /* Initialize Resource reference counters and pointers */
  OSSRPResourceCounter = 0;
  OSSRPTopOfStack = OSSRPSystemCeiling;
  OSSRPEmpty = &OSSRPSystemCeiling[1];
  
  /* Keep track of empty resources (dependent on the maximum stack size) */
  for (i = 1; i < MAX_RESOURCES -1; ++i){
    OSSRPSystemCeiling[i].previous = &OSSRPSystemCeiling[i+1];
  }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                  CREATE A SRP MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function creates a mutual exclusion semaphore based on SRP.
*
* Arguments  : prio          is the ceiling to use when accessing the mutual exclusion semaphore.  In
*                            other words, when the semaphore is acquired the system ceiling is raised
*                            to this value, such that all tasks with lower priority than the current 
*                            ceiling are prevented to preempt.
*                            It is assumed that you will specify a priority that is LOWER OR EQUAL in 
*                            value than ANY of the tasks competing for the mutex.
*
*              perr          is a pointer to an error code which will be returned to your application:
*                               OS_ERR_NONE         if the call was successful.
*                               OS_ERR_CREATE_ISR   if you attempted to create a MUTEX from an ISR
*                               OS_ERR_PEVENT_NULL  No more resource blocks available.
*                               OS_ERR_PRIO_INVALID if the priority you specify is higher that the
*                                                   maximum allowed (i.e. > OS_LOWEST_PRIO)
*
* Returns    : != (void *)0  is a pointer to the resource clock (Resource) associated with the
*                            created mutex.
*              == (void *)0  if an error is detected.
*
*********************************************************************************************************
*/
#if OS_MUTEX_SRP_EN > 0
Resource  *OSMutexSRPCreate (INT8U prio, INT8U *perr)
{
    Resource  *pevent;
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif

#if OS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return ((Resource *)0);
    }
    if (prio >= OS_LOWEST_PRIO) {                          /* Validate Ceiling                         */
        *perr = OS_ERR_PRIO_INVALID;
        return ((Resource *)0);
    }
#endif
    if (OSIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = OS_ERR_CREATE_ISR;                         /* ... can't CREATE mutex from an ISR       */
        return ((Resource *)0);
    }
    OS_ENTER_CRITICAL();
    OSSRPResourceCounter++;                                /* Get next free resource block             */
    if (OSSRPResourceCounter > MAX_RESOURCES) {            /* See if an ECB was available              */
        *perr              = OS_ERR_PEVENT_NULL;           /* No more resource blocks                  */
        OS_EXIT_CRITICAL();
        return (Resource*)0;
    } 
    pevent = OSSRPEmpty;                                   /* Update the empty list                    */
    OSSRPEmpty = OSSRPEmpty->previous;
    pevent->previous = 0;
#if OS_LOG_EN > 0 && OS_LOG_MUTEX_EN > 0
    logMutexCreated(pevent);
#endif
    pevent->ceiling = prio;                                /* Set the resource ceiling                 */
    pevent->lockingTask = 0;                               /* No task has locked the resource          */
    pevent->previous = 0;                                  /* Not initialized on the ceiling stack     */
    *perr                  = OS_ERR_NONE;
    OS_EXIT_CRITICAL();
    return (pevent);
}
#endif

/*
*********************************************************************************************************
*                                          DELETE A MUTEX
*
* Description: This function deletes a mutual exclusion semaphore and assumes no tasks pending on the it.
*
* Arguments  : pevent        is a pointer to the resource block associated with the desired mutex.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE             The call was successful and the mutex was deleted
*                            OS_ERR_DEL_ISR          If you attempted to delete the MUTEX from an ISR
*                            OS_ERR_PEVENT_NULL      If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the mutex was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the mutex MUST check the return code of OSMutexPend().
*
*              2) Because the resource will be deleted independent of potential users, you MUST be careful
*                 because the resource(s) will no longer be guarded by the mutex.
*
*********************************************************************************************************
*/
#if OS_MUTEX_SRP_EN > 0
Resource  *OSMutexSRPDel (Resource *pevent, INT8U *perr)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif

#if OS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return (pevent);
    }
    if (pevent == (Resource*)0) {                         /* Validate 'pevent'                        */
        *perr = OS_ERR_PEVENT_NULL;
        return (pevent);
    }
#endif
  OS_ENTER_CRITICAL();
  pevent->previous = OSSRPEmpty;
  OSSRPEmpty = pevent;
  OSSRPEmpty->lockingTask = 0;
  OS_EXIT_CRITICAL();
  return (Resource*)0;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                  PEND ON SRP MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function waits for a mutual exclusion semaphore based on SRP.
*
* Arguments  : pevent        is a pointer to the resource block associated with the desired mutex.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*                               OS_ERR_NONE        The call was successful and your task owns the mutex
*                               OS_ERR_PEVENT_NULL 'pevent' is a NULL pointer
*                               OS_ERR_PEND_ISR    If you called this function from an ISR and the result
*                                                  would lead to a suspension.
*                               OS_ERR_PEND_LOCKED If you called this function when the scheduler is locked
*
* Returns    : none
*
* Note(s)    : 1) You MUST NOT change the priority of the task that owns the mutex
*
*********************************************************************************************************
*/
#if OS_MUTEX_SRP_EN > 0
void  OSMutexSRPPend (Resource *pevent, INT8U *perr)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif
#if (OS_LOG_EN > 0) && OS_LOG_MUTEX_EN > 0 && (OS_LOG_MUTEX_VERBOSE_EN > 0)
    logMessage("task %s acquiring mutex %s", OSTCBCur->OSTCBTaskName, pevent->name);
#endif
#if OS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return;
    }
    if (pevent == (Resource *)0) {                         /* Validate 'pevent'                        */
        *perr = OS_ERR_PEVENT_NULL;
        return;
    }
#endif
    if (OSIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = OS_ERR_PEND_ISR;                           /* ... can't PEND from an ISR               */
        return;
    }
    if (OSLockNesting > 0) {                               /* See if called with scheduler locked ...  */
        *perr = OS_ERR_PEND_LOCKED;                        /* ... can't PEND when locked               */
        return;
    }
    /* In case of SRP we know that the preemption threshold exceeds the current system ceiling, so we 
       cannot block on the mutex. So we simply add it to the system ceiling stack.                     */
    OS_ENTER_CRITICAL();
    if (pevent->ceiling < OSSRPTopOfStack->ceiling) {
      pevent->previous = (void*)OSSRPTopOfStack;            /* Save the top of the stack in a structure  */
      OSSRPTopOfStack = pevent;                             /* Store the new system ceiling              */
    }
    pevent->lockingTask = OSTCBCur->OSTCBPrio;              /* Store the priority of the locking task    */
#if OS_LOG_EN > 0 && OS_LOG_MUTEX_EN > 0
    logMutexAcquiredUnsafe(pevent, OSTCBCur);
#endif
    OS_EXIT_CRITICAL();
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                  POST TO A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function signals a mutual exclusion semaphore
*
* Arguments  : pevent              is a pointer to the resource block associated with the desired mutex.
*
* Returns    : OS_ERR_NONE             The call was successful and the mutex was signaled.
*              OS_ERR_EVENT_TYPE       If you didn't pass a pointer to a mutex
*              OS_ERR_PEVENT_NULL      'pevent' is a NULL pointer
*              OS_ERR_POST_ISR         Attempted to post from an ISR (not valid for MUTEXes)
*              OS_ERR_NOT_MUTEX_OWNER  The task that did the post is NOT the owner of the MUTEX.
*
*********************************************************************************************************
*/

#if OS_MUTEX_SRP_EN > 0
INT8U  OSMutexSRPPost (Resource *pevent)
{
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    OS_CPU_SR  cpu_sr = 0;
#endif
#if OS_LOG_EN > 0 && OS_LOG_MUTEX_EN > 0 && OS_LOG_MUTEX_VERBOSE_EN > 0
    logMessage("task %s releasing mutex %s", OSTCBCur->OSTCBTaskName, pevent->name);
#endif
    if (OSIntNesting > 0) {                           /* See if called from ISR ...                    */
        return (OS_ERR_POST_ISR);                     /* ... can't POST mutex from an ISR              */
    }
    if (OSTCBCur->OSTCBPrio != pevent->lockingTask) { /* The task that tries to release the mutex      */
      return (OS_ERR_NOT_MUTEX_OWNER);                /* doen not own the mutex                        */
    }
#if OS_ARG_CHK_EN > 0
    if (pevent == (Resource *)0) {                    /* Validate 'pevent'                             */
        return (OS_ERR_PEVENT_NULL);
    }
#endif
    /* In case of SRP we simply remove the mutex from the system ceiling stack, and reschedule
       in case a higher priority task arrived and was "blocked" by the current task.                   */
    OS_ENTER_CRITICAL();
    if (OSSRPTopOfStack == pevent) {
      OSSRPTopOfStack = (Resource*)(pevent->previous);
      pevent->previous = 0;
    }
    pevent->lockingTask = 0;                          /* Clear the priority of the locking task        */
#if OS_LOG_EN > 0 && OS_LOG_MUTEX_EN > 0
    logMutexReleasedUnsafe(pevent, OSTCBCur);
#endif
    OS_EXIT_CRITICAL();
    OS_Sched();
    return (OS_ERR_NONE);
}
#endif

