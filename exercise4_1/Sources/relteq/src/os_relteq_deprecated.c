INT16U RelteqEventChainLength(RelteqEvent* event) {
  INT16U length;

  // traverse the queue and accumulate the time
  length = 0;
  for (; event != NULL; event = event->EventNext) {
    length++;
  }
  return length;
}

INT16U RelteqQueueLength(RelteqQueue* queue, INT8U* pErr) {
  INT16U length;
  RelteqEvent* event;
  
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return 0;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return 0;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // traverse the queue and accumulate the time
  length = 0;
  for (event = queue->QueueHead; event != NULL; event = event->EventNext) {
    length++;
  }
  return length;
}

void RelteqQueueSyncWithQueue(RelteqQueue* targetQueue, RelteqQueue* sourceQueue, INT8U* pErr) {
  INT8U err;
  
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (targetQueue == NULL || sourceQueue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  
  if (targetQueue == sourceQueue) {
    *pErr = OS_ERR_RELTEQ_QUEUE_SAME;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;  
//RelteqPortPrint("%d sync with queue\n", OSTimeGet());   

  RelteqEvent* source = sourceQueue->QueueHead;
  RelteqEvent* target = targetQueue->QueueHead;
  
  // traverse the two queues, until the either one of them is exhausted
  while (source != NULL && target != NULL) {
    // match the current source event with the target event
    while (source != NULL && source->EventTime <= target->EventTime) {
      // decerement the target time
      target->EventTime -= source->EventTime;

      // remove the event from the queue      
      RelteqQueuePop(sourceQueue, &err);

      // handle the source event (check for dummy to avoid the overhead of RelteqEventHandle())
      if (source->EventKind != kRelteqEventDummy) {
        RelteqEventHandle(sourceQueue, source, &err);
      }
      
      // move to the next event
      source = sourceQueue->QueueHead;
    }
    /* source == NULL or source.time > target.time */
    
    // check if reached the end of the source queue
    if (source == NULL) break;
    
    /* source.time > target.time */

    // decrement the source time
    source->EventTime -= target->EventTime;
    
    // remove the event from the queue      
    RelteqQueuePop(targetQueue, &err);

    // handle the target event (check for dummy to avoid the overhead of RelteqEventHandle())
//    if (target->EventKind != kRelteqEventDummy) {
      RelteqEventHandle(targetQueue, target, &err);
//    }
    
    // move to the next event
    target = targetQueue->QueueHead;
  }
}

void RelteqQueueAppendACopy(RelteqQueue* targetQueue, RelteqQueue* sourceQueue, INT8U* pErr) {
  INT8U err;
  RelteqEvent* source;
  
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (targetQueue == NULL || sourceQueue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  
  if (targetQueue == sourceQueue) {
    *pErr = OS_ERR_RELTEQ_QUEUE_SAME;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // traverse the source queue
  for (source = sourceQueue->QueueHead; source != NULL; source = source->EventNext) {
    // if appending a dummy event, try to merge it with the folloowing event
    if (source->EventKind == kRelteqEventDummy
    &&  targetQueue->QueueHead 
    &&  source->EventTime + targetQueue->QueueHead->EventTime < RelteqMaxTime) {

//      RelteqPortPrint("merge %d with queue %s\n", source->EventTime, targetQueue->QueueName);
      targetQueue->QueueHead->EventTime += source->EventTime;

    } else {
//       RelteqPortPrint("append %d to queue %s (%p)\n", source->EventTime, targetQueue->QueueName, targetQueue->QueueHead);
     // otherwise create a new event
      RelteqEvent* event = RelteqEventCreate(source->EventKind, source->EventTime, source->EventPtr);
      if (event == NULL) {
        *pErr = OS_ERR_RELTEQ_EVENT_NULL;
        return;
      }
      RelteqQueueLinkEvent(targetQueue, NULL, event, targetQueue->QueueHead, &err); ASSERT_OK(err);
    }
  }
}

RelteqTime RelteqQueueAccumulate(RelteqQueue* queue, INT8U* pErr) {
  RelteqEvent* event;
  RelteqTime time;

#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return 0;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return 0;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // traverse the queue and accumulate the time
  time = 0;
  for (event = queue->QueueHead; event != NULL; event = event->EventNext) {
    if (time + event->EventTime > RelteqMaxTime) {
      *pErr = OS_ERR_RELTEQ_TIME_TOO_LARGE;
      return 0;
    } else {
      time += event->EventTime;
    }
  }

  return time;
}

RelteqTime RelteqQueueAccumulateUntilEvent(RelteqQueue* queue, RelteqEvent* stopEvent, INT8U* pErr) {
  RelteqEvent* event;
  RelteqTime time;

#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return 0;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return 0;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // traverse the queue and accumulate the time
  time = 0;
  for (event = queue->QueueHead; event != NULL && event != stopEvent; event = event->EventNext) {
    if (time + event->EventTime > RelteqMaxTime) {
      *pErr = OS_ERR_RELTEQ_TIME_TOO_LARGE;
      return 0;
    } else {
      time += event->EventTime;
    }
  }

  return time;
}

void RelteqQueueClear(RelteqQueue* queue, INT8U* pErr) {
  INT8U err;
  RelteqEvent* event;
  
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // traverse the source queue
  while (queue->QueueHead != NULL) {
    event = queue->QueueHead;
    RelteqQueuePop(queue, &err);
    RelteqEventDel(event);
  }
}

/*
 * This scheduler is deprecated, because it scans through all servers in each turn, rather than
 * using a priority queue, which allows to abort the search as soon as the highest priority ready
 * server is found.
 */
 
RelteqServer* RelteqServerSchedulerFPS(void) {
  INT8U i;
  RelteqServer* server;
  RelteqServer* highestReadyServer = NULL;
  
  // traverse through all created servers, which are not depleted ...
  // (note that we also need to check the RelteqCurrentServer, since it may have become unready)
  for (i = 0; i < OS_RELTEQ_MAX_SERVERS; i++) {
    server = &RelteqServers[i];

    if (server->ServerType != kRelteqServerNone
    && (server->ServerState == kRelteqServerReady || server->ServerState == kRelteqServerRunning)
    && (highestReadyServer == NULL || server->ServerPriority < highestReadyServer->ServerPriority) ) {
      highestReadyServer = server;
    }
  }
  
  return highestReadyServer;
}

/*
 * These three methods, which are called when a task becomes ready or not ready, assume a fixed
 * priority local scheduling, as in they simply mark the task ready or not ready. Upon scheduling
 * the ready queue is traversed looking for the highest priority ready task. Alternatively,
 * the task can be inserted or removed here from a priority queue, allowing the scheduler to simply
 * inspect the head of the queue, and to easily implement fixed and dynamic priority schedulers.
 */

void RelteqServerMakeTaskReady(OS_TCB* ptask) {
  OS_TCB* task = ptask;
  RelteqServer* server = (RelteqServer*)task->OSTCBServer;
  if (!server) return;
  if (!server->ServerHighestReadyTask || server->ServerHighestReadyTask->OSTCBPrio > task->OSTCBPrio) {
    server->ServerHighestReadyTask = task;
  }

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

void RelteqServerMakeTaskNotReady(OS_TCB* ptask) {
  OS_TCB* task = ptask;
  RelteqServer* server = (RelteqServer*)task->OSTCBServer;
  if (!server) return;
  // check if task is the highest ready task
  if (server->ServerHighestReadyTask == task) {
    // find a highest ready task. Note that this is done here (rather than in the scheduler) , since
    // every change in task ready state will result in a call to the scheduler. However, during 
    // a scheduler invocation the readyness of a server may not have changed.
    server->ServerHighestReadyTask = RelteqServerHighestReadyTask(server);
  }
  
  // update server's state depending on its current state
  if (!server->ServerHighestReadyTask) {
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

OS_TCB* RelteqServerHighestReadyTask(RelteqServer* server) {
  RelteqEvent* event;
  OS_TCB* task;
  INT8U taskReady;
  
  // scan through all the tasks in the queue until a ready task is found. Note that the queue is
  // assumed to be sorted by priority.
  for (event = server->ServerTasks->QueueHead; event != NULL; event = event->EventNext) {
    task = (OS_TCB*)event->EventPtr;
    taskReady = (OSRdyTbl[task->OSTCBY] & task->OSTCBBitX) != 0;
    if (taskReady) {
      return task;
    }
  }
  
#if OS_RELTEQ_PERIODIC_SERVER_EN > 0
  if (server->ServerType == kRelteqServerPeriodic) {
    return OSTCBPrioTbl[OS_TASK_IDLE_PRIO];
  }
#endif

  return NULL;
}

/*
 * Replenishment event handler.
 */
 
void RelteqServerHandleReplenishment(RelteqQueue* queue, RelteqEvent* event) {
  INT8U err;
  RelteqServer* server = (RelteqServer*)event->EventPtr;

  RelteqEventDel(event);

  // create a new replenishment event and insert into the global queue
  event = RelteqEventCreate(kRelteqEventReplenishment, 0, server);
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
  event = RelteqEventCreate(kRelteqEventDepletion, 0, server);
  RelteqQueueInsertEvent(server->ServerRelativeQueue, event, server->ServerBudgetLeft, &err); ASSERT_OK(err);
  server->ServerDepletionEvent = event;

#if OS_LOG_EN > 0
  logServerReplenished(server, server->ServerBudgetLeft);
#if OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent replenished \"%s\"", server->ServerName, 0);
#else
  logCustomEvent("relteqEvent replenished \"Server%d\"", (void*)(long)server->ServerPriority, 0);
#endif
#endif
#endif                 
}

/*
 * Depletion event handler.
 */
 
void RelteqServerHandleDepletion(RelteqQueue* queue, RelteqEvent* event) {
  RelteqServer* server = (RelteqServer*)event->EventPtr;

  // mark server as depleted (i.e. it has pending workload, but no budget)
  server->ServerState = kRelteqServerDepleted;
  
  ASSERT(server->ServerBudgetLeft == 0, "time=%d, something wrong with budget keeping of %s, budget=%d, queue=%s", OSTimeGet(), server->ServerName, server->ServerBudgetLeft, queue->QueueName);
  
  server->ServerDepletionEvent = NULL;

  RelteqEventDel(event);

#if OS_LOG_EN > 0  && OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent depleted \"%s\"", server->ServerName, 0);
#else
  logCustomEvent("relteqEvent depleted \"Server%d\"", (void*)(long)server->ServerPriority, 0);
#endif
#endif
}

/*
 * This was an attempt at responding ot a change in the server state. In case of global EDF, we
 * would insert or remove the server from the global queue. However, it is not necessary, as the
 * deadline of a server can only change when the server depletes or a task arrives.
 */
 
#if OS_RELTEQ_GLOBAL_EDF_EN > 0
void RelteqServerMakeServerReady(RelteqServer* server, RelteqServerState state) {
  
}

void RelteqServerMakeServerNotReady(RelteqServer* server, RelteqServerState state) {  

}

void RelteqServerSetState(RelteqServer* server, RelteqServerState state) {  
  // insert into the global ready queue, if not already inserted
  if ((server->ServerState == kRelteqServerReady) || (server->ServerState == kRelteqServerRunning)
    && (state == kRelteqServerDepleted) || (state == kRelteqServerWaiting)) {
    RelteqServerMakeServerNotReady(server);
  } else if ((server->ServerState == kRelteqServerDepleted) || (server->ServerState == kRelteqServerWaiting)
    && (state == kRelteqServerReady) || (state == kRelteqServerRunning)) {
    RelteqServerMakeServerReady(server);
  }

  // change server's state
  server->ServerState = state;
}

#else
#define RelteqServerSetState(server, state) server->ServerState = state  
#endif
