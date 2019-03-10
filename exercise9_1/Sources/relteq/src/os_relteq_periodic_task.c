void RelteqPeriodicTaskSetPeriod(RelteqTask* task, INT32U period, INT32U offset) {
  INT8U err;
  RelteqEvent* event;
  RelteqQueue* queue;
#if OS_RELTEQ_HSF_EN > 0 
  RelteqServer* server;
#endif
  
  ASSERT(period > 0, "Trying to set period 0 for task with priority %d", RelteqPortTaskPriority(task));

#if OS_RELTEQ_HSF_EN > 0 
  server = RelteqPortTaskServer(task);
  
#if OS_RELTEQ_LOCAL_EDF_EN > 0
  // In case of local EDF, make sure that task's period is set BEFORE adding it to a server
  ASSERT(server == NULL, "When using local EDF, you must set the task's period BEFORE adding it to a server.");
#endif
  
#endif

  event = RelteqPortTaskPeriodEvent(task);

  // make sure to set the period here, in case the user forgot to set it outside
  RelteqPortTaskSetPeriod(task, period);
  RelteqPortTaskSetOffset(task, offset);
  
  // if offset is not 0, then it means than we set the period event to expire after offset
  if (offset == 0) offset = period;
  
  // if the period was already set, then remove the old period event
	if ( event != NULL ) {							
#if OS_RELTEQ_HSF_EN > 0 
    // if task has a parent server ...
    if (server) {
      // ... remove the old period event from the absolute queue of the parent server
      RelteqQueueDeleteEvent(server->ServerAbsoluteQueue, event, &err); ASSERT_OK(err);
    } else {
      // ... otherwise remove from the global queue
      RelteqQueueDeleteEvent(RelteqGlobalQueue, event, &err); ASSERT_OK(err);
    }
#else
		RelteqQueueDeleteEvent(RelteqGlobalQueue, event, &err); ASSERT_OK(err);
#endif
  }
  
  // create a new period event
  event = RelteqEventCreate(kRelteqEventPeriod, task);

  // Insert first period event into the appropriate queue
#if OS_RELTEQ_HSF_EN > 0 
  // if task has a parent server ...
  if (server) {
    // ... add the new period event to the absolute queue of the parent server
    queue = server->ServerAbsoluteQueue;
  } else {
    // ... otherwise add to the global queue
 	  queue = RelteqGlobalQueue;
  }
#else
 	queue = RelteqGlobalQueue;
#endif
  RelteqQueueInsertEvent(queue, event, offset, &err);  ASSERT_OK(err);

  // since RelteqPeriodicTaskSetPeriod may be called outside of the tick handler, we need to
  // update the earliest time, to make sure that newly inserted period event gets noticed.
  RelteqQueueUpdateEarliestTime(queue);

  // store the period event, so that we can remove it from the queue in case the period is reset
  RelteqPortTaskSetPeriodEvent(task, event);
}

/*
 * Handle the arrival of a periodic task, by 
 * 1. inserting a new period event into the appropriate queue
 * 2. posting the corresponding period semaphore
 */

void RelteqPeriodicTaskArrived(RelteqTask* task) {
  INT8U err;
  
  // create a new period event
  RelteqEvent* event = RelteqEventCreate(kRelteqEventPeriod, task);

  // insert the event into the the appropriate queue
#if OS_RELTEQ_HSF_EN > 0
  RelteqServer* server = RelteqPortTaskServer(task);
  if (server) {
    RelteqQueueInsertEvent(server->ServerAbsoluteQueue, event, RelteqPortTaskPeriod(task), &err); ASSERT_OK(err);
  } else {
    RelteqQueueInsertEvent(RelteqGlobalQueue, event, RelteqPortTaskPeriod(task), &err); ASSERT_OK(err);
  }
#else
  RelteqQueueInsertEvent(RelteqGlobalQueue, event, RelteqPortTaskPeriod(task), &err); ASSERT_OK(err);
#endif

  // store the period event, so that we can remove it from the queue in case the period is reset
  RelteqPortTaskSetPeriodEvent(task, event);

  // indicate to the task that the period has arrived
  RelteqPortTaskPeriodArrived(task);
  
#if OS_RELTEQ_HSF_EN > 0
  if (server) {
    // indicate to the parent server that a task has arrived
    ASSERT(RelteqServerTaskArrivedHandlers[server->ServerType] != NULL, "(RelteqServerTaskArrivedHandler is NULL for server type %d", server->ServerType);
    RelteqServerTaskArrivedHandlers[server->ServerType](server, task);
  }
#endif
}

void RelteqHandlePeriod(RelteqQueue* queue, RelteqEvent* event) {
  RelteqTask* task = event->EventPtr;
#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
  logCustomEvent("relteqEvent period \"%s %s\"", RelteqPortTaskName(task), queue->QueueName);
#endif      

  // delete the old period event. delete it before handling sporadic tasks, to make sure that
  // it is indeed deleted, even in case the sporadic task still has to wait for the event arrival. 
  RelteqEventDel(event);
  RelteqPortTaskSetPeriodEvent(task, NULL);

#if OS_RELTEQ_SPORADIC_TASK_EN > 0
  if (RelteqPortTaskIsSporadic(task)) {
    RelteqPortSporadicTaskSetPeriodExpired(task);
    // check if both the event and the task have arrived, otherwise break
    if (RelteqPortSporadicTaskEventArrived(task)) {
      RelteqPortSporadicTaskClear(task);
    } else {
      return;
    }
  }
#endif

  RelteqPeriodicTaskArrived(task);
}

/*
 * RelteqHandleDeadline()
 */

void RelteqHandleDeadline(RelteqQueue* queue, RelteqEvent* event) {
  (void)queue;
#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
  RelteqTask* task = event->EventPtr;
  logCustomEvent("relteqEvent deadline \"%s\"", RelteqPortTaskName(task), 0);
#endif      
  RelteqEventDel(event);
}

/*
 * RelteqPeriodic_Init()
 */

void RelteqPeriodicTask_Init(void) {
  RelteqRegisterEventHandler(kRelteqEventPeriod, RelteqHandlePeriod);
  RelteqRegisterEventHandler(kRelteqEventDeadline, RelteqHandleDeadline);
}
