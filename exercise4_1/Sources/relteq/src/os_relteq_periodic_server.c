void RelteqServerSwitchInPeriodic(RelteqServer* server) {
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
}

void RelteqServerSwitchOutPeriodic(RelteqServer* server) {
  INT8U err;

  RelteqQueueDeactivate(server->ServerAbsoluteQueue, &err); ASSERT_OK(err);
  RelteqQueueDeactivate(server->ServerRelativeQueue, &err); ASSERT_OK(err);
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  RelteqQueueDeactivate(server->ServerReadyQueue, &err); ASSERT_OK(err);
#endif

#if OS_LOG_EN > 0
  logServerPreempted(server);
#endif
}

void RelteqPeriodicServer_Init(void) {
  RelteqServerRegisterInitializer(kRelteqServerPeriodic, RelteqServerInitialize);
  RelteqServerRegisterSwitchInHandler(kRelteqServerPeriodic, RelteqServerSwitchInPeriodic);
  RelteqServerRegisterSwitchOutHandler(kRelteqServerPeriodic, RelteqServerSwitchOutPeriodic);
  RelteqServerRegisterTaskArrivedHandler(kRelteqServerPeriodic, RelteqServerHandleTaskArrived);
}
