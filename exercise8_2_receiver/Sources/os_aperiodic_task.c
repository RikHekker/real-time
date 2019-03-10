#include "ucos_ii.h"

/*
 * Generic wrapper for a periodic task. It takes a function pointer as argument, which specifies
 * the behavior of a single job of the periodic task.
 */

void OSAperiodicTask (void (*p_arg)(void)) {
  void (*function)(void) = p_arg;
    
  while (OS_TRUE) {
		// call the function
    function();
  }
}

/*
 * Convenience methods for creating aperiodic tasks. The function parameter specifies the body of
 * the task, which will be executed for every job.
 */

INT8U  OSTaskCreateAperiodic(void   (*function)(void),
                            OS_STK  *ptos,
                            INT8U    prio) {
  INT8U err;
  
  // create the task with the periodic task wrapper around the job function
  err = OSTaskCreate(OSAperiodicTask, function, ptos, prio); ASSERT_OK(err);
  
  return OS_ERR_NONE;
}