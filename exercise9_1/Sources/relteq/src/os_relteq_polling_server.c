INT8U RelteqServerCheckPollingServerDepleted(RelteqServer* server) {
  INT8U err;
  if (!RelteqServerIsReady(server)) {
    server->ServerBudgetLeft = 0;
    server->ServerState = kRelteqServerDepleted;
    if (server->ServerDepletionEvent) {
      RelteqQueueDeleteEvent(server->ServerRelativeQueue, server->ServerDepletionEvent, &err); ASSERT_OK(err);
      server->ServerDepletionEvent = NULL;
    }
#if OS_LOG_EN > 0
    logServerDepleted(server);
#endif
    return 1;
  }
  
  return 0;
}

void RelteqServerSwitchInPolling(RelteqServer* server) {
  INT8U err;

#if OS_RELTEQ_LOCAL_EDF_EN > 0
  // process all events in the ready queue which have passed
  // IMPORTANT: the order in which queues are synchronized with stopwatch matters! Deadlines in 
  // the ready queue must be processed before the periodic task arrivals in the absolute queue.
  // Therefore, the ready queue must be sunchronized before absolute queue.
  RelteqQueueSyncWithQueueUntilEvent(server->ServerReadyQueue, RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);
#endif

  // process all events in the absolute queue which have passed
  RelteqQueueSyncWithQueueUntilEvent(server->ServerAbsoluteQueue, RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);

  // deplete the budget immediately if no ready task
  RelteqServerCheckPollingServerDepleted(server);
}

void RelteqServerSwitchOutPolling(RelteqServer* server) {
  INT8U err;

  RelteqQueueDeactivate(server->ServerAbsoluteQueue, &err); ASSERT_OK(err);
  RelteqQueueDeactivate(server->ServerRelativeQueue, &err); ASSERT_OK(err);
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  RelteqQueueDeactivate(server->ServerReadyQueue, &err); ASSERT_OK(err);
#endif
  
  // in case no task is ready, deplete the server immediately (note that in case a task is ready
  // but blocked on a shared resource, then the server will retain its capacity)
  RelteqServerCheckPollingServerDepleted(server);
  
#if OS_LOG_EN > 0
  logServerPreempted(server);
#endif
}

void RelteqPollingServer_Init(void) {
  RelteqServerRegisterInitializer(kRelteqServerPolling, RelteqServerInitialize);
  RelteqServerRegisterSwitchInHandler(kRelteqServerPolling, RelteqServerSwitchInPolling);
  RelteqServerRegisterSwitchOutHandler(kRelteqServerPolling, RelteqServerSwitchOutPolling);
  RelteqServerRegisterTaskArrivedHandler(kRelteqServerPolling, RelteqServerHandleTaskArrived);
}
