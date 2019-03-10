#ifndef   OS_CPU_H
#define   OS_CPU_H

#include  "app_cfg.h"
#include  "cpu.h"

#pragma MESSAGE DISABLE C1420
#pragma MESSAGE DISABLE C2705
#pragma MESSAGE DISABLE C5660
#pragma MESSAGE DISABLE C5703
#pragma MESSAGE DISABLE C5905
#pragma MESSAGE DISABLE C12053
#pragma MESSAGE DISABLE C12056

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;
typedef signed   char  INT8S;
typedef unsigned int   INT16U;
typedef signed   int   INT16S;
typedef unsigned long  INT32U;
typedef signed   long  INT32S;
typedef float          FP32;
typedef double         FP64;

typedef INT8U          OS_STK;
typedef INT16U         OS_CPU_SR;

#define OS_CRITICAL_METHOD 3

#if CPU_CRITICAL_XGIE == 0

#if OS_CRITICAL_METHOD == 1
#define OS_ENTER_CRITICAL() asm sei;
#define OS_EXIT_CRITICAL()  asm cli;
#endif

#if OS_CRITICAL_METHOD == 3                            
#define OS_ENTER_CRITICAL() { asm pshcw; asm sei; asm movw 2,sp+,cpu_sr; }
#define OS_EXIT_CRITICAL()  { asm movw cpu_sr,2,-sp; asm pulcw; }
#endif

#else // CPU_CRITICAL_XGIE != 0

#if OS_CRITICAL_METHOD == 1
#define OS_ENTER_CRITICAL() { asm sei; XGMCTL_XGIE=0; }
#define OS_EXIT_CRITICAL()  { asm cli; XGMCTL_XGIE=1; }
#endif

#if OS_CRITICAL_METHOD == 3                            
// Abusing (unused) bit 14 of cpu_sr to store XGIE bit
#define OS_ENTER_CRITICAL() { asm pshcw; asm sei; asm movw 2,sp+,cpu_sr; cpu_sr|=XGMCTL_XGIE<<14; XGMCTL_XGIE=0; }
#define OS_EXIT_CRITICAL()  { XGMCTL_XGIE=(cpu_sr>>14)&1; cpu_sr&=0xBFFF; asm movw cpu_sr,2,-sp; asm pulcw; }
#endif

#endif // CPU_CRITICAL_XGIE

#define OS_STK_GROWTH 1

#define OS_TASK_SW()        asm swi;

void OSIntCtxSw(void);
void OSStartHighRdy(void);

#endif
