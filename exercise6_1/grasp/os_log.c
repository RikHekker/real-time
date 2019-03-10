#include "os_log.h"

#if OS_LOG_EN > 0

#define kLogJobArrived 1
#define kLogJobResumed 2
#define kLogJobCompleted 3
#define kLogJobPreempted 4
#define kLogNewTask 6

unsigned int LogEventCounter;
unsigned int LogCurrentJob[OS_LOWEST_PRIO+1];
unsigned int LogArrivedJob[OS_LOWEST_PRIO+1];
unsigned int LogBreakPoint = 0;

LogTask* LogTargetTask;
LogTask* LogSourceTask;

void logBreak(void) {
  LogEventCounter++;
}

void logTaskArrival(LogTask* task) {
  asm sei;
  LogTargetTask = task;
  LogArrivedJob[task->OSTCBPrio]++;
  LogBreakPoint = kLogJobArrived;
  logBreak();  
  asm cli; 
}

void logTaskStart(LogTask* task) {
  asm sei;
  LogTargetTask = task;
  LogCurrentJob[task->OSTCBPrio]++; 
  LogBreakPoint = kLogJobResumed;
  logBreak();  
  asm cli; 
}

void logTaskCompletion(LogTask* task) {
  asm sei;
  LogTargetTask = task; 
  LogBreakPoint = kLogJobCompleted;
  logBreak();  
  asm cli;
}

void logTaskPreemption(LogTask* target, LogTask* source) {
  asm sei;
  LogSourceTask = source;
  LogTargetTask = target;
  LogBreakPoint = kLogJobPreempted;
  logBreak();
  asm cli;
}

void logTaskMissedDeadline(LogTask* task) {}

void logTaskCreated(LogTask* task) {
  asm sei;
  LogTargetTask = task;
  LogBreakPoint = kLogNewTask;
  logBreak(); 
  asm cli; 
}

void logTaskDeleted(LogTask* task) {
}

#endif /* OS_LOG_EN */