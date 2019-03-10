inline void RelteqMakeSporadic(RelteqTask* task, INT8U* pErr) {
  *pErr = OS_ERR_NONE;
  RelteqPortTaskMakeSporadic(task);
}

void RelteqSporadicEventArrived(RelteqTask* task) {
  // indicate that the task has arrived
  RelteqPortSporadicTaskSetEventArrived(task);
  
  if (RelteqPortSporadicTaskPeriodExpired(task)) {
#if OS_RELTEQ_HSF_EN > 0
      RelteqServer* server = RelteqPortTaskServer(task);
      INT8U err;
      if (server) {
      /* If task belongs to a server, insert the periodic event at the head of the relative queue.
         We want to handle the periodc event as soon as the server is switched in. Inserting it into
         the absolute queue could place it earlier than now if the server is incative. This will be
         a problem if the server was switched out, since handling the period event inserts a new
         period event relative to the handled event in the absolute queue (i.e. in the past). Since
         the relative queue is synchronised with 0 (in RelteqServerSwitchIn()) after the absolute 
         queue has been synchronized with the stopwatch, when the period event residing at the head 
         of the realtive queue is handled its next period event will be inserted into the absolute 
         queue at the right spot.
         Since RelteqSporadicFlagPeriodArrived was true, at this point we know that the period
         event has occured and that it was removed from the absolute queue, so we don't need to
         worry about removing an existing period event. */
        RelteqEvent* event = RelteqEventCreate(kRelteqEventPeriod, task);
        RelteqQueueInsertEvent(server->ServerRelativeQueue, event, 0, &err); ASSERT_OK(err);
        /* update server's state depending on its current state */
        switch (server->ServerState) {
          case kRelteqServerReady:
          case kRelteqServerRunning: 
          case kRelteqServerDepleted:
            /* keep the current state */
            break;
          case kRelteqServerWaiting:
             server->ServerState = kRelteqServerReady;
             RelteqNeedScheduling = 1;
        }
      } else {
        /* If the task is not assigned to any server, than we could insert the period event into
           the global queue. Since the global queue is always active the event would be handled
           immediately, so instead we handle the event directly. */
        RelteqPortSporadicTaskClear(task);
        RelteqPeriodicTaskArrived(task);        
      }
#else
      /* if servers are disabled, then simply handle the periodic task */
      RelteqPortSporadicTaskClear(task);
      RelteqPeriodicTaskArrived(task);
#endif
  }
}
