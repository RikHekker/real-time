#ifndef OS_RELTEQ_HSF_H
#define OS_RELTEQ_HSF_H

#if OS_RELTEQ_HSF_EN > 0

#if OS_RELTEQ_EN == 0
  #error "OS_RELTEQ_HSF_EN requires OS_RELTEQ_EN to be enabled"
#endif

#if OS_RELTEQ_MAX_SERVERS < 1
  #error "OS_RELTEQ_HSF_EN requires OS_RELTEQ_MAX_SERVERS to be at least 1"
#endif


/*
 * Enumeration type for selecting different schedulers (local and global).
 */

#define OS_RELTEQ_FPS 0
#define OS_RELTEQ_EDF 1

/*
 * Server types. Add your server type here, so that later you can bind context switch handlers to it.
 */
 
typedef enum {
  kRelteqServerNone = 0,
#if OS_RELTEQ_PERIODIC_SERVER_EN > 0
  kRelteqServerPeriodic,
#endif
#if OS_RELTEQ_POLLING_SERVER_EN > 0
  kRelteqServerPolling,
#endif
#if OS_RELTEQ_DEFERRABLE_SERVER_EN > 0
  kRelteqServerDeferrable,
#endif
#if OS_RELTEQ_CBS_SERVER_EN > 0
  kRelteqServerCBS,
#endif
  kRelteqServerTypeNum
} RelteqServerType;

typedef enum {
  kRelteqServerRunning = 0,
  kRelteqServerDepleted,
  kRelteqServerReady,
  kRelteqServerWaiting
} RelteqServerState;

typedef struct RelteqServer {
  RelteqServerType      ServerType; /*  TODO: see if replacing the type with INT8U */
  RelteqServerState     ServerState; /*  will reduce the memory footprint */
  INT32U                ServerBudgetLeft;    /* For monitoring. */
  INT32U                ServerBudget;
  INT32U                ServerPeriod;
  INT8U                 ServerPriority;
#if OS_RELTEQ_NAME_SIZE > 1
  char                 ServerName[OS_RELTEQ_NAME_SIZE];
#endif
  RelteqQueue*          ServerReadyQueue; /* stores the tasks. use event time for task priority */
  RelteqQueue*          ServerOverrunTime;
  RelteqQueue*          ServerRelativeQueue; /* Server event queues. */
  RelteqQueue*          ServerAbsoluteQueue;
  RelteqEvent*          ServerDepletionEvent; /* Allow to remove depletion event, in case server
                                                  replenished before depletion event has expired */
  RelteqEvent*          ServerStopwatchEvent;
  RelteqEvent*          ServerWakeupEvent;
  struct RelteqServer*  ServerNext;
#if OS_RELTEQ_LOCAL_EDF_EN == 0
  RelteqTask*           ServerHighestReadyTask;
#endif
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  INT32U                ServerDeadline;
  RelteqEvent*          ServerGlobalReadyQueueEvent;
#endif
} RelteqServer;

typedef void (*RelteqServerInitializer)(RelteqServer* server, INT8U* pErr);
typedef void (*RelteqServerSwitchHandler)(RelteqServer* server);
typedef void (*RelteqServerTaskArrivedHandler)(RelteqServer* server, RelteqTask* task);
typedef void (*RelteqServerWakeupHandler)(RelteqServer* server);

typedef RelteqServer* (*RelteqServerScheduler)(void);

/*
 * Global variables
 */

extern RelteqServer    RelteqServers[OS_RELTEQ_MAX_SERVERS];
extern RelteqServer*   RelteqServerFreeList;
extern RelteqServer*   RelteqCurrentServer;
extern RelteqQueue*    RelteqStopwatch;
extern RelteqQueue*    RelteqGlobalReadyQueue;

extern RelteqServerInitializer RelteqServerInitializers[kRelteqServerTypeNum];
extern RelteqServerSwitchHandler RelteqServerSwitchInHandlers[kRelteqServerTypeNum];
extern RelteqServerSwitchHandler RelteqServerSwitchOutHandlers[kRelteqServerTypeNum];
extern RelteqServerTaskArrivedHandler RelteqServerTaskArrivedHandlers[kRelteqServerTypeNum];
extern RelteqServerWakeupHandler RelteqServerWakeupHandlers[kRelteqServerTypeNum];

extern RelteqServerScheduler RelteqServerGlobalScheduler;

/*
 * Public methods
 */

void            RelteqServer_Init(void);
RelteqTask*     RelteqServerSchedule(void);

RelteqServer*   RelteqServerCreate(INT8U priority, INT16U budget, INT16U period, 
                                   INT8U type, INT16U deadline, INT8U* pErr);
void            RelteqServerDel(RelteqServer* server);

/*
 * Assign a task to a server.
 * IMPORTANT: make sure to call this methods BEFORE making the task periodic!
 */
 
void            RelteqServerAddTask(RelteqServer* server, RelteqTask* task, INT8U* pErr);

/*
 * Protected methods, intended ONLY for RELTEQ extension developers.
 *
 * DO NOT CALL THESE DIRECTLY FROM APPLICATION !!!
 */
 
void            RelteqServerInsertIntoStopwatch(RelteqServer* server);
void            RelteqServerMakeTaskReady(RelteqTask* task);
void            RelteqServerMakeTaskNotReady(RelteqTask* task);
void            RelteqQueueIncrement(RelteqQueue* queue, INT8U* pErr);

#define         RelteqServerRegisterInitializer(server, handler) \
                    RelteqServerInitializers[server] = handler;
#define         RelteqServerRegisterSwitchInHandler(server, handler) \
                    RelteqServerSwitchInHandlers[server] = handler;
#define         RelteqServerRegisterSwitchOutHandler(server, handler) \
                    RelteqServerSwitchOutHandlers[server] = handler;
#define         RelteqServerRegisterTaskArrivedHandler(server, handler) \
                    RelteqServerTaskArrivedHandlers[server] = handler;
#define         RelteqServerRegisterWakeupHandler(server, handler) \
                    RelteqServerWakeupHandlers[server] = handler;

#if OS_RELTEQ_NAME_SIZE > 1
void            RelteqServerNameSet(RelteqServer* server, char* name, INT8U *perr);
#endif

#if OS_RELTEQ_DEBUG_EN > 0
void            RelteqDumpServer(RelteqServer* server);
#endif

#endif /* OS_RELTEQ_HSF_EN */

#endif /* OS_RELTEQ_HSF_H */
