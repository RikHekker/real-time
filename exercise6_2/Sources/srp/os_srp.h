/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                  MUTUAL EXCLUSION SRP SEMAPHORE MANAGEMENT
*
*
* File    : OS_SRP.H
* By      : Martijn van den Heuvel
*					:	SAN group, Dep. of Mathematics and Computer Science, Eindhoven University of Technology
* Version : V2.86
*
*********************************************************************************************************
*/
#ifndef OS_SRP_H
#define OS_SRP_H

#if OS_MUTEX_SRP_EN > 0
#if OS_MUTEX_SIRAP_EN > 0 || OS_MUTEX_HSTP_EN > 0
#define OS_MUTEX_PROTOCOL_SIRAP   0
#define OS_MUTEX_PROTOCOL_OVERRUN 1
#define OS_MUTEX_PROTOCOL_BROE    2

struct resource;            /* Forward declaration */

typedef struct {
  struct resource* globalResource;  /* The resource for which the subsystem info is stored                             */
  INT8U localCeiling;               /* subsystem local resource ceiling                                                */
  volatile INT8U localLockingTask;  /* Task that currently holds the resource                                          */
  void* previous;                   /* pointer to the previous localResourceInfo block on the stack                    */
} localResourceInfo;                /* Resource data that is uniquely defined for each server that shares the resource */
#endif

typedef struct resource{
#if OS_TASK_NAME_SIZE > 1
    INT8U    name[OS_TASK_NAME_SIZE];       /* Name of the resource (optionally)                        */
#endif
    INT8U    busy;                          /* HSTP: >0 if busy; otherwise 0                                  */
    INT8U    ceiling;                       /* (Global) Resource ceiling                                */
    INT8U    lockingTask;                   /* Task that currently holds the resource                   */
    void*    previous;                      /* Pointer to the previous resource on the stack            */
#if OS_MUTEX_SIRAP_EN > 0 || OS_MUTEX_HSTP_EN > 0
    /* For each server we keep a pointer to the local data for this resource. 
     * If the resource is not shared by the server, the value is 0                                      */
    localResourceInfo* localInfo[OS_MAX_SERVERS]; 
#endif
  } Resource;

extern Resource  OSSRPSystemCeiling[MAX_RESOURCES];                       /* SRP system ceiling                              */
extern INT8U     OSSRPResourceCounter;
extern Resource* OSSRPTopOfStack;
extern Resource* OSSRPEmpty;

void          OS_MutexSRPInit         (void);

Resource      *OSMutexSRPCreate       (INT8U           prio, 
                                       INT8U           *perr);

Resource      *OSMutexSRPDel          (Resource        *pevent, 
                                       INT8U           *perr);

void          OSMutexSRPPend          (Resource        *pevent, 
                                       INT8U           *perr);

INT8U         OSMutexSRPPost          (Resource        *pevent);

#if OS_MUTEX_EN < 1
/* if the original mutex implementation of uC/OS-II is disabled, we allow to use the original uC/OS-II 
 * function calls  to be redirected to the SRP interface:
 */
#define OSMutexCreate(prio, perr) OSMutexSRPCreate(prio, perr)
#define OSMutexPend(pevent, timeout, perr) OSMutexSRPPend(pevent, perr)
#define OSMutexPost(pevent) OSMutexSRPPost(pevent)
#define OSMutexDel(prio, opt, perr) OSMutexSRPDel(prio, perr)
#endif

#if OS_MUTEX_SIRAP_EN > 0 || OS_MUTEX_HSTP_EN > 0
void          OS_MutexSIRAPInit       (void);
Resource      *OSMutexSIRAPCreate     (INT8U           prio, 
                                       INT8U           *perr);

void          OSSetSubsystemCeiling   (Resource        *pevent,
                                       INT8U           subsystemID,
                                       INT8U           subCeiling,
                                       INT8U           *perr);

void          OSSetResourceHoldingTime(INT8U           subsystemID, 
                                       INT32U          overrun, 
                                       INT8U           *perr);

void          OSSetHSFMutexProtocol   (INT8U          subsystemID,
                                       INT8U          protocol);
#if OS_MUTEX_OVERRUN_EN > 0
#define OSSetSubsystemOverrun(subsystemID, overrun, perr) do {OSSetResourceHoldingTime(subsystemID, overrun, perr); \
                                                          OSSetHSFMutexProtocol(subsystemID, OS_MUTEX_PROTOCOL_OVERRUN); } while (0)
#endif

Resource      *OSMutexSIRAPDel        (Resource        *pevent, 
                                       INT8U           *perr);

void          OSMutexHSFPend          (Resource        *pevent, 
                                       INT8U           *perr);

INT8U         OSMutexHSFPost          (Resource        *pevent);

/* Todo: remove these macros. It remains here for legacy reasons */
#define OSMutexSIRAPPend(pevent, holdTime, perr) do { OSSCBCur->OSServerOverrunBudget = holdTime; OSMutexHSFPend(pevent, perr); } while (0)
#define OSMutexSIRAPPost(pevent) OSMutexHSFPost(pevent)

#endif
#endif /* OS_MUTEX_SRP_EN */

#endif

