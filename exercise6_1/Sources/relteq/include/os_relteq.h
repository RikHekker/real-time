#ifndef OS_RELTEQ_H
#define OS_RELTEQ_H

/*
 * Port definitions
 */
 
#include "os_relteq_port.h"

#if OS_RELTEQ_EN > 0

#ifndef NULL
#define NULL (void*)0
#endif

#if OS_RELTEQ_HSF_EN > 0
  #define OS_RELTEQ_MAX_QUEUES (OS_RELTEQ_NUM_CUSTOM_QUEUES + OS_RELTEQ_MAX_SERVERS*4 + 3)
#else
  #define OS_RELTEQ_MAX_QUEUES (OS_RELTEQ_NUM_CUSTOM_QUEUES + 1)
#endif

typedef enum {
  kRelteqEventDummy = 0,
  kRelteqEventDelay,
  kRelteqEventDeadline,
  kRelteqEventPeriod,
#if OS_RELTEQ_HSF_EN > 0
  kRelteqEventServerDeadline,
  kRelteqEventReplenishment,
  kRelteqEventDepletion,
  kRelteqEventStopwatch,
  kRelteqEventWakeup,
#endif
#if OS_RELTEQ_HSF_EN > 0
  kRelteqEventReplenishmentCBS,
  kRelteqEventDepletionCBS,  
#endif
  kRelteqEventUser1, /* custom event, which can be implemented by the user */
  kRelteqEventUser2,
  kRelteqEventUser3,
  kRelteqEventNum
} RelteqEventKind;

typedef struct RelteqEvent {
  RelteqEventKind       EventKind;
  RelteqTime            EventTime;
  void*                 EventPtr;
  struct RelteqEvent*  EventPrev; // TODO: change generic pointer to index in the table (can be INT16)
  struct RelteqEvent*  EventNext;
} RelteqEvent;


typedef struct RelteqQueue {
  RelteqEvent*          QueueHead;
  RelteqEvent*          QueueTail;
  struct RelteqQueue*  QueuePrev; // each queue is linked in one list: 
  struct RelteqQueue*  QueueNext; // QueueFreeList, ActiveQueues or InactiveQueues
#if OS_RELTEQ_NAME_SIZE > 1
  char                  QueueName[OS_RELTEQ_NAME_SIZE];
#endif
} RelteqQueue;

typedef void (*RelteqEventHandler)(RelteqQueue* queue, RelteqEvent* event);

/*
 * Global variables
 */
 
extern RelteqEvent*  RelteqEventFreeList;
extern RelteqEvent   RelteqEvents[OS_RELTEQ_MAX_EVENTS];

extern RelteqQueue   RelteqQueues[OS_RELTEQ_MAX_QUEUES];
extern RelteqQueue*  RelteqQueueFreeList;

extern RelteqQueue*  RelteqGlobalQueue;
extern RelteqQueue*  RelteqActiveQueues;
extern RelteqQueue*  RelteqInactiveQueues;
extern RelteqTime    RelteqCurrentTime;
extern RelteqTime    RelteqEarliestEventTime;
extern RelteqQueue*  RelteqEarliestQueue;

extern INT8U         RelteqNeedScheduling;

extern RelteqEventHandler RelteqEventHandlers[kRelteqEventNum];

#if OS_RELTEQ_STATS_EN > 0
extern INT16U RelteqStatsNumUsedEvents;
extern INT16U RelteqStatsMaxUsedEvents;
extern INT16U RelteqStatsNumCreatedEvents;
extern INT16U RelteqStatsNumUsedQueues;
extern INT16U RelteqStatsMaxUsedQueues;
extern INT16U RelteqStatsNumCreatedQueues;
#endif

/*
 * Public methods
 */

void          Relteq_Init(void);
void          RelteqTimeTick(void);

/*
 * Protected methods, intended ONLY for RELTEQ extensions developers.
 *
 * DO NOT CALL THESE DIRECTLY FROM APPLICATION !!!
 */
 
RelteqEvent*  RelteqEventCreate(RelteqEventKind kind, void* ptr);
void          RelteqEventDel(RelteqEvent* event);
RelteqQueue*  RelteqQueueCreate(void);
void          RelteqQueueInsertEvent(RelteqQueue* queue, RelteqEvent* event, INT32U time, INT8U* pErr);
void          RelteqQueuePop(RelteqQueue* queue, INT8U* pErr);
void          RelteqUpdateEarliestTime(void);
void          RelteqQueueUpdateEarliestTime(RelteqQueue* queue);

inline void   RelteqQueueLinkEvent(RelteqQueue* queue, RelteqEvent* prev, RelteqEvent* event, RelteqEvent* next, INT8U* pErr);
inline void   RelteqQueueUnlinkEvent(RelteqQueue* queue, RelteqEvent* event, INT8U* pErr);


#define RelteqRegisterEventHandler(event, handler) RelteqEventHandlers[event] = handler;

#if OS_RELTEQ_DEBUG_EN > 0 || OS_ARG_CHK_EN > 0
char*         RelteqErrToString(INT8U err);
#endif

#if OS_RELTEQ_DEBUG_EN > 0
char*         RelteqKindToString(INT8U kind);
void          RelteqDumpEvent(RelteqEvent* event);
void          RelteqDumpQueue(RelteqQueue* queue);
#endif

/*
 * RELTEQ extensions
 */
 
#include "os_relteq_periodic_task.h"
#include "os_relteq_sporadic_task.h"
#include "os_relteq_hsf.h"



#endif /* OS_RELTEQ_EN > 0 */

#endif /* OS_RELTEQ_H */
