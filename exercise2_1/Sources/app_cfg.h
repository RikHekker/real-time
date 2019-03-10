#ifndef   APP_CFG_H
#define   APP_CFG_H

// Set CPU_CRITICAL_XGIE to 1 to clear XGate Interrupt Enable (XGIE) bit 
// on OS_ENTER_CRITICAL() and restore it on OS_EXIT_CRITICAL()
#define CPU_CRITICAL_XGIE        0

// Set CPU_ENABLE_XIRQ to 1 to enable XIRQ in CPU
#define CPU_ENABLE_XIRQ          0
 
// Set #define CPU_ENABLE_ATD to 1 to enable the Analogue to Digital converter
#define CPU_ENABLE_ATD 1
                         
// Set #define CPU_ENABLE_RTI to 1 to enable the real-time timer interrupt
#define CPU_ENABLE_RTI 1                         
                            
// Catch ALL interrupt vectors (mainly used for debugging unexpected interrupts)
#define CPU_CATCH_ALL_INTERRUPTS 0


#endif
