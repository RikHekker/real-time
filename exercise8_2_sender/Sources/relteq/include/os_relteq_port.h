#ifndef OS_RELTEQ_PORT_H
#define OS_RELTEQ_PORT_H
 
#ifndef  OS_MASTER_FILE
#include <ucos_ii.h>
#endif

#if OS_RELTEQ_EN > 0

/*
 * Translation of relteq terms to those used in the target operating system
 */

typedef unsigned char INT8U;

#define RelteqTask          OS_TCB
#define RelteqTime          INT16U

#define RelteqMaxTime 65535

/*
 * Compiler directives
 */
 
#define inline

/*
 * Return the current time. It is used for debugging.
 */
 
#define RelteqPortTimeGet() OSTimeGet()

/*
 * Enter a ciritical section.
 */

#define RelteqPortEnterCritical() OS_ENTER_CRITICAL()

/*
 * Exit a ciritical section.
 */

#define RelteqPortExitCritical() OS_EXIT_CRITICAL()


/*
 * Return if a task is suspended or not.
 */
 
#define RelteqPortTaskIsSuspended(task) ((task->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY)

/*
 * Returns a pointer to the period event for a task.
 */

#define RelteqPortTaskPeriodEvent(task) ((task)->OSTCBPeriodRelteqEvent)

/*
 * Sets the pointer to the period event for a task.
 */

#define RelteqPortTaskSetPeriodEvent(task, event) ((task)->OSTCBPeriodRelteqEvent = (event))

/*
 * Returns a pointer to the task in the ready queue (wrapped inside a relteq event)
 */

#define RelteqPortTaskReadyQueueEvent(task) ((task)->OSTCBReadyQueueEvent)

/*
 * Sets the pointer to the task in the ready queue (wrapped inside a relteq event)
 */

#define RelteqPortTaskSetReadyQueueEvent(task, event) ((task)->OSTCBReadyQueueEvent = (event))

/*
 * Returns the task period.
 */

#define RelteqPortTaskPeriod(task) ((task)->OSTCBPeriod)

/*
 * Sets the task period.
 */

#define RelteqPortTaskSetPeriod(task, period) ((task)->OSTCBPeriod = period)

/*
 * Returns the task offset.
 */

#define RelteqPortTaskOffset(task) ((task)->OSTCBOffset)

/*
 * Sets the task offset.
 */

#define RelteqPortTaskSetOffset(task, offset) ((task)->OSTCBOffset = offset)

/*
 * Sets the pointer to the priority event for a task.
 */

#define RelteqPortTaskPriority(task) ((task)->OSTCBPrio)

/* 
 * indicate to the task that the period has arrived
 */

void RelteqPortTaskPeriodArrived(RelteqTask* task);

/*
 * Return the task name.
 */

#define RelteqPortTaskName(task) ((task)->OSTCBTaskName)

/*
 * Return the parent server of a task.
 */

#define RelteqPortTaskServer(task) ((task)->OSTCBServer)

/*
 * Sets the parent server of a task.
 */

#define RelteqPortTaskSetServer(task, server) ((task)->OSTCBServer = server)

/*
 * Idle task.
 */
 
#define RelteqPortTaskIdle (OSTCBPrioTbl[OS_TASK_IDLE_PRIO]) 

/*
 * Make a task a sporadic task.
 */

#define RelteqPortTaskMakeSporadic(task) ((task)->OSTCBSporadicFlag |= RelteqSporadicFlagIsSporadic)

/*
 * Return whether a task is sporadic or not.
 */

#define RelteqPortTaskIsSporadic(task) ((task)->OSTCBSporadicFlag & RelteqSporadicFlagIsSporadic)

/*
 * Indicate that the event of a sporadic task has arrived.
 */

#define RelteqPortSporadicTaskSetEventArrived(task) ((task)->OSTCBSporadicFlag |= RelteqSporadicFlagEventArrived)

/*
 * Indicate that the minimum interarrival period of a sporadic task has expired.
 */

#define RelteqPortSporadicTaskSetPeriodExpired(task) ((task)->OSTCBSporadicFlag |= RelteqSporadicFlagPeriodArrived)
 
/*
 * Return whether the minimum interarrival period for a sporadic event has expired.
 */
 
#define RelteqPortSporadicTaskEventArrived(task) ((task)->OSTCBSporadicFlag & RelteqSporadicFlagEventArrived)

/*
 * Return whether the minimum interarrival period for a sporadic event has expired.
 */
 
#define RelteqPortSporadicTaskPeriodExpired(task) ((task)->OSTCBSporadicFlag & RelteqSporadicFlagPeriodArrived)

/*
 * Clear the event and period flags for a sporadic task.
 */
 
#define RelteqPortSporadicTaskClear(task) \
        task->OSTCBSporadicFlag &= ~RelteqSporadicFlagEventArrived; \
        task->OSTCBSporadicFlag &= ~RelteqSporadicFlagPeriodArrived;

/*
 * Print a string to stdout.
 */

void RelteqPortPrint(char* s, ...);

/*
 * Print a string to a string.
 */

void RelteqPortStringPrint(char* a, ...);

/*
 * Handler for the delay event.
 */

void RelteqPortHandleDelay(RelteqTask* task);

/*
 * Copy one string into another.
 */
 
void RelteqPortStringCopy(char* target, char* source, INT8U* perr);

/*
 * Utilities
 */

#if OS_ARG_CHK_EN > 0

#ifndef ASSERT

#if OS_LOG_EN > 1
#define ASSERT( b , ...) \
  if ( !(b) ) { \
    RelteqPortPrint( "ASSERT @ %s # %d, ", __FILE__, __LINE__); \
    RelteqPortPrint(__VA_ARGS__); \
    RelteqPortPrint("\n"); \
    logPrint("error_trace.log");\
    exit(0);\
  };

#else /* OS_LOG_EN == 0 */
#define ASSERT( b , ...) \
  if ( !(b) ) { \
    RelteqPortPrint( "ASSERT @ %s # %d, ", __FILE__, __LINE__); \
    RelteqPortPrint(__VA_ARGS__); \
    RelteqPortPrint("\n"); \
    exit(0);\
  };

#endif

#endif /* ASSERT */

#define ASSERT_OK(err) ASSERT(err == OS_ERR_NONE, "err=%s", RelteqErrToString(err));

#define WARNING( b , ...) \
  if ( !(b) ) { \
    RelteqPortPrint( "WARNING @ %s # %d, ", __FILE__, __LINE__); \
    RelteqPortPrint(__VA_ARGS__); \
    RelteqPortPrint("\n"); \
  };

#else /* OS_ARG_CHK_EN == 0 */
void ASSERT(int a, ...); 
void ASSERT_OK(int a, ...);
void WARNING(char* a, ...); 
#endif /* OS_ARG_CHK_EN */


#endif /* OS_RELTEQ_EN */

#endif /* OS_RELTEQ_PORT_H */
