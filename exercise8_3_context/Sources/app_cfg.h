#ifndef   APP_CFG_H
#define   APP_CFG_H

#include  "os_cfg.h"

// Set priority of the uC/OS-II timer task
#if OS_TMR_EN == 1
#define OS_TASK_TMR_PRIO    (OS_LOWEST_PRIO-2u)
#endif

// Set CPU_CRITICAL_XGIE to 1 to clear XGate Interrupt Enable (XGIE) bit 
// on OS_ENTER_CRITICAL() and restore it on OS_EXIT_CRITICAL()
#define CPU_CRITICAL_XGIE        0

// Set CPU_ENABLE_XIRQ to 1 to enable XIRQ in CPU
#define CPU_ENABLE_XIRQ          0

// Set #define CPU_ENABLE_ATD to 1 to enable Analogue to Digital converter
#define CPU_ENABLE_ATD 1
                            
// Catch ALL interrupt vectors (mainly used for debugging unexpected interrupts)
#define CPU_CATCH_ALL_INTERRUPTS 0

#endif
