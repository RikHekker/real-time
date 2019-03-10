#include <cpu.h>
#include <ucos_ii.h>

#pragma NO_ENTRY
#pragma NO_FRAME
#pragma TRAP_PROC
void OSStartHighRdy(void) 
{
  OSTaskSwHook();
  OSRunning++;
  asm{
    ldx     OSTCBHighRdy
    lds     0,x
  }
}

#pragma NO_ENTRY
#pragma NO_FRAME
void interrupt VectorNumber_Vswi OSCtxSw() 
{
  asm {
    ldx     OSTCBCur
    sts     0,x
  }
#if OS_LOG_EN > 0
  logTaskPreemption(OSTCBHighRdy, OSTCBCur);
#endif
  OSTaskSwHook();
  OSTCBCur = OSTCBHighRdy;
  OSPrioCur = OSPrioHighRdy;
  asm {
    ldx     OSTCBHighRdy
    lds     0,x
  }
}

#pragma NO_ENTRY
#pragma NO_FRAME
#pragma TRAP_PROC
void OSIntCtxSw(void) 
{
  asm {
    leas    6,+sp
    ldx     OSTCBCur
    sts     0,x
  }
  OSTaskSwHook();
  OSTCBCur = OSTCBHighRdy;
  OSPrioCur = OSPrioHighRdy;
  asm {
    ldx     OSTCBHighRdy
    lds     0,x
  }
}

OS_STK *OSTaskStkInit(void (*task)(void *p_task_arg), void *p_arg, OS_STK *ptos, INT16U opt){
  INT16U *wstk;
  INT8U  *bstk;
  
  wstk    = (INT16U*) ptos;
  *--wstk = (INT16U)  task;             // PC
  *--wstk = (INT16U)  0u;               // IY
  *--wstk = (INT16U)  0u;               // IX
  
  bstk    = (INT8U*)  wstk;
  *--bstk = (INT8U)((INT16U)p_arg>>8);  // B
  *--bstk = (INT8U)   p_arg;            // A
  *--bstk = (1-CPU_ENABLE_XIRQ)*0x40;   // CCRL
  *--bstk = 0x00;                       // CCRH
  
  return (OS_STK*)bstk;
}

void interrupt VectorNumber_Vrti OSTickISR(void) 
{
  OSIntEnter();
  CPUResetTimer();
  OSTimeTick();
  OSIntExit();
}

#if OS_CPU_HOOKS_EN > 0

void OSInitHookBegin() {
  CPUInitSystem();
}

void OSInitHookEnd() {
}

void OSTaskCreateHook(OS_TCB *pTcb){
}

void OSTaskDelHook(OS_TCB *pTcb){
}

void OSTCBInitHook(OS_TCB *pTcb){  
}

void OSTaskIdleHook(){
  asm wai; // Wait for next interrupt
}

void OSTaskStatHook(){
}

void OSTimeTickHook(){
}

void OSTaskSwHook(){
}

#endif
