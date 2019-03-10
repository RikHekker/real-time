void RelteqServerSwitchInDeferrable(RelteqServer* server) {
  INT8U err;
#if OS_RELTEQ_SERVER_WAKEUP_EVENT_EN > 0
  // rempve the old wakeup event, if any, in case the server was switched in before it has 
  // expired, which could happen in case the server was switched out due to preemption.
  if (server->ServerWakeupEvent) {
    RelteqQueueDeleteEvent(RelteqGlobalQueue, server->ServerWakeupEvent, &err); ASSERT_OK(err);
    server->ServerWakeupEvent = NULL;
  }
  
  
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  // process all events in the ready queue which have passed
  // IMPORTANT: the order in which queues are synchronized with stopwatch matters! Deadlines in 
  // the ready queue must be processed before the periodic task arrivals in the absolute queue.
  // Therefore, the ready queue must be sunchronized before absolute queue.
  RelteqQueueSyncWithQueueUntilEvent(server->ServerReadyQueue, RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);
#endif
  
  // process all events in the absolute queue which have passed
  RelteqQueueSyncWithQueueUntilEvent(server->ServerAbsoluteQueue, RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);
#endif
}

void RelteqServerSwitchOutDeferrable(RelteqServer* server) {
  INT8U err;

  RelteqQueueDeactivate(server->ServerRelativeQueue, &err); ASSERT_OK(err);

#if OS_RELTEQ_SERVER_WAKEUP_EVENT_EN > 0
  RelteqQueueDeactivate(server->ServerAbsoluteQueue, &err); ASSERT_OK(err);
  // insert a new wakeup event into the global system queue
  // note that since a server is always switched in (where any wakeup event is removed from
  // the system queue) before being switched out, we are sure that there is no wakeup event for 
  // this server already in the system queue.
  if (server->ServerAbsoluteQueue->QueueHead) {
    RelteqTime time = server->ServerAbsoluteQueue->QueueHead->EventTime;
    server->ServerWakeupEvent = RelteqEventCreate(kRelteqEventWakeup, server);
    RelteqQueueInsertEvent(RelteqGlobalQueue, server->ServerWakeupEvent, time, &err); ASSERT_OK(err);
  }
#endif

#if OS_RELTEQ_LOCAL_EDF_EN > 0
  RelteqQueueDeactivate(server->ServerReadyQueue, &err); ASSERT_OK(err);
#endif

#if OS_LOG_EN > 0
  logServerPreempted(server);
#endif
}

void RelteqDeferrableServer_Init(void) {
  RelteqServerRegisterInitializer(kRelteqServerDeferrable, RelteqServerInitialize);
  RelteqServerRegisterSwitchInHandler(kRelteqServerDeferrable, RelteqServerSwitchInDeferrable);
  RelteqServerRegisterSwitchOutHandler(kRelteqServerDeferrable, RelteqServerSwitchOutDeferrable);
  RelteqServerRegisterTaskArrivedHandler(kRelteqServerDeferrable, RelteqServerHandleTaskArrived);
}
