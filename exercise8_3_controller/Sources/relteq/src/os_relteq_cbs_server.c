#if OS_RELTEQ_GLOBAL_EDF_EN == 0
  #error "os_relteq_hsf_cbs.c requires OS_RELTEQ_GLOBAL_EDF_EN to be enabled."
#endif

void RelteqServerInitializeCBS(RelteqServer* server, INT8U* pErr) {
  INT8U err;  
  RelteqEvent* event;

  pErr = OS_ERR_NONE;

  // create the deplenishment event
  event = RelteqEventCreate(kRelteqEventDepletionCBS, server);
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
  RelteqQueueInsertEvent(server->ServerRelativeQueue, event, server->ServerBudget, &err); ASSERT_OK(err);
  server->ServerDepletionEvent = event;

  // Note that we DO NOT create a replenishment event, since CBS server does not handle them.
}

INT32U RelteqQueueAccumulateUntilEvent(RelteqQueue* queue, RelteqEvent* stopEvent, INT8U* pErr) {
  RelteqEvent* event;
  INT32U time;

#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return 0;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return 0;
  }
  if (stopEvent == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return 0;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // traverse the queue and accumulate the time
  time = 0;
  for (event = queue->QueueHead; event != NULL && event != stopEvent; event = event->EventNext) {
    if (time > RelteqMaxTime - event->EventTime) {
      *pErr = OS_ERR_RELTEQ_TIME_TOO_LARGE;
      return 0;
    } else {
      time += event->EventTime;
    }
  }

  if (event) {
    return time + event->EventTime;
  } else {
    *pErr = OS_ERR_RELTEQ_EVENT_NOT_FOUND;
    return 0;
  }
}

/*
 * Check if server was idle before this task arrived, i.e. if it has at most one task in its ready 
 * queue. Note that it may have non tasks, if it is called from within a wakeup event, where
 * the proper task arrival handling is postponed.
 */

#if OS_RELTEQ_LOCAL_EDF_EN > 0
  #define RelteqServerWasIdle(server) (server->ServerReadyQueue->QueueHead && !server->ServerReadyQueue->QueueHead->EventNext)
#else
  int RelteqServerWasIdle(RelteqServer* server) {
    RelteqEvent* event;
    int counter = 0;    
    for (event = server->ServerReadyQueue->QueueHead; event != NULL; event = event->EventNext) {
      if (RelteqPortTaskIsReady((RelteqTask*)event->EventPtr)) {
        counter++;
        if (counter > 1) { break; }
      }
    }
    return (counter <= 1);
  }
#endif

/*
 * Depletion event handler.
 */
 
void RelteqServerHandleDepletionCBS(RelteqQueue* queue, RelteqEvent* event) {
  INT8U err;  
  INT32U deadline;
  RelteqServer* server = (RelteqServer*)event->EventPtr;

  RelteqEventDel(event);
  server->ServerDepletionEvent = NULL;
    
#if OS_RELTEQ_NAME_SIZE > 0
  ASSERT(server->ServerBudgetLeft == 0, "something wrong with budget keeping of %s, budget=%d, queue=%s", server->ServerName, server->ServerBudgetLeft, queue->QueueName);
#else
  ASSERT(server->ServerBudgetLeft == 0, "something wrong with budget keeping, budget=%d", server->ServerBudgetLeft);
#endif
  
  // postpone server's deadline by its period, by postponing its deadline event in the global ready queue
  deadline = RelteqQueueAccumulateUntilEvent(RelteqGlobalReadyQueue, server->ServerGlobalReadyQueueEvent, &err); ASSERT_OK(err);
  RelteqQueueDeleteEvent(RelteqGlobalReadyQueue, server->ServerGlobalReadyQueueEvent, &err); ASSERT_OK(err);
  event = RelteqEventCreate(kRelteqEventServerDeadline, server);
  RelteqQueueInsertEvent(RelteqGlobalReadyQueue, event, deadline + server->ServerPeriod, &err); ASSERT_OK(err);
  server->ServerGlobalReadyQueueEvent = event;

  // replenish server's budget
  server->ServerBudgetLeft = server->ServerBudget;

#if OS_LOG_EN > 0
  logServerReplenished(server, server->ServerBudgetLeft);
#endif

  // delete the depletion event from server's relative queue (if not already depleted)
  // Note that the event is already popped from the queue, since we are handling the event.
  
  // create a new depletion event and insert into server's relative queue
  event = RelteqEventCreate(kRelteqEventDepletionCBS, server);
  RelteqQueueInsertEvent(server->ServerRelativeQueue, event, server->ServerBudgetLeft, &err); ASSERT_OK(err);
  server->ServerDepletionEvent = event;

#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent depleted \"CBS %s\"", server->ServerName, NULL);
#else
  logCustomEvent("relteqEvent depleted \"CBS Server%d\"", (void*)(long)server->ServerPriority, NULL);
#endif
#endif
}

void RelteqServerHandleTaskArrivedCBS(RelteqServer* server, RelteqTask* task) {
  INT8U err;  
  RelteqEvent* event;
  INT32U deadline = 0;

  // check if server was idle before this task arrived
  if (server->ServerGlobalReadyQueueEvent) {
    deadline = RelteqQueueAccumulateUntilEvent(RelteqGlobalReadyQueue, server->ServerGlobalReadyQueueEvent, &err); ASSERT_OK(err);
  }

  // check if task was idle  
  if (RelteqServerWasIdle(server) && ((server->ServerBudgetLeft)*(server->ServerPeriod) > deadline*(server->ServerBudget)) ) {
    // set server's deadline to its period, by postponing its deadline event in the global ready queue
    if (server->ServerGlobalReadyQueueEvent) {
      RelteqQueueDeleteEvent(RelteqGlobalReadyQueue, server->ServerGlobalReadyQueueEvent, &err); ASSERT_OK(err);
    }
    event = RelteqEventCreate(kRelteqEventServerDeadline, server);
    RelteqQueueInsertEvent(RelteqGlobalReadyQueue, event, server->ServerPeriod, &err); ASSERT_OK(err);
    server->ServerGlobalReadyQueueEvent = event;
    
    // replenish server's budget
    server->ServerBudgetLeft = server->ServerBudget;
  
#if OS_LOG_EN > 0
    logServerReplenished(server, server->ServerBudgetLeft);
#endif

    // delete the depletion event from server's relative queue (if not already depleted)
    if (server->ServerDepletionEvent) {
      RelteqQueueDeleteEvent(server->ServerRelativeQueue, server->ServerDepletionEvent, &err); ASSERT_OK(err);
    }
    // create a new depletion event and insert into server's relative queue
    event = RelteqEventCreate(kRelteqEventDepletionCBS, server);
    RelteqQueueInsertEvent(server->ServerRelativeQueue, event, server->ServerBudgetLeft, &err); ASSERT_OK(err);
    server->ServerDepletionEvent = event;
  }
}

void RelteqCBSHandleWakeup(RelteqServer* server) {
  RelteqEvent* firstEvent = server->ServerAbsoluteQueue->QueueHead;
  if (firstEvent->EventKind == kRelteqEventPeriod) {
    RelteqServerHandleTaskArrivedCBS(server, (RelteqTask*)firstEvent->EventPtr);
  }
}

void RelteqCBSServer_Init(void) {
  RelteqServerRegisterInitializer(kRelteqServerCBS, RelteqServerInitializeCBS);
  RelteqServerRegisterSwitchInHandler(kRelteqServerCBS, RelteqServerSwitchInDeferrable);
  RelteqServerRegisterSwitchOutHandler(kRelteqServerCBS, RelteqServerSwitchOutDeferrable);
  RelteqServerRegisterTaskArrivedHandler(kRelteqServerCBS, RelteqServerHandleTaskArrivedCBS);
  RelteqServerRegisterWakeupHandler(kRelteqServerCBS, RelteqCBSHandleWakeup);

  // register CBS relteq event handlers
  RelteqRegisterEventHandler(kRelteqEventDepletionCBS, RelteqServerHandleDepletionCBS);
}
