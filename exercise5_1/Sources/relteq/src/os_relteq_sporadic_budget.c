/*
 * NOTE: This code has not been tested. It also is not included from os_relteq.c.
 */

void RelteqServerSwitchInSporadicBudget(RelteqServer* server) {
  INT8U err;

#if OS_RELTEQ_SERVER_WAKEUP_EVENT_EN > 0
  // rempve the old wakeup event, if any, in case the server was switched in before it has 
  // expired, which could happen in case the server was switched out due to preemption.
  if (server->ServerWakeupEvent) {
    RelteqQueueDeleteEvent(RelteqGlobalQueue, server->ServerWakeupEvent, &err); ASSERT_OK(err);
    server->ServerWakeupEvent = NULL;
  }
  // process all events in the absolute queue which have passed
  RelteqQueueSyncWithQueueUntilEvent(server->ServerAbsoluteQueue, RelteqStopwatch, server->ServerStopwatchEvent, &err); ASSERT_OK(err);
#endif

  // deplete the budget immediately if no ready task
  RelteqServerCheckPollingServerDepleted(server);
}

void RelteqServerSwitchOutSporadicBudget(RelteqServer* server) {
  INT8U err;

  RelteqQueueDeactivate(server->ServerAbsoluteQueue, &err); ASSERT_OK(err);
  RelteqQueueDeactivate(server->ServerRelativeQueue, &err); ASSERT_OK(err);
  
  // in case no task is ready, deplete the server immediately (note that in case a task is ready
  // but blocked on a shared resource, then the server will retain its capacity)
  RelteqServerCheckPollingServerDepleted(server);
  
#if OS_LOG_EN > 0
  logServerPreempted(server);
#endif
}

void RelteqSporadicBudget_Init(void) {
  RelteqServerRegisterSwitchInHandler(kRelteqServerSporadicBudget, RelteqServerSwitchInSporadicBudget);
  RelteqServerRegisterSwitchOutHandler(kRelteqServerSporadicBudget, RelteqServerSwitchOutSporadicBudget);
}
