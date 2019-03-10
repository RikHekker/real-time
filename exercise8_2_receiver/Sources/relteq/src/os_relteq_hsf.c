/*
 * Global variables
 */

RelteqServer    RelteqServers[OS_RELTEQ_MAX_SERVERS];
RelteqServer*   RelteqServerFreeList;
RelteqServer*   RelteqCurrentServer;
RelteqQueue*    RelteqStopwatch;
RelteqQueue*    RelteqGlobalReadyQueue;

RelteqServerInitializer RelteqServerInitializers[kRelteqServerTypeNum];
RelteqServerSwitchHandler RelteqServerSwitchInHandlers[kRelteqServerTypeNum];
RelteqServerSwitchHandler RelteqServerSwitchOutHandlers[kRelteqServerTypeNum];
RelteqServerTaskArrivedHandler RelteqServerTaskArrivedHandlers[kRelteqServerTypeNum];
RelteqServerWakeupHandler RelteqServerWakeupHandlers[kRelteqServerTypeNum];

RelteqServerScheduler RelteqServerGlobalScheduler;

/*
 * Method definitions
 */

RelteqTask* RelteqServerFindHighestReadyTask(RelteqServer* server) {
  RelteqEvent* event;

  for (event = server->ServerReadyQueue->QueueHead; event != NULL && !RelteqPortTaskIsReady(((RelteqTask*)event->EventPtr)); event = event->EventNext) {}
  
  if (event) {
    return (RelteqTask*)event->EventPtr;
  }
  
#if OS_RELTEQ_PERIODIC_SERVER_EN > 0
  if (server->ServerType == kRelteqServerPeriodic) {
    return RelteqPortTaskIdle;
  }
#endif

  return NULL;
}

#if OS_RELTEQ_LOCAL_EDF_EN > 0

#if OS_RELTEQ_PERIODIC_SERVER_EN > 0
  #define RelteqServerIsReady(server) (server->ServerType == kRelteqServerPeriodic ? 1 : server->ServerReadyQueue->QueueHead != NULL)
#else
  #define RelteqServerIsReady(server) (server->ServerReadyQueue->QueueHead != NULL)
#endif
  
  #define RelteqServerHighestReadyTask(server) RelteqServerFindHighestReadyTask(server)

#else
  #define RelteqServerIsReady(server) (server->ServerHighestReadyTask != NULL)
  #define RelteqServerHighestReadyTask(server) (server->ServerHighestReadyTask)
#endif /* OS_RELTEQ_LOCAL_EDF_EN > 0 */

/*
 * Wakeup event handler.
 */

#if OS_RELTEQ_SERVER_WAKEUP_EVENT_EN > 0 
void RelteqServerHandleWakeup(RelteqQueue* queue, RelteqEvent* event) {
  INT8U err;
  RelteqServer* server = (RelteqServer*)event->EventPtr;
  RelteqQueue* absoluteQueue = server->ServerAbsoluteQueue;

  RelteqEventDel(event);
  server->ServerWakeupEvent = NULL;

#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent wakeup \"%s (%s)\"", server->ServerName, queue->QueueName);
#else
  logCustomEvent("relteqEvent wakeup \"Server%d (%s)\"", (void*)(long)server->ServerPriority, queue->QueueName);
#endif
#endif      

  if (absoluteQueue->QueueHead->EventKind == kRelteqEventDummy) {
    // if the head event in the aboslute queue is a dummy event, set up a new wakeup event
    RelteqQueuePop(absoluteQueue, &err); ASSERT_OK(err);
    if (absoluteQueue->QueueHead) {
      RelteqTime time = absoluteQueue->QueueHead->EventTime;
      server->ServerWakeupEvent = RelteqEventCreate(kRelteqEventWakeup, server);
      RelteqQueueInsertEvent(RelteqGlobalQueue, server->ServerWakeupEvent, time, &err); ASSERT_OK(err);
    }
  } else {
    // clear the reference to the wakeup event. Do it before handling the event, in case the
    // handler inserts a new wakeup event. Note that this should actually not occur.
    server->ServerWakeupEvent = NULL;
    
    // the new server state depends on the current state
    switch (server->ServerState) {
      case kRelteqServerReady:
      case kRelteqServerRunning: 
      case kRelteqServerDepleted:
        // keep the current state
        break;
      case kRelteqServerWaiting:
        server->ServerState = kRelteqServerReady;
    }
    
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  // make sure that the server is inside the ready queue, otherwise it will not be noticed by
  // the scheduler. Hence, insert it into the global ready queue, in case its deadline expired.
  if (!server->ServerGlobalReadyQueueEvent) {
    RelteqEvent* event = RelteqEventCreate(kRelteqEventServerDeadline, server);
    // NOTE: we picked deadline 0, to faciliate the CBS implementation. CBS postpones the deadline
    // of the server when a task arrives on an idle server. It can happen that the server deadline 
    // expiration is handled before the arrival of a task.
    RelteqQueueInsertEvent(RelteqGlobalReadyQueue, event, 0, &err); ASSERT_OK(err);
    server->ServerGlobalReadyQueueEvent = event;
  }
#endif
    
#if OS_RELTEQ_SERVER_WAKEUP_EVENT_DEFERRED_EN > 0
    // do not handle any events here, instead indicate that the sever is ready and postpone
    // handling of the events until the server is switched in
#else
    // handle the head event in the aboslute queue (and all following expired events)
    // IMPORTANT: we do not sync the event time and rely on proper timing of the wakeup event!!!
    RelteqEvent* headEvent = absoluteQueue->QueueHead;
    headEvent->EventTime = 0;
    
    for (; headEvent && headEvent->EventTime == 0; headEvent = absoluteQueue->QueueHead) {
      RelteqQueuePop(absoluteQueue, &err); ASSERT_OK(err);
      RelteqEventHandle(absoluteQueue, headEvent, &err); ASSERT_OK(err);
    }
    
    // we did here a RelteqQueueSyncWithQueueUntilEvent() "light", since we know that the first
    // event in the absolute queue has just expired (assuming the IMPORTANT part). Therefore we
    // need to reinsert the stopwatch event, to make sure that the following SwitchIn will
    // properly sync with stopwatch.
    RelteqServerInsertIntoStopwatch(server);
    
    // insert the next wakeup event into the global system queue. Note that the wakeup event will
    // be preserved until it is reinserted again into the system queue, i.e. it is not deleted.
    if (absoluteQueue->QueueHead) {
      RelteqTime time = absoluteQueue->QueueHead->EventTime;
      server->ServerWakeupEvent = RelteqEventCreate(kRelteqEventWakeup, 0, server);
      RelteqQueueInsertEvent(RelteqGlobalQueue, server->ServerWakeupEvent, time, &err); ASSERT_OK(err);
    }
#endif /* OS_RELTEQ_SERVER_WAKEUP_EVENT_DEFERRED_EN */

    // perfrom a custom handler (if specified)
    if (RelteqServerWakeupHandlers[server->ServerType]) {
      RelteqServerWakeupHandlers[server->ServerType](server);
    }

  } /* event is a dummy */
}
#endif /* OS_RELTEQ_SERVER_WAKEUP_EVENT_EN */

/*
 * RelteqServer_Init
 */

void RelteqServer_Init() {
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  INT8U err;
#endif
  RelteqServer* server;
  int i;
  
  // initialize servers
#if OS_RELTEQ_MAX_SERVERS > 0
  // link all queues in the global queues table
  for (i = 1; i < OS_RELTEQ_MAX_SERVERS; i++) {
    server = &RelteqServers[i-1];
    RelteqServerDel(server);
    server->ServerNext = &RelteqServers[i];
  }

  // initialise the last queue, which was skipped in the loop
  server = &RelteqServers[i-1];
  RelteqServerDel(server);
  server->ServerNext = NULL;

  // point the free list to the first queue in the table
  RelteqServerFreeList = &RelteqServers[0];
#endif

  RelteqCurrentServer = NULL;
  RelteqStopwatch = RelteqQueueCreate();
#if OS_RELTEQ_NAME_SIZE > 1
  RelteqPortStringPrint(RelteqStopwatch->QueueName, "%s", "Stopwatch");  
#endif

  RelteqGlobalReadyQueue = RelteqQueueCreate();
#if OS_RELTEQ_NAME_SIZE > 1
  RelteqPortStringPrint(RelteqGlobalReadyQueue->QueueName, "%s", "GlobalReady");  
#endif
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  // in case of global EDF, treat the ready queue like any other relteq queue.
  RelteqQueueActivate(RelteqGlobalReadyQueue, &err); ASSERT_OK(err);
#endif

  // register event handlers
  RelteqRegisterEventHandler(kRelteqEventReplenishment, RelteqServerHandleReplenishment);
  RelteqRegisterEventHandler(kRelteqEventDepletion, RelteqServerHandleDepletion);
  RelteqRegisterEventHandler(kRelteqEventServerDeadline, RelteqServerHandleServerDeadline);
  RelteqRegisterEventHandler(kRelteqEventStopwatch, NULL);
#if OS_RELTEQ_SERVER_WAKEUP_EVENT_EN > 0 
  RelteqRegisterEventHandler(kRelteqEventWakeup, RelteqServerHandleWakeup);
#endif
}

RelteqServer* RelteqServerCreate(INT8U priority, INT16U budget, INT16U period, INT8U type, INT16U deadline, INT8U* pErr) {
#if OS_CRITICAL_METHOD == 3     /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif
  INT8U err;
  RelteqEvent* event;
  RelteqServer* server;

  (void)deadline; /* avoid compiler warning about unused variable */
  
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return NULL;
  }
  if (budget == 0) {
    *pErr = OS_ERR_RELTEQ_BUDGET_ZERO;
    return NULL;
  }
#endif

  *pErr = OS_ERR_NONE;
  RelteqPortEnterCritical();

  if (RelteqServerFreeList == NULL) {    
    RelteqPortExitCritical();
    *pErr = OS_ERR_RELTEQ_SERVER_NULL;
    return NULL;
  }
  
  // create the server control block
  server = RelteqServerFreeList;
  server->ServerType = type;
  server->ServerBudgetLeft = budget;
  server->ServerBudget = budget;
  server->ServerPeriod = period;
  server->ServerPriority = priority;
  
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  ASSERT(deadline >= budget, "server with priority %d has a deadline smaller than its capacity", priority);
  server->ServerDeadline = deadline;
#endif

  // the initial state of a server does not depend on its kind. Since a periodic idling task always 
  // has an idle task ready, it is initially in the ready state. A deferrable server could be 
  // created in the waiting state, but we let it be ready and have the first iteration check its
  // ready queue (otherwise doing the first iteration is complicated). This is becuase the periodic
  // tasks need to execute short initialization code before they block on the first PeriodWait().
  server->ServerState = kRelteqServerReady;
  
#if OS_RELTEQ_NAME_SIZE > 1
    RelteqPortStringCopy(server->ServerName, "?", &err); ASSERT_OK(err);
#endif

  // remove the server control block frrom the free list
  RelteqServerFreeList = RelteqServerFreeList->ServerNext;
  server->ServerNext = NULL;
  
  // create the server queues
  server->ServerReadyQueue = RelteqQueueCreate();
  server->ServerOverrunTime = RelteqQueueCreate();  
  server->ServerRelativeQueue = RelteqQueueCreate();  
  server->ServerAbsoluteQueue = RelteqQueueCreate();  
  
  if (server->ServerReadyQueue == NULL
  || server->ServerOverrunTime == NULL 
  || server->ServerRelativeQueue == NULL
  || server->ServerAbsoluteQueue == NULL) {
    RelteqPortExitCritical();
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return NULL;
  }

  // activate the queue, in case the server may be woken up by its own events, even without
  // being switched in (like bandwidth preserving servers)
  //RelteqQueueActivate(server->ServerAbsoluteQueue, &err); ASSERT_OK(err);
  
  // insert an event at the head of the stopwatch queue, to make sure that when the server is 
  // switched in for the first time the stopwatch is synched appropriately
  event = RelteqEventCreate(kRelteqEventStopwatch, server);
  if (event == NULL) {
    RelteqPortExitCritical();
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return NULL;
  }
  RelteqQueueInsertEvent(RelteqStopwatch, event, 0, &err); ASSERT_OK(err);
  server->ServerStopwatchEvent = event;

  // create an event for the server inside the global ready queue. note that we use relteq queue
  // here, since we need to sort the queue on priority, for which we reuse the EventTime field.
  event = RelteqEventCreate(kRelteqEventServerDeadline, server);
  if (event == NULL) {
    RelteqPortExitCritical();
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return NULL;
  }
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  // note that any server (also CBS) will be created with a dedline = D. This is to make the server
  // ready allowing it to process the initialization part of the periodic tasks before they reach
  // their first PeriodWait().
  RelteqQueueInsertEvent(RelteqGlobalReadyQueue, event, server->ServerDeadline, &err); ASSERT_OK(err);
  server->ServerGlobalReadyQueueEvent = event;
#else
  RelteqQueueInsertEvent(RelteqGlobalReadyQueue, event, server->ServerPriority, &err); ASSERT_OK(err);
#endif

  // do server specific initialization
  RelteqServerInitializers[type](server, &err);
  
  if (err != OS_ERR_NONE) {
    RelteqPortExitCritical();
    *pErr = err;
    return NULL;
  }

#if OS_LOG_EN > 0
  logServerCreated(server);
#endif
  
  RelteqPortExitCritical();
  return server;
}

void RelteqServerDel(RelteqServer* server) {
#if OS_CRITICAL_METHOD == 3     /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif

  RelteqPortEnterCritical();
  RelteqQueueDel(server->ServerReadyQueue);
  RelteqQueueDel(server->ServerOverrunTime);
  RelteqQueueDel(server->ServerRelativeQueue);
  RelteqQueueDel(server->ServerAbsoluteQueue);
  
  if (server->ServerDepletionEvent) RelteqEventDel(server->ServerDepletionEvent);
  if (server->ServerStopwatchEvent) RelteqEventDel(server->ServerStopwatchEvent);
  if (server->ServerWakeupEvent) RelteqEventDel(server->ServerWakeupEvent);
  //ASSERT(0, "There is a memory leak here. Relteq events created by the server (e.g. replenishment) are not deleted.");
  
  server->ServerType = kRelteqServerNone;
  server->ServerState = kRelteqServerDepleted;
  server->ServerBudgetLeft = 0;
  server->ServerBudget = 0;
  server->ServerPeriod = 0;
  server->ServerPriority = 0;
#if OS_RELTEQ_NAME_SIZE > 1
  server->ServerName[0] = '\0';
#endif
  server->ServerReadyQueue = NULL;
  server->ServerOverrunTime = NULL;  
  server->ServerRelativeQueue = NULL;  
  server->ServerAbsoluteQueue = NULL;  
  server->ServerDepletionEvent = NULL;
  server->ServerStopwatchEvent = NULL;
  server->ServerWakeupEvent = NULL;
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  server->ServerDeadline = 0;
  server->ServerGlobalReadyQueueEvent = NULL;
#endif

#if OS_LOG_EN > 0
  logServerDeleted(server);
#endif

  server->ServerNext = RelteqServerFreeList;
  RelteqServerFreeList = server;
  RelteqPortExitCritical();
}

void RelteqServerAddTask(RelteqServer* server, RelteqTask* task, INT8U* pErr) {
#if OS_CRITICAL_METHOD == 3     /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif
#if OS_RELTEQ_PERIODIC_TASK_EN > 0 || OS_RELTEQ_LOCAL_EDF_EN == 0
  INT8U err;
#endif
#if OS_RELTEQ_PERIODIC_TASK_EN > 0
  RelteqEvent* event;
#endif

  *pErr = OS_ERR_NONE;

  RelteqPortEnterCritical();    

#if OS_RELTEQ_PERIODIC_TASK_EN > 0
  // in case the task is periodic and its period has been already set, we need to remove
  // its period event from the global queue, and reinsert it into the servers queue
  event = RelteqPortTaskPeriodEvent(task);
  if (event) {
    ASSERT(RelteqPortTaskServer(task) == NULL, "task can be added to a server only once");
    ASSERT(RelteqPortTimeGet() == 0, "task can be added to a server only at time 0, while it is time %d", RelteqPortTimeGet());
    RelteqQueueDeleteEvent(RelteqGlobalQueue, event, &err); ASSERT_OK(err);

    // point the task to the server
    RelteqPortTaskSetServer(task, server);

    // reinsert the period event into the servers queue
    RelteqPortTaskSetPeriodEvent(task, NULL);
    RelteqPeriodicTaskSetPeriod(task, RelteqPortTaskPeriod(task), RelteqPortTaskOffset(task));
  } else {
    // point the task to the server
    RelteqPortTaskSetServer(task, server);
  }
#else
  // point the task to the server
  RelteqPortTaskSetServer(task, server);
#endif
  
#if OS_RELTEQ_LOCAL_EDF_EN == 0
  RelteqPortTaskSetReadyQueueEvent(task, RelteqEventCreate(kRelteqEventDeadline, task));
  RelteqQueueInsertEvent(server->ServerReadyQueue, RelteqPortTaskReadyQueueEvent(task), RelteqPortTaskPriority(task), &err); ASSERT_OK(err);
#endif

  // check if task is ready
  if (RelteqPortTaskIsReady(task)) {
    RelteqServerMakeTaskReady(task);
  }
  
  RelteqPortExitCritical();
}

void RelteqServerInsertIntoStopwatch(RelteqServer* server) {
  INT8U err;

  // delete the server event
  if (server->ServerStopwatchEvent) {
    RelteqQueueDeleteEvent(RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);
  }

  // create a new stopwatch event at the head of the stopwatch
  server->ServerStopwatchEvent = RelteqEventCreate(kRelteqEventStopwatch, server);
  RelteqQueueLinkEvent(RelteqStopwatch, NULL, server->ServerStopwatchEvent, RelteqStopwatch->QueueHead, &err); ASSERT_OK(err);
  //RelteqQueueInsertEvent(RelteqStopwatch, server->ServerStopwatchEvent, 0, &err); ASSERT_OK(err);
}

INT8U RelteqServerSwitchIn(RelteqServer* server) {  
  INT8U err;
  //logMessage("switching in %s", (char*)server->ServerName, 0);
  
#if OS_RELTEQ_NAME_SIZE > 0
  ASSERT(server->ServerType != kRelteqServerNone, "switching in server '%s' of type none", server->ServerName);
#else
  ASSERT(server->ServerType != kRelteqServerNone, "switching in server of type none");
#endif

  // update server's state
  server->ServerState = kRelteqServerRunning;
  
  // call the appropriate switch handler
  ASSERT(server->ServerType < kRelteqServerTypeNum && RelteqServerSwitchInHandlers[server->ServerType],
    "unknown server type %d", server->ServerType);
  RelteqServerSwitchInHandlers[server->ServerType](server);
  
  // process head events with time 0 in the realtive queue, in case an event was inserted while 
  // the server was inactive (e.g. a period event for a sporadic task)
  RelteqQueueSyncWithTime(server->ServerRelativeQueue, 0, &err); ASSERT_OK(err);
    
  // return if need to reschedule, e.g. moved to waiting state while processing queues
  if (server->ServerState != kRelteqServerRunning) {
    // since the absolute queue was synchronizeed, we need to reinsert the server's stopawtch event at the head
    RelteqServerInsertIntoStopwatch(server);
    return 1;
  }
  
  // remove the stopwatch event from the stopwatch ueue
  if (server->ServerStopwatchEvent) {
    RelteqQueueDeleteEvent(RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);
    server->ServerStopwatchEvent = NULL;
  }
  
  // switch in both server queues
  RelteqQueueActivate(server->ServerAbsoluteQueue, &err); ASSERT_OK(err);
  RelteqQueueActivate(server->ServerRelativeQueue, &err); ASSERT_OK(err);
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  // IMPORTANT: the order in which queues are synchronized with current time matters! Deadlines in 
  // the ready queue must be processed before the periodic task arrivals in the absolute queue.
  // Therefore, the ready queue must be sunchronized before absolute queue. Since an activated
  // queue is inserted at the head of the active queues, it must be inserted last.
  RelteqQueueActivate(server->ServerReadyQueue, &err); ASSERT_OK(err);
#endif
    
#if OS_LOG_EN > 0
	logServerResumed(server);
#endif

  return 0;
}

void RelteqServerSwitchOut(RelteqServer* server) {
  //logMessage("switching out %s", (char*)server->ServerName, 0);  

  // insert the server's stopwatch event into the stopwatch queue
  RelteqServerInsertIntoStopwatch(server);

  // call the appropriate switch handler
  ASSERT(server->ServerType < kRelteqServerTypeNum && RelteqServerSwitchInHandlers[server->ServerType], "unknown server type %d", server->ServerType);
  RelteqServerSwitchOutHandlers[server->ServerType](server);
  
  // update server's state
  switch (server->ServerState) {
    case kRelteqServerReady:
      // a server may be switched out in the ready state, when a higher priority server arrives
      break;
    case kRelteqServerRunning:
      // check if any of the server's tasks are ready and update the server ready state accordingly
      if (RelteqServerIsReady(server)) {
        server->ServerState = kRelteqServerReady;
      } else {
        server->ServerState = kRelteqServerWaiting;
      }
      break;
    case kRelteqServerWaiting:
    case kRelteqServerDepleted:
      // n.b. if the server is depleted then it means that it just became depleted.
      break;
  }
}

/*
 * Return the highest priority server ready to run
 */

RelteqServer* RelteqServerPriorityBasedScheduler(void) {
  RelteqEvent* serverEvent;
  RelteqServer* server;
  
  // traverse through all created servers, which are not depleted ...
  // (note that we also need to check the RelteqCurrentServer, since it may have become unready)
  for (serverEvent = RelteqGlobalReadyQueue->QueueHead; serverEvent != NULL; serverEvent = serverEvent->EventNext) {
    server = (RelteqServer*)serverEvent->EventPtr;
    if (server->ServerState == kRelteqServerReady || server->ServerState == kRelteqServerRunning) {
      break;
    }
  }

  if (serverEvent) {
    return server;
  }
  
  return NULL; /* don't return in else to avoid compiler warning that reaching end of non-void function */
}

/*
 * Return the highest priority task ready to run
 */
 
RelteqTask* RelteqServerSchedule(void) {
#if OS_CRITICAL_METHOD == 3     /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr = 0;
#endif
  RelteqServer* highestReadyServer;
  RelteqTask* task;
  
  RelteqPortEnterCritical();
  // call the global scheduler to determin the highest ready server
  highestReadyServer = RelteqServerGlobalScheduler();
  //RelteqPortPrint("%d highest ready server %s\n", RelteqPortTimeGet(), highestReadyServer ? highestReadyServer->ServerName : "(null)");
  
  // check if any context switch will occur (i.e. in or out)
  // note that RelteqCurrentServer was included in the computation of highestReadyServer
  if (RelteqCurrentServer != highestReadyServer) { 
    // make sure to sync all currently active queues, before switching queues in or out, so that
    // they are up to date (note that the tick handler syncs queus only after "earliest time" expires)
    RelteqSyncWithCurrentTime();

    // check if need to switch out the current server
    if (RelteqCurrentServer != NULL) {
      RelteqServerSwitchOut(RelteqCurrentServer);
    }
  
    // check if need to switch in the highestReadyServer
    if (highestReadyServer != NULL) {
      if (RelteqServerSwitchIn(highestReadyServer)) {
        RelteqCurrentServer = NULL;

        // adjust the earliest event time, needed for multiplexing active queues
        RelteqUpdateEarliestTime();

        RelteqPortExitCritical();
        return RelteqServerSchedule();
      }
    }

    RelteqCurrentServer = highestReadyServer;
  }

  // get the highest ready task in the highestReadyServer
  if (highestReadyServer != NULL && RelteqServerIsReady(highestReadyServer)) {
    task = RelteqServerHighestReadyTask(highestReadyServer);
  } else {
    task = RelteqPortTaskIdle;
  }
  
  //logMessage("highest task %s", RelteqPortTaskName(task), 0);

  // adjust the earliest event time, needed for multiplexing active queues
  RelteqUpdateEarliestTime();

  RelteqPortExitCritical();
  
  return task;
}

void RelteqServerMakeTaskReady(RelteqTask* task) {
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  INT8U err;
  RelteqEvent* event;
#endif

  RelteqServer* server = RelteqPortTaskServer(task);
  if (!server) return;

#if OS_RELTEQ_LOCAL_EDF_EN > 0
  event = RelteqPortTaskReadyQueueEvent(task);
  if (event) {
    RelteqQueueDeleteEvent(server->ServerReadyQueue, event, &err); ASSERT_OK(err);
  }
  // insert task into server's ready queue. We assume here that the task is not in the ready queue.
  event = RelteqEventCreate(kRelteqEventDeadline, task);
  RelteqQueueInsertEvent(server->ServerReadyQueue, event, RelteqPortTaskPeriod(task), &err); ASSERT_OK(err);
  RelteqPortTaskSetReadyQueueEvent(task, event);

#else
  if (!server->ServerHighestReadyTask || RelteqPortTaskPriority(server->ServerHighestReadyTask) > RelteqPortTaskPriority(task)) {
    server->ServerHighestReadyTask = task;
  }
#endif

  // update server's state depending on its current state
  switch (server->ServerState) {
    case kRelteqServerReady:
    case kRelteqServerRunning: 
    case kRelteqServerDepleted:
      // keep the current state
      break;
    case kRelteqServerWaiting:
       server->ServerState = kRelteqServerReady;
  }
}

void RelteqServerMakeTaskNotReady(RelteqTask* task) {
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  INT8U err;
#endif
  RelteqServer* server;

  ASSERT(task != NULL, "task is null");

  server = RelteqPortTaskServer(task);
  if (!server) return;

#if OS_RELTEQ_LOCAL_EDF_EN > 0
  // remove the task from server's ready queue
  RelteqQueueDeleteEvent(server->ServerReadyQueue, RelteqPortTaskReadyQueueEvent(task), &err); ASSERT_OK(err);
  RelteqPortTaskSetReadyQueueEvent(task, NULL);
#else
  // check if task is the highest ready task
  if (server->ServerHighestReadyTask == task) {
    // find a highest ready task. Note that this is done here (rather than in the scheduler) , since
    // every change in task ready state will result in a call to the scheduler. However, during 
    // a scheduler invocation the readyness of a server may not have changed.
    server->ServerHighestReadyTask = RelteqServerFindHighestReadyTask(server);
  }  
#endif

  // update server's state depending on its current state
  if (!RelteqServerIsReady(server)) {
    switch (server->ServerState) {
      case kRelteqServerDepleted:
      case kRelteqServerWaiting:
        // keep the current state
        break;
      case kRelteqServerReady:
      case kRelteqServerRunning: 
        server->ServerState = kRelteqServerWaiting;
    }
  }
}


/*
*********************************************************************************************************
*                                     ASSIGN A NAME TO A SERVER
*
* Description: This function is used to set the name of a server.
*
* Arguments  : prio      is the priority of the server that you want the assign a name to.
*
*              pname     is a pointer to an ASCII string that contains the name of the task.  The ASCII
*                        string must be NUL terminated.
*
*              perr       is a pointer to an error code that can contain one of the following values:
*
*                        OS_ERR_NONE                if the requested task is resumed
*                        OS_ERR_TASK_NOT_EXIST      if the server has not been created
*                        OS_ERR_TASK_NAME_TOO_LONG  if the name you are giving to the server exceeds the
*                                                   storage capacity of a task name as specified by
*                                                   OS_TASK_NAME_SIZE.
*                        OS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        OS_ERR_PRIO_INVALID        if you specified an invalid priority:
*                                                   A higher value than the idle task or not OS_PRIO_SELF.
*                        OS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/
#if OS_RELTEQ_NAME_SIZE > 1
void  RelteqServerNameSet(RelteqServer* server, char* name, INT8U *perr) {
#if OS_ARG_CHK_EN > 0
    if (perr == NULL) {                        
        return;
    }
    if (server == NULL) {                 
	    *perr = OS_ERR_RELTEQ_SERVER_NULL;             			 
	    return;
    }
    if (name == NULL) {                       
        *perr = OS_ERR_RELTEQ_NAME_NULL;                   
        return;
    }
#endif

    *perr = OS_ERR_NONE;

    RelteqPortStringCopy(server->ServerName, name, perr);

    // name also the queues
    RelteqPortStringPrint(server->ServerAbsoluteQueue->QueueName, "%s.%s", server->ServerName, "Abs");
    RelteqPortStringPrint(server->ServerRelativeQueue->QueueName, "%s.%s", server->ServerName, "Rel");
    RelteqPortStringPrint(server->ServerOverrunTime->QueueName, "%s.%s", server->ServerName, "OverrunTime");
    RelteqPortStringPrint(server->ServerReadyQueue->QueueName, "%s.%s", server->ServerName, "Tasks");

    *perr = OS_ERR_NONE;
}
#endif /* OS_RELTEQ_NAME_SIZE */


#if OS_RELTEQ_DEBUG_EN > 0

void RelteqDumpServer(RelteqServer* server) {
  if (server == NULL) {
    RelteqPortPrint("Server=NULL\n");
  } else {  
    RelteqPortPrint("Server %s:\n", server->ServerName);
    RelteqPortPrint("  ServerType=%d\n", server->ServerType);
    RelteqPortPrint("  ServerState=%d\n", server->ServerState);
    RelteqPortPrint("  ServerBudget=%d\n", server->ServerBudget);
    RelteqPortPrint("  ServerBudgetLeft=%d\n", server->ServerBudgetLeft);
    RelteqPortPrint("  ServerPeriod=%d\n", server->ServerPeriod);
    RelteqPortPrint("  ServerPriority=%d\n", server->ServerPriority);
    RelteqPortPrint("  ServerReadyQueue=\n");
    RelteqDumpQueue(server->ServerReadyQueue);
    RelteqPortPrint("  ServerOverrunTime=\n");
    RelteqDumpQueue(server->ServerOverrunTime);
    RelteqPortPrint("  ServerAbsoluteQueue=\n");
    RelteqDumpQueue(server->ServerAbsoluteQueue);
    RelteqPortPrint("  ServerRelativeQueue=\n");
    RelteqDumpQueue(server->ServerRelativeQueue);
  }
}

#endif /* OS_RELTEQ_DEBUG_EN */
