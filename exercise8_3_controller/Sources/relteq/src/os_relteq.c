#include "os_relteq.h"
           
#if OS_RELTEQ_EN > 0

  #include "os_relteq_port.c"
  #include "os_relteq_core.c"

#if OS_RELTEQ_PERIODIC_TASK_EN > 0
  #include "os_relteq_periodic_task.c"
#endif

#if OS_RELTEQ_SPORADIC_TASK_EN > 0
  #include "os_relteq_sporadic_task.c"
#endif

#if OS_RELTEQ_HSF_EN > 0

  #include "os_relteq_fixed_priority_server.c"
  #include "os_relteq_hsf.c"

#if OS_RELTEQ_POLLING_SERVER_EN > 0
  #include "os_relteq_polling_server.c"
#endif

#if OS_RELTEQ_PERIODIC_SERVER_EN > 0
  #include "os_relteq_periodic_server.c"
#endif

#if OS_RELTEQ_DEFERRABLE_SERVER_EN > 0
  #include "os_relteq_deferrable_server.c"
#endif

#if OS_RELTEQ_CBS_SERVER_EN > 0
  #include "os_relteq_cbs_server.c"
#endif

#endif

/*
 * Global variables
 */

RelteqEvent*  RelteqEventFreeList;
RelteqEvent   RelteqEvents[OS_RELTEQ_MAX_EVENTS];

RelteqQueue   RelteqQueues[OS_RELTEQ_MAX_QUEUES];
RelteqQueue*  RelteqQueueFreeList;

RelteqQueue*  RelteqGlobalQueue;
RelteqQueue*  RelteqActiveQueues;
RelteqQueue*  RelteqInactiveQueues;
RelteqTime    RelteqCurrentTime;
RelteqTime    RelteqEarliestEventTime;
RelteqQueue*  RelteqEarliestQueue;

INT8U         RelteqNeedScheduling;

RelteqEventHandler RelteqEventHandlers[kRelteqEventNum];

#if OS_RELTEQ_STATS_EN > 0
INT16U RelteqStatsNumUsedEvents;
INT16U RelteqStatsMaxUsedEvents;
INT16U RelteqStatsNumCreatedEvents;
INT16U RelteqStatsNumUsedQueues;
INT16U RelteqStatsMaxUsedQueues;
INT16U RelteqStatsNumCreatedQueues;
#endif


/*
 * Initialize RELTEQ. This is where all extensions are hooked into RELTEQ.
 */

void Relteq_Init(void) {

  RelteqCore_Init();
  RelteqPort_Init();

#if OS_RELTEQ_PERIODIC_TASK_EN > 0
  RelteqPeriodicTask_Init();
#endif

#if OS_RELTEQ_HSF_EN > 0
  RelteqServer_Init();
  
  RelteqServerGlobalScheduler = RelteqServerPriorityBasedScheduler;

#if OS_RELTEQ_PERIODIC_SERVER_EN > 0
  RelteqPeriodicServer_Init();
#endif

#if OS_RELTEQ_POLLING_SERVER_EN > 0
  RelteqPollingServer_Init();
#endif

#if OS_RELTEQ_DEFERRABLE_SERVER_EN > 0
  RelteqDeferrableServer_Init();
#endif

#if OS_RELTEQ_CBS_SERVER_EN > 0
  RelteqCBSServer_Init();
#endif

#endif /* OS_RELTEQ_HSF_EN > 0 */
}

#endif
