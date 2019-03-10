#ifndef OS_RELTEQ_PERIODIC_TASK_H
#define OS_RELTEQ_PERIODIC_TASK_H

#if OS_RELTEQ_PERIODIC_TASK_EN > 0

void RelteqPeriodicTaskSetPeriod(RelteqTask* task, INT32U period, INT32U offset);

#endif /* OS_RELTEQ_PERIODIC_TASK_EN */

#endif /* OS_RELTEQ_PERIODIC_TASK_H */
