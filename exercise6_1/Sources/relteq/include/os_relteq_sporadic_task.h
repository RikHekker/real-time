#ifndef OS_RELTEQ_SPORADIC_TASK_H
#define OS_RELTEQ_SPORADIC_TASK_H

#if OS_RELTEQ_SPORADIC_TASK_EN > 0

#if OS_RELTEQ_PERIODIC_TASK_EN == 0
  #error "OS_RELTEQ_SPORADIC_TASK_EN requires OS_RELTEQ_PERIODIC_TASK_EN to be enabled"
#endif

/* flag indexes for the OSTCBSporadicFlag field inteh TCB */
#define RelteqSporadicFlagIsSporadic    1
#define RelteqSporadicFlagPeriodArrived   2
#define RelteqSporadicFlagEventArrived  4

void RelteqMakeSporadic(RelteqTask* task, INT8U* pErr);
void RelteqSporadicEventArrived(RelteqTask* task);

#endif /* OS_RELTEQ_SPORADIC_TASK_EN */

#endif /* OS_RELTEQ_SPORADIC_TASK_H */
