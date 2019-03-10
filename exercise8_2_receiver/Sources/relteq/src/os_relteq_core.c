/*
 * IMPORTAT: All methods assume they are called within a critical section.
 */

#include "os_relteq.h"

/*
 * Event
 */

/*
 * Create an event (by taking it from the events freelist) and initialize it. 
 *
 * Note that, since the event time will be overwritten when inserting into a queue,
 * we do not specify the event time here.
 */

RelteqEvent* RelteqEventCreate(RelteqEventKind kind, void* ptr) {
  RelteqEvent* event;
  
  if (RelteqEventFreeList == NULL) {
    return NULL;
  }
  
  event = RelteqEventFreeList;
  event->EventKind = kind;
  event->EventTime = 0;
  event->EventPtr = ptr;
  RelteqEventFreeList = RelteqEventFreeList->EventNext;
  event->EventNext = NULL;

  // update stats
#if OS_RELTEQ_STATS_EN > 0
  RelteqStatsNumCreatedEvents++;
  RelteqStatsNumUsedEvents++;
  if (RelteqStatsMaxUsedEvents < RelteqStatsNumUsedEvents) {
    RelteqStatsMaxUsedEvents = RelteqStatsNumUsedEvents;
  }

  if (RelteqEventFreeList == NULL || RelteqEventFreeList->EventNext == NULL) {
    /* IMPORTANT: if this code is reached before the events should have been exhausted, it may mean
       that an event is deleted, while a pointer pointing to it (e.g. server->ServerStopwatchEvent)
       is not reset to NULL. Consequently, the event may be later modified, messing up
       messing up the event freelist. */
    RelteqPortPrint("RelteqStatsNumUsedEvents=%d\n", RelteqStatsNumUsedEvents);
    RelteqPortPrint("RelteqStatsMaxUsedEvents=%d\n", RelteqStatsMaxUsedEvents);
    RelteqPortPrint("RelteqStatsNumCreatedEvents=%d\n", RelteqStatsNumCreatedEvents);
    ASSERT(RelteqEventFreeList != NULL, "OS_RELTEQ_MAX_EVENTS=%d", OS_RELTEQ_MAX_EVENTS);
  }
#endif

  return event;
}

void RelteqEventDel(RelteqEvent* event) {
  ASSERT(event != NULL, "event==NULL when trying to delete it");

  event->EventKind = kRelteqEventDummy;
  event->EventTime = 0;
  event->EventPtr = NULL;
  event->EventPrev = NULL;
  event->EventNext = RelteqEventFreeList;
  RelteqEventFreeList = event;

  // update stats
#if OS_RELTEQ_STATS_EN > 0
  RelteqStatsNumUsedEvents--;
#endif
}

inline void RelteqQueueLinkEvent(RelteqQueue* queue, RelteqEvent* prev, RelteqEvent* event, RelteqEvent* next, INT8U* pErr) {
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // link with the previous event in the queue
  if (prev != NULL) {
    prev->EventNext = event;
    event->EventPrev = prev;
  } else {
    queue->QueueHead = event;
  }
  
  // link with the next event in the queue
  if (next != NULL) {
    event->EventNext = next;
    next->EventPrev = event;
  } else {
    queue->QueueTail = event;
  }
  
  // verify the integrity of the queue. failure means that events were added or removed incorrectly
#if OS_RELTEQ_NAME_SIZE > 0  
  ASSERT(queue->QueueHead != NULL, "Queue '%s' has no head", queue->QueueName);
  ASSERT(queue->QueueTail != NULL, "Queue '%s' has no tail", queue->QueueName);
#else 
  ASSERT(queue->QueueHead != NULL, "Queue has no head");
  ASSERT(queue->QueueTail != NULL, "Queue has no tail");
#endif  
}

inline void RelteqQueueUnlinkEvent(RelteqQueue* queue, RelteqEvent* event, INT8U* pErr) {
#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;
  
  // relink the queue, without the event
  if (event->EventPrev != NULL) {
    event->EventPrev->EventNext = event->EventNext;
  }    
  if (event->EventNext != NULL) {
    event->EventNext->EventPrev = event->EventPrev;
  }

  // update the queue head in case this was the first event
  if (queue->QueueHead == event) {
    queue->QueueHead = event->EventNext;
  }
  
  // update the queue tail in case this was the last event
  if (queue->QueueTail == event) {
    queue->QueueTail = event->EventPrev;
  }
}

extern void RelteqEventHandle(RelteqQueue* queue, RelteqEvent* event, INT8U* pErr);

/*
 * Event queue
 */

RelteqQueue* RelteqQueueCreate(void) {
  RelteqQueue* queue;
  
  if (RelteqQueueFreeList == NULL) {
    return NULL;
  }

  // get a new queue from the free list
  queue = RelteqQueueFreeList;
  RelteqQueueFreeList = RelteqQueueFreeList->QueueNext;
  queue->QueueNext = NULL;

  // update stats
#if OS_RELTEQ_STATS_EN > 0
  RelteqStatsNumCreatedQueues++;
  RelteqStatsNumUsedQueues++;
  if (RelteqStatsMaxUsedQueues < RelteqStatsNumUsedQueues) {
    RelteqStatsMaxUsedQueues = RelteqStatsNumUsedQueues;
  }
#endif

  return queue;
}

void RelteqQueueDel(RelteqQueue* queue) {
  INT8U err;

  // make sure that queue exists
  if (queue == NULL) return;

  // delete events still in the queue, if any
  while (queue->QueueHead != NULL) {
    RelteqQueuePop(queue, &err);
  }

  queue->QueuePrev = NULL;
  
  // return the queue to the free list
  queue->QueueNext = RelteqQueueFreeList;
  RelteqQueueFreeList = queue;
  
  // update stats
#if OS_RELTEQ_STATS_EN > 0
  RelteqStatsNumUsedQueues--;
#endif
}

void RelteqQueueInsertEvent(RelteqQueue* queue, RelteqEvent* event, INT32U time, INT8U* pErr) {
  RelteqEvent* last = NULL;
  RelteqEvent* next = NULL;

#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
#endif
  
  *pErr = OS_ERR_NONE;
  
//  RelteqPortPrint("RelteqQueueInsertEvent(): event (%d, %s) to queue %s\n", event->EventTime, RelteqKindToString(event->EventKind), queue->QueueName);
  next = queue->QueueHead;

  // traverse the queue while accumulating the absolute time of the events, and subtracting
  // it from the inserted event
  while (next != NULL && next != event && next->EventTime < time) {
    // decrement the time
    time -= next->EventTime;
    
    // move to the next event in the queue
    last = next;
    next = next->EventNext;
  }
  
  // check if tried to insert an event which is already in the queue
  if (next == event) {
    *pErr = OS_ERR_RELTEQ_QUEUE_CONTAINS_EVENT;
    return;
  }
  
  if (next) {
    event->EventTime = time;
    RelteqQueueLinkEvent(queue, last, event, next, pErr); ASSERT_OK(*pErr);  
    // adjust the time of the following event, which has changed due to the inserted event
    next->EventTime -= time;
  } else {
    // insert dummy events, if needed
    while (RelteqMaxTime < time) {
      next = RelteqEventCreate(kRelteqEventDummy, NULL);
      // we need to specify the time explicitly, since we link the event, rather than inserting
      next->EventTime = RelteqMaxTime;
      RelteqQueueLinkEvent(queue, last, next, NULL, pErr); ASSERT_OK(*pErr);
      time -= RelteqMaxTime;
      last = next;
    }
    event->EventTime = time;
    RelteqQueueLinkEvent(queue, last, event, NULL, pErr); ASSERT_OK(*pErr);
  }
}

void RelteqQueuePop(RelteqQueue* queue, INT8U* pErr) {
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

  // get the head event
  event = queue->QueueHead;
  
  // if the event exists, remove it
  if (event != NULL) {
    if (event->EventNext != NULL) {
      event->EventNext->EventPrev = NULL;
    }
    queue->QueueHead = event->EventNext;
    if (queue->QueueTail == event) {
      queue->QueueTail = NULL;
    }    
  } else {
    *pErr = OS_ERR_RELTEQ_QUEUE_EMPTY;
  }
}

void RelteqQueueDeleteEvent(RelteqQueue* queue, RelteqEvent* event, INT8U* pErr) {

#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (queue == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  if (event == NULL) {
    *pErr = OS_ERR_RELTEQ_EVENT_NULL;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;

  // if it is at the end of the queue or its time is 0 or it can be merged with the next event,
  // then delete the event completely
  if (event->EventNext == NULL || event->EventTime == 0
  || event->EventNext->EventTime <= RelteqMaxTime - event->EventTime) {

    // merge the event time with the following event, if anything to merge
    if (event->EventNext && event->EventTime != 0) {
      event->EventNext->EventTime += event->EventTime;
    }

    // remove the event from the queue
    RelteqQueueUnlinkEvent(queue, event, pErr); ASSERT_OK(*pErr);

    // delete the event
    RelteqEventDel(event);
    
  } else {
    // otherwise it is important for the integrity of the queue, so make it a dummy
    event->EventKind = kRelteqEventDummy;
    event->EventPtr = NULL;
  }
  
  // remove any trailing dummy events
  for (event = queue->QueueTail; event && event->EventKind == kRelteqEventDummy; event = queue->QueueTail) {
    // remove the event from the queue
    RelteqQueueUnlinkEvent(queue, event, pErr); ASSERT_OK(*pErr);
    // delete the event
    RelteqEventDel(event);
  }
}

/*
 * Synchronises the target queue with the source queue, until an event in the source queue.
 * Conceptually it accumulates the time until (and including) the event in the source quueue and 
 * then synchronizes the target queue with that time.
 * If event is not in the queue then the complete queue is synchronized.
 * If event is NULL then nothing is synchronized.
 */

void RelteqQueueSyncWithQueueUntilEvent(RelteqQueue* targetQueue, RelteqQueue* sourceQueue, RelteqEvent* stopEvent, INT8U* pErr) {
  INT8U err;
  RelteqEvent* source = NULL;
  RelteqEvent* target = NULL;
  RelteqTime overflow = 0;
  
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

  if (!stopEvent) return;
                                  
  source = sourceQueue->QueueHead;
  target = targetQueue->QueueHead;
  
  // traverse the two queues, until either one of them is exhausted
  while (source != NULL && source->EventPrev != stopEvent && target != NULL) {
    // match the current source event with the target event
    while (source != NULL && source->EventPrev != stopEvent && (source->EventTime - overflow) <= target->EventTime) {

      // decerement the target time
      target->EventTime -= (source->EventTime - overflow);
      overflow = 0;
      // move to the next event
      source = source->EventNext;
    }
    /* source == NULL or source->prev == stopEvent or source.time - overflow > target.time */
    
    // check if reached the end of the source queue
    if (source == NULL || source->EventPrev == stopEvent) break;

    /* source.time - overflow > target.time */

    // decrement the source time. Note that the overflow needs to be accumulated in case a long
    // source event can accomodate several target events
    overflow += target->EventTime;
    
    // remove the event from the queue      
    RelteqQueuePop(targetQueue, &err);

    // handle the target event
    RelteqEventHandle(targetQueue, target, &err);
    
    // move to the next event
    target = targetQueue->QueueHead;
  }
  
  // sync all leading 0 events (e.g. in case the stop event is at the head of the source queue)
  for (; target && target->EventTime == 0; target = targetQueue->QueueHead) {
    RelteqQueuePop(targetQueue, &err);
    RelteqEventHandle(targetQueue, target, &err);    
  }
}


void RelteqQueueSyncWithTime(RelteqQueue* queue, RelteqTime time, INT8U* pErr) {
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
  if (time < 0) {
    *pErr = OS_ERR_RELTEQ_TIME_NEGATIVE;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;  

//RelteqPortPrint("%d sync with time=%d queue=%s head=%d\n", OSTimeGet(), time, queue->QueueName, queue->QueueHead->EventTime);
  // traverse the queue and handle expired events
  event = queue->QueueHead;
  while (event != NULL && event->EventTime <= time) {
    time -= event->EventTime;
    RelteqQueuePop(queue, &err);
    RelteqEventHandle(queue, event, &err);
    event = queue->QueueHead;
  }
  
  // decrement the first unhandled event by the remianing time
  if (event != NULL) {
    event->EventTime -= time;
  }
}

void RelteqQueueIncrement(RelteqQueue* queue, INT8U* pErr) {
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
  
  // get the head event
  event = queue->QueueHead;
  
  if (event != NULL && event->EventTime < RelteqMaxTime) {
    event->EventTime++;
  } else {
    event = RelteqEventCreate(kRelteqEventDummy, NULL);
    if (event == NULL) {
      *pErr = OS_ERR_RELTEQ_EVENT_NULL;
      return;
    }
    
    event->EventTime = 1;
    RelteqQueueLinkEvent(queue, NULL, event, queue->QueueHead, pErr);
  }
}

inline void RelteqQueueUpdateEarliestTime(RelteqQueue* queue) {
  if (queue->QueueHead && RelteqEarliestEventTime > queue->QueueHead->EventTime) {
    RelteqEarliestEventTime = queue->QueueHead->EventTime;
    RelteqEarliestQueue = queue;
  }
}


inline void RelteqUpdateEarliestTime(void) {
  RelteqQueue* queue;
  for (queue = RelteqActiveQueues; queue != NULL; queue = queue->QueueNext) {
    RelteqQueueUpdateEarliestTime(queue);
  }
}

void RelteqQueueMove(RelteqQueue* queue, RelteqQueue** pSourceList, 
                     RelteqQueue** pTargetList, INT8U* pErr) {
  RelteqQueue* source;
  RelteqQueue* target;

#if OS_ARG_CHK_EN > 0
  if (pErr == NULL) {
      return;
  }
  if (queue == NULL || pTargetList == NULL) {
    *pErr = OS_ERR_RELTEQ_QUEUE_NULL;
    return;
  }
  if (pSourceList == pTargetList) {
    *pErr = OS_ERR_RELTEQ_QUEUE_SAME;
    return;
  }
#endif

  *pErr = OS_ERR_NONE;

  // check if queue is not already the head of the target list
  target = *pTargetList;
  if (target == queue) {
    return; 
  }

  // remove from the source list, by adapting its head if pointing to the queue
  if (pSourceList != NULL) {
    source = *pSourceList;
    if (source == queue) {
      *pSourceList = source->QueueNext;
    }
  }
  
  // remove the queue from the source chain
  if (queue->QueuePrev != NULL) {
    queue->QueuePrev->QueueNext = queue->QueueNext;
  }
  if (queue->QueueNext != NULL) {
    queue->QueueNext->QueuePrev = queue->QueuePrev;
  }

  // add the queue at the beginning of the target list
  if (target != NULL) {
    target->QueuePrev = queue;
  }
  queue->QueueNext = target;
  queue->QueuePrev = NULL;
  *pTargetList = queue;
  
  ASSERT(queue != queue->QueuePrev, "queue != queue->QueuePrev");
  ASSERT(queue != queue->QueueNext, "queue != queue->QueueNext");
}

void RelteqQueueActivate(RelteqQueue* queue, INT8U* pErr) {
  RelteqQueueMove(queue, &RelteqInactiveQueues, &RelteqActiveQueues, pErr);
}

void RelteqQueueDeactivate(RelteqQueue* queue, INT8U* pErr) {
  RelteqQueueMove(queue, &RelteqActiveQueues, &RelteqInactiveQueues, pErr);
}

/*
 * RelteqHandleDummy()
 */

void RelteqHandleDummy(RelteqQueue* queue, RelteqEvent* event) {
#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
#if OS_RELTEQ_NAME_SIZE > 1
  logCustomEvent("relteqEvent dummy \"inside %s\"", (void*)queue->QueueName, NULL);
#else
  logCustomEvent("relteqEvent dummy", NULL, NULL);
#endif
#endif
  RelteqEventDel(event);
}

/*
 * RelteqHandleDelay()
 */
 
void RelteqHandleDelay(RelteqQueue* queue, RelteqEvent* event) {
  RelteqTask* task;
  (void)queue;
  
  task = event->EventPtr;

  // if the task was suspended, make it ready
#if OS_RELTEQ_HSF_EN > 0
  if (RelteqPortTaskIsSuspended(task)) {
    RelteqServerMakeTaskReady(task);
  }
#endif

  RelteqPortHandleDelay(task);
  
  RelteqEventDel(event);
#if OS_LOG_EN > 0 && OS_RELTEQ_DEBUG_EN > 0
  logCustomEvent("relteqEvent delay \"%s\"", RelteqPortTaskName(task), 0);
#endif
}

/*
 * RelteqEventHandle()
 */

void RelteqEventHandle(RelteqQueue* queue, RelteqEvent* event, INT8U* pErr) {
  *pErr = OS_ERR_NONE;
  //RelteqPortPrint("Handling %s @ %d\n", RelteqKindToString(event->EventKind), event->EventTime); 
  ASSERT(event->EventKind < kRelteqEventNum && RelteqEventHandlers[event->EventKind], "unknown event kind %d", event->EventKind);
  RelteqEventHandlers[event->EventKind](queue, event);
}

/*
 * RelteqSyncWithCurrentTime()
 */

void RelteqSyncWithCurrentTime(void) {
  INT8U err;
  RelteqQueue* queue;
 
  // reset the earliest event time
  RelteqEarliestEventTime = RelteqMaxTime;
  RelteqEarliestQueue = NULL;
  
  // traverse thorough all active queues and handle expired events
  for (queue = RelteqActiveQueues; queue != NULL; queue = queue->QueueNext) {
    // handle all expired events in the current queue
    RelteqQueueSyncWithTime(queue, RelteqCurrentTime, &err); ASSERT_OK(err);       
    ASSERT(queue != queue->QueueNext, "queue != queue->QueueNext");
  }

  // adjust the earliest event time, needed for multiplexing active queues
  RelteqUpdateEarliestTime();

  // NOTE: in case of HSF, then the earliest time is goign to be updated at the end of
  // the scheduler. Hence we actually do not need to update the earliest time here, since 
  // the scheduler will be called whenever time is synced. However, to avoid bugs when
  // this method is called by some future extension, we play it safe and update the earliest
  // time anyway.

  // reset the current time
  RelteqCurrentTime = 0;
}

/*
 * RelteqTimeTick()
 */

void RelteqTimeTick(void) {
#if OS_RELTEQ_HSF_EN > 0    
  INT8U err;
#endif

  //if (OSTimeGet() == 50) { logPrint("early_termination.log"); }

  // make sure not to call the scheduler unnecessarily, when returning from the tick ISR
  RelteqNeedScheduling = 0;

  // increment the timer
  RelteqCurrentTime++;
  
//RelteqPortPrint("Tick %d %d\n", OSTime, RelteqCurrentTime);
// RelteqPortPrint("Tick %d: relteq=%d, eraliest: (time=%d event=%s queue=%s)\n", OSTime, RelteqCurrentTime, RelteqEarliestEventTime, RelteqKindToString(RelteqEarliestQueue->QueueHead->EventKind), RelteqEarliestQueue->QueueName);
  
#if OS_RELTEQ_HSF_EN > 0    
  // monitor the consuption of the currently running server
  if (RelteqCurrentServer != NULL) {
    RelteqCurrentServer->ServerBudgetLeft--;
  }

  // account the time which has passed in the stopwatch
  RelteqQueueIncrement(RelteqStopwatch, &err); ASSERT_OK(err);
#endif

  // check if any event has expired
  if (RelteqEarliestEventTime <= RelteqCurrentTime) {

    RelteqSyncWithCurrentTime();
    
    // call the scheduler in case servers depleted, replenished, period arrived, ...
    RelteqNeedScheduling = 1;
  }
}

/*
 * RelteqCore_Init()
 */

void RelteqCore_Init(void) {
  INT8U err;
  int i;
  RelteqEvent* event;
  RelteqQueue* queue;

  // initialize events
#if OS_RELTEQ_MAX_EVENTS > 0
  // link all events in the global events table
  for (i = 0; i < OS_RELTEQ_MAX_EVENTS-1; i++) {
    event = &RelteqEvents[i];
    RelteqEventDel(event);
    event->EventNext = &RelteqEvents[i+1];
  }

  // initialise the last event, which was skipped in the loop
  event = &RelteqEvents[i];
  RelteqEventDel(event);
  event->EventNext = NULL;

  // point the free list to the first event in the table
  RelteqEventFreeList = &RelteqEvents[0];
#endif
  
  // initialize queues
#if OS_RELTEQ_MAX_QUEUES > 0
  // link all queues in the global queues table
  for (i = 0; i < OS_RELTEQ_MAX_QUEUES-1; i++) {
    queue = &RelteqQueues[i];
    RelteqQueueDel(queue);
    queue->QueueNext = &RelteqQueues[i+1];
  }

  // initialise the last queue, which was skipped in the loop
  queue = &RelteqQueues[i];
  RelteqQueueDel(queue);
  queue->QueueNext = NULL;

  // point the free list to the first queue in the table
  RelteqQueueFreeList = &RelteqQueues[0];
#endif
  
  // intialize the stats
#if OS_RELTEQ_STATS_EN > 0
  RelteqStatsNumUsedEvents = 0;  
  RelteqStatsNumUsedQueues = 0;
  RelteqStatsMaxUsedEvents = 0;  
  RelteqStatsMaxUsedQueues = 0;
  RelteqStatsNumCreatedEvents = 0;
  RelteqStatsNumCreatedQueues = 0;
#endif

  // create the global queue
  RelteqGlobalQueue = RelteqQueueCreate();
#if OS_RELTEQ_NAME_SIZE > 1  
  RelteqPortStringPrint(RelteqGlobalQueue->QueueName, "%s", "Global");
#endif

  // make activate the global queue
  RelteqQueueActivate(RelteqGlobalQueue, &err); ASSERT_OK(err);
  
  // make sure to schedule for the first time
  RelteqNeedScheduling = 1;
  
  // register event handlers
  RelteqRegisterEventHandler(kRelteqEventDummy, RelteqHandleDummy);
  RelteqRegisterEventHandler(kRelteqEventDelay, RelteqHandleDelay);  
}

#if OS_RELTEQ_NAME_SIZE > 1
void  RelteqQueueNameSet(RelteqQueue* queue, char* name, INT8U* perr) {
#if OS_ARG_CHK_EN > 0
    if (perr == NULL) {                        
        return;
    }
    if (queue == NULL) {                 
	    *perr = OS_ERR_RELTEQ_QUEUE_NULL;             			 
	    return;
    }
    if (name == NULL) {                       
        *perr = OS_ERR_RELTEQ_NAME_NULL;                   
        return;
    }
#endif

    *perr = OS_ERR_NONE;

    RelteqPortStringCopy(queue->QueueName, name, perr);
}
#endif /* OS_RELTEQ_NAME_SIZE */

#if OS_RELTEQ_DEBUG_EN > 0 || OS_ARG_CHK_EN > 0
char* RelteqErrToString(INT8U err) {
  switch (err) {
    case OS_ERR_NONE :
      return "OS_ERR_NONE";
    case OS_ERR_RELTEQ_TIME_NEGATIVE :
      return "OS_ERR_RELTEQ_EVENT_NEGATIVE";
    case OS_ERR_RELTEQ_QUEUE_EMPTY :
      return "OS_ERR_RELTEQ_QUEUE_EMPTY";
    case OS_ERR_RELTEQ_QUEUE_CONTAINS_EVENT :
      return "OS_ERR_RELTEQ_QUEUE_CONTAINS_EVENT";
    case OS_ERR_RELTEQ_QUEUE_NULL :
      return "OS_ERR_RELTEQ_QUEUE_NULL";
    case OS_ERR_RELTEQ_QUEUE_SAME :
      return "OS_ERR_RELTEQ_QUEUE_SAME";
    case OS_ERR_RELTEQ_EVENT_NULL :
      return "OS_ERR_RELTEQ_EVENT_NULL";
    case OS_ERR_RELTEQ_SERVER_NULL :
      return "OS_ERR_RELTEQ_SERVER_NULL";
    case OS_ERR_RELTEQ_BUDGET_ZERO :
      return "OS_ERR_RELTEQ_BUDGET_ZERO";
    case OS_ERR_RELTEQ_TIME_TOO_LARGE :
      return "OS_ERR_RELTEQ_TIME_TOO_LARGE";
    case OS_ERR_RELTEQ_NO_MORE_PERIODIC_TASKS :
      return "OS_ERR_RELTEQ_NO_MORE_PERIODIC_TASKS";
      
    default :
      return "unknown error code";
  }
  return ""; /* avoid compiler warning that reaching end of non-void function */
}
#endif

#if OS_RELTEQ_DEBUG_EN > 0
char* RelteqKindToString(INT8U kind) {
  switch (kind) {
    case kRelteqEventDummy :
      return "kRelteqEventDummy";
    case kRelteqEventDeadline :
      return "kRelteqEventDeadline";
    case kRelteqEventPeriod :
      return "kRelteqEventPeriod";
#if OS_RELTEQ_HSF_EN > 0
    case kRelteqEventReplenishment :
      return "kRelteqEventReplenishment";
    case kRelteqEventDepletion :
      return "kRelteqEventDepletion";
    case kRelteqEventStopwatch :
      return "kRelteqEventStopwatch";
#endif
    default :
      return "unknown kind";
  }
  return ""; /* avoid compiler warning that reaching end of non-void function */
}

void RelteqDumpEvent(RelteqEvent* event) {
  if (event == NULL) {
    RelteqPortPrint("  Event=NULL\n");
  } else {
    RelteqPortPrint("  EventKind=%s\n", RelteqKindToString(event->EventKind));
    RelteqPortPrint("  EventTime=%d\n", event->EventTime);
    RelteqPortPrint("  EventPtr=%p\n", event->EventPtr);
  }
}

void RelteqDumpQueue(RelteqQueue* queue) {
  RelteqEvent* event;
  
  if (queue == NULL) {
    RelteqPortPrint("Queue=NULL\n");
  } else {
    RelteqPortPrint("Queue %p (%s)= {\n", queue, queue->QueueName);
    event = queue->QueueHead;
    while (event != NULL) {
      RelteqDumpEvent(event);
      event = event->EventNext;
    }
    RelteqPortPrint("}\n");
  }
}

void RelteqDumpQueueCompact(RelteqQueue* queue) {
  RelteqEvent* event;
  
  RelteqPortPrint("%d : ", RelteqPortTimeGet());
  if (queue == NULL) {
    RelteqPortPrint("Queue=NULL\n");
  } else {
    RelteqPortPrint("Queue (%s)= {\n", queue->QueueName);
    event = queue->QueueHead;
    while (event != NULL) {
      RelteqPortPrint("  %d:", event->EventTime);
      switch (event->EventKind) {
        case kRelteqEventDummy:
          RelteqPortPrint("*");
          break;
        case kRelteqEventPeriod:
          RelteqPortPrint("P(%s)", RelteqPortTaskName((RelteqTask*)event->EventPtr));
          break;
        case kRelteqEventDeadline:
          RelteqPortPrint("D(%s)", RelteqPortTaskName((RelteqTask*)event->EventPtr));
          break;
        case kRelteqEventDelay:
          RelteqPortPrint("DY(%s)", RelteqPortTaskName((RelteqTask*)event->EventPtr));
          break;
#if OS_RELTEQ_HSF_EN > 0
       case kRelteqEventStopwatch:
          RelteqPortPrint("SW(%s)", ((RelteqServer*)event->EventPtr)->ServerName);
          break;
        case kRelteqEventWakeup:
          RelteqPortPrint("W(%s)", ((RelteqServer*)event->EventPtr)->ServerName);
          break;
        case kRelteqEventReplenishment:
          RelteqPortPrint("R(%s)", ((RelteqServer*)event->EventPtr)->ServerName);
          break;
        case kRelteqEventServerDeadline:
          RelteqPortPrint("SD(%s)", ((RelteqServer*)event->EventPtr)->ServerName);
          break;
#endif
        default:
          RelteqPortPrint("?");
          break;
      }
      event = event->EventNext;
    }
    RelteqPortPrint("\n}\n");
  }
}
#endif /* OS_RELTEQ_DEBUG_EN */
