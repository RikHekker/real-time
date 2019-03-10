if LogBreakPoint == 1
  fprintf(log.grasp, "plot %d jobArrived %d.%d %d\n", OSTime, LogTargetTask->OSTCBPrio, LogArrivedJob[LogTargetTask->OSTCBPrio], LogTargetTask->OSTCBPrio)
elseif LogBreakPoint == 2
  fprintf(log.grasp, "plot %d jobResumed %d.%d\n", OSTime, LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
elseif LogBreakPoint == 3
  fprintf(log.grasp, "plot %d jobCompleted %d.%d\n", OSTime, LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
elseif LogBreakPoint == 4
  if LogSourceTask != 0
    if LogTargetTask != 0
      fprintf(log.grasp, "plot %d jobPreempted %d.%d -target %d.%d\n", OSTime, LogSourceTask->OSTCBPrio, LogCurrentJob[LogSourceTask->OSTCBPrio], LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
    else
      fprintf(log.grasp, "plot %d jobPreempted %d.%d\n", OSTime, LogSourceTask->OSTCBPrio, LogArrivedJob[LogSourceTask->OSTCBPrio])
    endif
  endif
  if LogTargetTask != 0
    fprintf(log.grasp, "plot %d jobResumed %d.%d\n", OSTime, LogTargetTask->OSTCBPrio, LogCurrentJob[LogTargetTask->OSTCBPrio])
  endif
elseif LogBreakPoint == 6
  fprintf(log.grasp, "newTask %d -priority %d\n", LogTargetTask->OSTCBPrio, LogTargetTask->OSTCBPrio)
else
  printf("unknown break point %d", LogBreakPoint)
endif