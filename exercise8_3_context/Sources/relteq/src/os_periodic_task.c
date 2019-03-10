/*
 * Periodic task implementation on top of RELTEQ.
 */

#include "os_periodic_task.h"

/*
 *********************************************************************************************************
 *                                           WAIT FOR NEXT TASK PERIOD
 *
 * Description: This function waits for the next task period.
 *              Note: this function does nothing (simply returns) for non-periodic tasks.
 *
 * Arguments  : reset_periods	-- When OS_TRUE, OSTaskPeriodWait() will reset the period overrun count.
 *                                  The next invocation of OSTaskPeriodWait() will thus wait again for a 
 *                                  new period, and not for the next missed one. Default: OS_FALSE
 *
 * Returns    : Number of periods overrun (i.e. returns 0 if called before the start of the next period).
 *
 */

INT16U OSTaskPeriodWaitEx(INT8U reset_periods) {
	OS_SEM_DATA sem_data;
	INT16U		overrun_count = 0;
	INT8U		err;
	
	if ( OSTCBCur == (OS_TCB *)0 ) {
		return 0;
	}
	if ( OSTCBCur->OSTCBPeriod == 0 ) {			/* Not a period task? */
		return 0;
	}
  
#if OS_LOG_EN > 0    
  logTaskCompletion(OSTCBCur);
#endif
  
	/* Since the task last called OSTaskPeriodWait(), how many period events were fired? */
	/* If larger than zero, we overran our period. */
	if ( OSSemQuery(OSTCBCur->OSTCBPeriodEvent, &sem_data) == OS_ERR_NONE ) {
		overrun_count = sem_data.OSCnt;
		if ( overrun_count > 0 ) {
			if ( reset_periods ) {
				/* Reset period events. */
				OSSemSet(OSTCBCur->OSTCBPeriodEvent, 0, &err);
			} else {
				/* Lower overrun count by one. */
				OSSemPend(OSTCBCur->OSTCBPeriodEvent, 0, &err);
			}
      
#if OS_LOG_EN > 0
			logTaskMissedDeadline(OSTCBCur);
      logTaskStart(OSTCBCur);
#endif
			/* We overran our period. Don't wait. Return overrun counter. */
			return overrun_count;
		}
	}
  
	/* Pend on periodic event. */
	do { 
		OSSemPend(OSTCBCur->OSTCBPeriodEvent, 0, &err);
	} while ( err != OS_ERR_NONE );
  
	/* We waited for one period event. The event count should again be zero. If not, */
	/* it means we've not been waken up in time, i.e. another task was blocking us.  */
  
	if ( OSSemQuery(OSTCBCur->OSTCBPeriodEvent, &sem_data) == OS_ERR_NONE ) {
		overrun_count = sem_data.OSCnt;
	}
  
	if ( reset_periods ) {
		/* Reset the period events, i.e. OSTaskPeriodWait() will wait for the next period */
		/* again (and not for the next missed one).                                       */
		OSSemSet(OSTCBCur->OSTCBPeriodEvent, 0, &err);
	}
  
#if OS_LOG_EN > 0    
  logTaskStart(OSTCBCur);
#endif
  
	return overrun_count;
}

INT16U OSTaskPeriodWait() {
	return OSTaskPeriodWaitEx(OS_FALSE);
}

/*$PAGE*/
/*
 *********************************************************************************************************
 *                                        MAKE TASK PERIODIC without offset
 *
 * Description: Set a period for a task. Also, modify a previously set period for a task.
 *
 * Arguments  : prio    is the priority of the task to make periodic. Note that you can explicitely modify
 *                      the current task without knowing its priority level by setting 'prio' to
 *                      OS_PRIO_SELF.
 *
 *			   period  required period in ticks
 *
 * Returns    : OS_ERR_NONE             if the call is successful
 *              OS_ERR_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
 *                                      (i.e. >= OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
 *              OS_ERR_TASK_NOT_EXIST   if the task you want to modify does not exist.
 *              Other                   see return values for OSSemSet
 */

INT8U  OSTaskSetPeriod (RelteqTask* task, INT16U period) {
	return OSTaskSetPeriodEx(task, period, 0);
}

/*
 *********************************************************************************************************
 *                                        MAKE TASK PERIODIC with offset
 *
 * Description: Set a period and offset for a task. Also, modify a previously set period for a task.
 *
 * Arguments  : prio    is the priority of the task to make periodic. Note that you can explicitely modify
 *                      the current task without knowing its priority level by setting 'prio' to
 *                      OS_PRIO_SELF.
 *
 *			   period  required period in ticks
 *        offset  required offset of the first task arrival in ticks
 *
 * Returns    : OS_ERR_NONE             if the call is successful
 *              OS_ERR_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
 *                                      (i.e. >= OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
 *              OS_ERR_TASK_NOT_EXIST   if the task you want to modify does not exist.
 *              Other                   see return values for OSSemSet
 */

INT8U  OSTaskSetPeriodEx (OS_TCB* task, INT16U period, INT16U offset) {
  INT8U      err;
  
#if OS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
  OS_CPU_SR  cpu_sr = 0;
#endif
  
#if OS_ARG_CHK_EN > 0
  if (task == (OS_TCB *)0) { /* Make sure task already exists at this priority. */
		return (OS_ERR_TASK_NOT_EXIST);
	}
#endif
  
  OS_ENTER_CRITICAL();
  
	if ( task->OSTCBPeriodEvent != NULL ) {							      /* Has a period already been set? */
		OSSemSet(task->OSTCBPeriodEvent, 0, &err);			/* Reset period event. */
		/* Note that we are ok if there is already a task waiting for the period semaphore, in this
     case it will be delayed until the new offset */
		if ( err != OS_ERR_NONE && err != OS_ERR_TASK_WAITING) {
			OS_EXIT_CRITICAL();
			return err;
		}
	} else { 
		task->OSTCBPeriodEvent = OSSemCreate(0);			/* Create new period event. */
		OSEventNameSet(task->OSTCBPeriodEvent, (INT8U*)"Period", &err);
	}
	task->OSTCBPeriod = period;								/* Set period */
	
	// make sure the semaphore was created
	if (!task->OSTCBPeriodEvent) {
    OS_EXIT_CRITICAL();
    return OS_ERR_RELTEQ_NO_MORE_PERIODIC_TASKS;	  
	}
	
	/* Insert first period event. If no offset is specified then treat it as 0 and handle directly. */
	if (offset == 0) {
	  offset = period;     
		OSSemPost(task->OSTCBPeriodEvent);
#if OS_LOG_EN > 0
		logTaskArrivalUnsafe(task);
#endif
  }
	
	// setup the period event in the appropriate relteq queue
	RelteqPeriodicTaskSetPeriod(task, period, offset);
	
  OS_EXIT_CRITICAL();
	return (OS_ERR_NONE);
}

/*
 * Generic wrapper for a periodic task. It takes a function pointer as argument, which specifies
 * the behavior of a single job of the periodic task.
 */

void OSPeriodicTask (void (*p_arg)(void)) {
#if OS_CRITICAL_METHOD == 3
  OS_CPU_SR     cpu_sr = 0;
#endif
  void (*function)(void) = p_arg;
  
  // wait for the arrival of the first periodic job
  OSTaskPeriodWait();
  
  // enter critical section here, to match the critical section at the end of the loop
  OS_ENTER_CRITICAL();
  while (OS_TRUE) {    
    OS_EXIT_CRITICAL();
    function();
    
#if OS_FPDS_EN > 0
    // if task is scheduled accrding to FPDS, then yield at least between jobs
    if (OSTCBCur->OSTCBSchedulePolicy == OS_TEV_SCHEDULING_FPPS) {
      OSTaskYield();
    }
#endif
    
    // wait until the next invocation
    OSTaskPeriodWait();
    
    OS_ENTER_CRITICAL();
  }
  OS_EXIT_CRITICAL();
}

/*
 * Convenience methods for creating periodic tasks. The function parameter specifies the body of
 * the task, which will be executed for every job.
 */

INT8U  OSTaskCreatePeriodic(void   (*function)(void),
                            INT16U period,
                            INT16U offset,
                            OS_STK  *ptos,
                            INT8U    prio) {
  INT8U err;
  
  // create the task with the periodic task wrapper around the job function
  err = OSTaskCreate(OSPeriodicTask, function, ptos, prio); ASSERT_OK(err);
  
  // make the task actually periodic
  err = OSTaskSetPeriodEx(OSTCBPrioTbl[prio], period, offset); ASSERT_OK(err);
  
  return OS_ERR_NONE;
}

INT8U  OSTaskCreatePeriodicExt (void   (*function)(void),
                                INT16U period,
                                INT16U offset,
                                OS_STK  *ptos,
                                INT8U    prio,
                                INT16U   id,
                                OS_STK  *pbos,
                                INT32U   stk_size,
                                void    *pext,
                                INT16U   opt) {
  INT8U err;
  
  // create the task with the periodic task wrapper around the job function
  err = OSTaskCreateExt(OSPeriodicTask, function, ptos, prio, id, pbos, stk_size, pext, opt); ASSERT_OK(err);
  
  // make the task actually periodic
  err = OSTaskSetPeriodEx(OSTCBPrioTbl[prio], period, offset); ASSERT_OK(err);
  
  return OS_ERR_NONE;  
}
