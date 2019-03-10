if LogBreakPoint == 1
  printf("plot %d jobArrived %d.%d %d\n", OSTime, LogTargetTask->OSTCBPrio, LogArrivedJob[LogTargetTask->OSTCBPrio], LogTargetTask->OSTCBPrio)
  showcycles
elseif LogBreakPoint == 2
  printf("plot %d jobResumed %d.%d\n", OSTime, LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
  showcycles
elseif LogBreakPoint == 3
  printf("plot %d jobCompleted %d.%d\n", OSTime, LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
  showcycles
elseif LogBreakPoint == 4
  if LogSourceTask != 0
    if LogTargetTask != 0
      printf("plot %d jobPreempted %d.%d -target %d.%d\n", OSTime, LogSourceTask->OSTCBPrio, LogCurrentJob[LogSourceTask->OSTCBPrio], LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
      showcycles
    else
      printf("plot %d jobPreempted %d.%d\n", OSTime, LogSourceTask->OSTCBPrio, LogArrivedJob[LogSourceTask->OSTCBPrio])
      showcycles
    endif
  endif
  if LogTargetTask != 0
    printf("plot %d jobResumed %d.%d\n", OSTime, LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
    showcycles
  endif
elseif LogBreakPoint == 6
  printf("newTask %d -priority %d\n", LogTargetTask->OSTCBPrio, LogTargetTask->OSTCBPrio)
  showcycles
else
  printf("unknown break point %d", LogBreakPoint)
endif