void RelteqServerInitialize(RelteqServer* server, INT8U* pErr) {
  INT8U err;  
  RelteqEvent* event;

  pErr = OS_ERR_NONE;

  // create the replenishment event
  event = RelteqEventCreate(kRelteqEventReplenishment, server);
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
  RelteqQueueInsertEvent(RelteqGlobalQueue, event, server->ServerPeriod, &err); ASSERT_OK(err);

  // create the deplenishment event
  event = RelteqEventCreate(kRelteqEventDepletion, server);
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
  RelteqQueueInsertEvent(server->ServerRelativeQueue, event, server->ServerBudget, &err); ASSERT_OK(err);
  server->ServerDepletionEvent = event;
}

/*
 * Replenishment event handler.
 */
 
void RelteqServerHandleReplenishment(RelteqQueue* queue, RelteqEvent* event) {
  INT8U err;
  RelteqServer* server = (RelteqServer*)event->EventPtr;
  RelteqEventDel(event);

  // create a new replenishment event and insert into the global queue
  event = RelteqEventCreate(kRelteqEventReplenishment, server);
  RelteqQueueInsertEvent(queue, event, server->ServerPeriod, &err); ASSERT_OK(err);
  
  // adjust servers budget
  server->ServerBudgetLeft = server->ServerBudget;
  
  // the new server state depends on the current state
  switch (server->ServerState) {
    case kRelteqServerReady:
    case kRelteqServerRunning: 
    case kRelteqServerWaiting:
      // keep the current state
      break;
    case kRelteqServerDepleted:
      server->ServerState = kRelteqServerReady;
  }
  
  // delete the depletion event from server's relative queue (if not already depleted)
  if (server->ServerDepletionEvent) {
    RelteqQueueDeleteEvent(server->ServerRelativeQueue, server->ServerDepletionEvent, &err); ASSERT_OK(err);
    server->ServerDepletionEvent = NULL;
  }
  
  // create a new depletion event and insert into server's relative queue
  event = RelteqEventCreate(kRelteqEventDepletion, server);
  RelteqQueueInsertEvent(server->ServerRelativeQueue, event, server->ServerBudgetLeft, &err); ASSERT_OK(err);
  server->ServerDepletionEvent = event;
  
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  // set server's deadline to its period, by postponing its deadline event in the global ready queue
  if (server->ServerGlobalReadyQueueEvent) {
    RelteqQueueDeleteEvent(RelteqGlobalReadyQueue, server->ServerGlobalReadyQueueEvent, &err); ASSERT_OK(err);
  }
  event = RelteqEventCreate(kRelteqEventServerDeadline, server);
  RelteqQueueInsertEvent(RelteqGlobalReadyQueue, event, server->ServerDeadline, &err); ASSERT_OK(err);
  server->ServerGlobalReadyQueueEvent = event;
#endif

#if OS_LOG_EN > 0
  logServerReplenished(server, server->ServerBudgetLeft);
#if OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent replenished \"%s\"", server->ServerName, NULL);
#else
  logCustomEvent("relteqEvent replenished \"Server%d\"", (void*)(long)server->ServerPriority, NULL);
#endif
#endif
#endif                 
}

/*
 * Depletion event handler.
 */
 
void RelteqServerHandleDepletion(RelteqQueue* queue, RelteqEvent* event) {
  RelteqServer* server = (RelteqServer*)event->EventPtr;
  RelteqEventDel(event);
  server->ServerDepletionEvent = NULL;

  // mark server as depleted (i.e. it has pending workload, but no budget)
  server->ServerState = kRelteqServerDepleted;

#if OS_RELTEQ_NAME_SIZE > 0  
  ASSERT(server->ServerBudgetLeft == 0, "something wrong with budget keeping of %s, budget=%d, queue=%s", server->ServerName, server->ServerBudgetLeft, queue->QueueName);
#else
  ASSERT(server->ServerBudgetLeft == 0, "something wrong with budget keeping, budget=%d", server->ServerBudgetLeft);
#endif  

#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent depleted \"%s\"", server->ServerName, NULL);
#else
  logCustomEvent("relteqEvent depleted \"Server%d\"", (void*)(long)server->ServerPriority, NULL);
#endif
#endif
}

void RelteqServerHandleServerDeadline(RelteqQueue* queue, RelteqEvent* event) {
  RelteqServer* server = (RelteqServer*)event->EventPtr;
  RelteqEventDel(event);
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
  server->ServerGlobalReadyQueueEvent = NULL;
#endif

  // a server past its deadline becomes waiting
  server->ServerState = kRelteqServerWaiting;
  
#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent serverDeadline \"%s (%s)\"", server->ServerName, queue->QueueName);
#else
  logCustomEvent("relteqEvent serverDeadline \"Server%d (%s)\"", (void*)(long)server->ServerPriority, queue->QueueName);
#endif
#endif
}

void RelteqServerHandleTaskArrived(RelteqServer* server, RelteqTask* task) {
  (void)server;
  (void)task;
}
