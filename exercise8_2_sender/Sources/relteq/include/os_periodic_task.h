#ifndef OS_PERIODIC_TASK_H
#define OS_PERIODIC_TASK_H

INT8U  OSTaskSetPeriod(OS_TCB* task, INT16U period);
INT8U  OSTaskSetPeriodEx(OS_TCB* task, INT16U period, INT16U offset);
INT16U OSTaskPeriodWait(void);
INT8U  OSTaskCreatePeriodic(void  (*function)(void),
                            INT16U  period,
                            INT16U  offset,
                            OS_STK* ptos,
                            INT8U   prio);
INT8U  OSTaskCreatePeriodicExt (void  (*function)(void),
                                INT16U  period,
                                INT16U  offset,
                                OS_STK* ptos,
                                INT8U   prio,
                                INT16U  id,
                                OS_STK* pbos,
                                INT32U  stk_size,
                                void*   pext,
                                INT16U  opt);

#endif /* OS_PERIODIC_TASK_H */
